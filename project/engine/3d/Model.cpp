#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <assert.h>
#include "MyMath.h"

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{
	modelCommon_ = modelCommon;

	modelData = LoadObjectFile(directorypath, filename);

	//モデルオブジェクト
	//モデル用のVertexResourceを作成
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialData->enableLighting = false;//有効にするか否か
	materialData->uvTransform = Math::MakeIdentity4x4();
	materialData->shininess = 70;
	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャ番号を取得
	modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(modelData.material.textureFilePath);
}


void Model::Draw()
{
	//VertexBufferViewを設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//マテリアルのCBufferの場所を設定
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	//描画！
	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}


MaterialData Model::LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename)
{
	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く

	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;
		//identifierの応じた処理
		if (identifile == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/" + textureFilename;
		}
	}
	return materialData;
}


ModelData Model::LoadObjectFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;//構築するModelData
	std::vector<Vector4>positions;//位置
	std::vector<Vector3>normals;//法線
	std::vector<Vector2>texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの

	//ファイル読み込み
	std::ifstream file(directoryPath + "/" + filename);//fileを開く

	assert(file.is_open());//開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1 - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			std::vector<std::string> tokensF;
			{
				std::string tok;
				while (s >> tok) tokensF.push_back(tok);
			}
			if (tokensF.size() < 3) { continue; }

			// 工具：保留空段分割 "v/vt/vn" 或 "v//vn" 或 "v/vt"
			auto splitKeepEmpty = [](const std::string& str) -> std::array<std::string, 3> {
				std::array<std::string, 3> out{};
				size_t p1 = str.find('/');
				if (p1 == std::string::npos) { out[0] = str; return out; }
				size_t p2 = str.find('/', p1 + 1);
				out[0] = str.substr(0, p1);
				out[1] = (p2 == std::string::npos) ? str.substr(p1 + 1)
					: str.substr(p1 + 1, p2 - p1 - 1);
				out[2] = (p2 == std::string::npos) ? std::string()
					: str.substr(p2 + 1);
				return out;
				};

			auto parseIndex = [](const std::string& s, int count) -> int {
				if (s.empty()) return -1;            // 缺项
				int idx = std::stoi(s);              // 允许负索引
				if (idx > 0) return idx - 1;         // OBJ 正索引从1开始
				if (idx < 0) return count + idx;     // 负索引：-1 表示最后
				return -1;
				};

			// 解析本面的所有索引
			std::vector<int> vIdx, vtIdx, vnIdx;
			vIdx.reserve(tokensF.size());
			vtIdx.reserve(tokensF.size());
			vnIdx.reserve(tokensF.size());

			for (auto& t : tokensF) {
				auto parts = splitKeepEmpty(t);                // 保留空段
				int vi = parseIndex(parts[0], (int)positions.size());
				int vti = parseIndex(parts[1], (int)texcoords.size()); // 可能为空 → -1
				int vni = parseIndex(parts[2], (int)normals.size());   // 可能为空 → -1
				if (vi < 0 || vi >= (int)positions.size()) {
					// 非法索引，整面跳过
					vIdx.clear(); vtIdx.clear(); vnIdx.clear();
					break;
				}
				vIdx.push_back(vi);
				vtIdx.push_back(vti);
				vnIdx.push_back(vni);
			}
			if (vIdx.size() < 3) { continue; }

			// 三角扇切分 (0, i, i+1)，并按你原来的“逆序入栈”保持绕序
			for (size_t i = 1; i + 1 < vIdx.size(); ++i) {
				int ids[3] = { 0, (int)i, (int)i + 1 };
				VertexData tri[3];
				for (int k = 0; k < 3; ++k) {
					int a = ids[k];
					Vector4 position = positions[vIdx[a]];
					// 你的坐标系在读 v 时已把 x *= -1；这里保持一致即可。:contentReference[oaicite:2]{index=2}

					Vector2 texcoord = { 0.0f, 0.0f };
					if (vtIdx[a] >= 0 && vtIdx[a] < (int)texcoords.size()) {
						texcoord = texcoords[vtIdx[a]];
					}

					Vector3 normal = { 0.0f, 0.0f, 0.0f };
					if (vnIdx[a] >= 0 && vnIdx[a] < (int)normals.size()) {
						normal = normals[vnIdx[a]];
					}

					tri[k] = { position, texcoord, normal };
				}

				// 如果三点法线都为零，计算面法线回填
				auto isZero = [](const Vector3& n) { return n.x == 0 && n.y == 0 && n.z == 0; };
				if (isZero(tri[0].normal) && isZero(tri[1].normal) && isZero(tri[2].normal)) {
					Vector3 e1 = (Vector3{ tri[1].position.x, tri[1].position.y, tri[1].position.z } -
						Vector3{ tri[0].position.x, tri[0].position.y, tri[0].position.z });
					Vector3 e2 = (Vector3{ tri[2].position.x, tri[2].position.y, tri[2].position.z } -
						Vector3{ tri[0].position.x, tri[0].position.y, tri[0].position.z });
					Vector3 n = Math::Normalize(Math::Cross(e1, e2));
					tri[0].normal = tri[1].normal = tri[2].normal = n;
				}

				// 与原代码一致：逆序 push（2,1,0）
				modelData.vertices.push_back(tri[2]);
				modelData.vertices.push_back(tri[1]);
				modelData.vertices.push_back(tri[0]);
			}
		}
		else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}