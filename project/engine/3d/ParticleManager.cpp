#include "ParticleManager.h"
#include "Math.h"
#include <assert.h>
#include <numbers>
#include <MyMath.h>

ParticleManager* ParticleManager::instance = nullptr;
//シングルトンインスタンスの取得
ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Initialize(DirectXCommon*directxBase,SrvManager*srvManager, Camera* camera) {
	this->directxBase_ = directxBase;
	this->srvManager_ = srvManager;
	this->camera_ = camera;
	randomEngine.seed(seedGenerator());
	//ルートシグネチャ
	//GenerateRootSignature();
	//グラフィックスパイプライン
	GenerategraphicsPipeline();
	//頂点データ
	VertexDataCreate();
	//マテリアルデータ
	MaterialCreate();
	//Field
	accelerationfield_.acceleration = { 15.0f,0.0f,0.0f };
	accelerationfield_.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationfield_.area.max = { 1.0f,1.0f,1.0f };
}

//終了
void ParticleManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ParticleManager::Update() {
	Matrix4x4 cameraMatrix = camera_->GetWorldMatrix();
	Matrix4x4 backToFrontMatrix = Math::MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = Math::Multiply(backToFrontMatrix, cameraMatrix);
	//ビュー行列
	Matrix4x4 viewMatrix = camera_->GetViewMatrix();
	//プロジェクション行列
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	//ビュープロジェクション行列
	Matrix4x4 viewprojectionMatrix = camera_->GetViewprojectionMatrix();
	//trueなら使う
	if (useBillboard) {
		billboardMatrix.m[0][0] = viewMatrix.m[0][0]; // Right
		billboardMatrix.m[1][0] = viewMatrix.m[0][1];
		billboardMatrix.m[2][0] = viewMatrix.m[0][2];
		billboardMatrix.m[0][1] = viewMatrix.m[1][0]; // Up
		billboardMatrix.m[1][1] = viewMatrix.m[1][1];
		billboardMatrix.m[2][1] = viewMatrix.m[1][2];
		billboardMatrix.m[0][2] = viewMatrix.m[2][0]; // Forward
		billboardMatrix.m[1][2] = viewMatrix.m[2][1];
		billboardMatrix.m[2][2] = viewMatrix.m[2][2];
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
		billboardMatrix.m[3][3] = 1.0f;
	}
	//falseなら単位行列
	else if (!useBillboard) {
		billboardMatrix = Math::MakeIdentity4x4();
	}
	for (auto& ParticleGroups : particleGroups) {
		ParticleGroups.second.kNumInstance = 0;
		for (std::list<Particle>::iterator particleIterator = ParticleGroups.second.particles.begin();
			particleIterator != ParticleGroups.second.particles.end();) {
			(*particleIterator).currentTime += kDeltaTime;
			//寿命に達したらグループから外す
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = ParticleGroups.second.particles.erase(particleIterator);
				continue;
			}
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			if (ParticleGroups.second.kNumInstance < kNumMaxInstance) {
				//行列計算
				// 获取粒子与相机的距离
				Vector3 cameraPos = camera_->GetTransform().translate;
				Vector3 particlePos = (*particleIterator).transform.translate;
				float distance = Math::Length(Math::Subtract(particlePos, cameraPos));

				// 根据距离缩放（你可以调整 5.0f 是越小看起来越大）
				float scaleFactor = 5.0f / max(distance, 0.1f); // 避免除以0
				Vector3 scaled = {
					(*particleIterator).transform.scale.x * scaleFactor,
					(*particleIterator).transform.scale.y * scaleFactor,
					(*particleIterator).transform.scale.z * scaleFactor,
				};

				// 替换原来的 scaleMatrix
				Matrix4x4 scaleMatrix = Math::MakeScaleMatrix(scaled);

				Matrix4x4 translateMatrix = Math::MakeTranslateMatrix((*particleIterator).transform.translate);
				Matrix4x4 worldMatrix = Math::Multiply(Math::Multiply(billboardMatrix, scaleMatrix), translateMatrix);
				Matrix4x4 worldViewProjectionMatrix = Math::Multiply(worldMatrix, viewprojectionMatrix);
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].WVP = worldViewProjectionMatrix;
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].World = worldMatrix;
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].color = (*particleIterator).color;
				//Fieldの範囲内のParticleには加速度を適用する
				if (IsCollision(accelerationfield_.area, (*particleIterator).transform.translate)) {
					(*particleIterator).velocity = Math::Add((*particleIterator).velocity,
						Math::Multiply(accelerationfield_.acceleration, kDeltaTime));
				}
				//速度を適用
				(*particleIterator).transform.translate.x += (*particleIterator).velocity.x * kDeltaTime;
				(*particleIterator).transform.translate.y += (*particleIterator).velocity.y * kDeltaTime;
				(*particleIterator).transform.translate.z += (*particleIterator).velocity.z * kDeltaTime;
				//インスタンシング用データ書き込み
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].WVP = worldViewProjectionMatrix;
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].World = worldMatrix;
				ParticleGroups.second.instancingData[ParticleGroups.second.kNumInstance].color.w = alpha;
				++ParticleGroups.second.kNumInstance;
			}
			++particleIterator;
		}
	}
}

void ParticleManager::Draw() {
	
	//ルートシグネチャ
	directxBase_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	//PSO設定
	directxBase_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	//描画形状設定
	directxBase_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VBV設定
	directxBase_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//パーティクルについて処理
	for (auto& ParticleGroups : particleGroups) {
		//マテリアルCBufferの場所を設定
		directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//テクスチャのSRVのDescriptorTableを設定
		directxBase_->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(ParticleGroups.second.materialData.textureIndex));
		//インスタンシングデータのSRVのDescriptorTableを設定
		directxBase_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(ParticleGroups.second.SRVIndex));
		//描画
		directxBase_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), ParticleGroups.second.kNumInstance,0,0);
	}
}

//パーティクルグループの生成
void ParticleManager::CreateparticleGroup(const std::string name, const std::string textureFilePath) {
	//登録済みの名前かチェック
	assert(particleGroups.find(name) == particleGroups.end());
	ParticleGroup newParticle;
	//particleGroups[name] = newParticle;
	//テクスチャファイルパスを設定
	newParticle.materialData.textureFilePath = textureFilePath;
	//テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	//マテリアルデータにテクスチャSRVインデックスを記録
	newParticle.materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	//インスタンシング用のリソース生成
	newParticle.instancingResource = directxBase_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	//アドレス取得
	newParticle.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&newParticle.instancingData));
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		newParticle.instancingData[index].WVP = Math::MakeIdentity4x4();
		newParticle.instancingData[index].World = Math::MakeIdentity4x4();
		newParticle.instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//SRV生成
	newParticle.SRVIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(newParticle.SRVIndex, newParticle.
		instancingResource.Get(), kNumMaxInstance, sizeof(ParticleForGPU));
	
	particleGroups[name] = newParticle;
}

//パーティクル生成関数
ParticleManager::Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	std::uniform_real_distribution<float>distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = Math::Add(translate, randomTranslate);
	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

//パーティクルの発生
void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
	//登録済みかチェック
	assert(particleGroups.find(name) != particleGroups.end());
	
	//新たなパーティクル作成し、指定されたグループに登録
	for (uint32_t i = 0; i < count; ++i) {
		particleGroups[name].particles.push_back(MakeNewParticle(randomEngine, position));
	}
}

//ParticleがFieldの範囲内か判定
bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point) {
	if ((aabb_.min.x <= point.x && aabb_.max.x >= point.x) &&
		(aabb_.min.y <= point.y && aabb.max.y >= point.y) &&
		(aabb.min.z <= point.z && aabb.max.z >= point.z)) {
		return true;
	}
	else {
		return false;
	}
}

//ルートシグネチャの作成
void ParticleManager::GenerateRootSignature() {
	D3D12_ROOT_SIGNATURE_DESC descriptitonRootSignature{};
	descriptitonRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameter[4] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].Descriptor.ShaderRegister = 0;

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParameter[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameter[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameter[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う

	descriptitonRootSignature.pParameters = rootParameter;
	descriptitonRootSignature.NumParameters = _countof(rootParameter);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptitonRootSignature.pStaticSamplers = staticSamplers;
	descriptitonRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptitonRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = directxBase_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}
//グラフィックスパイプラインの生成
void ParticleManager::GenerategraphicsPipeline() {
	GenerateRootSignature();
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//カリングしない(裏面も表示)
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = directxBase_->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = directxBase_->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）タイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの昨日を有効化にする
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数はLessEqual.つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 実際に生成
	hr = directxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}
//頂点データ作成
void ParticleManager::VertexDataCreate() {
	modelData.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f},.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });//左上
	modelData.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });//右上
	modelData.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });//左下
	modelData.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });//左下
	modelData.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });//右上
	modelData.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f},.texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });//右下
	modelData.material.textureFilePath = "./resources/circle.png";
	//リソースを作る
	vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//VertexResourceにデータを書き込むためのアドレス取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}
//マテリアルデータ作成
void ParticleManager::MaterialCreate() {
	//リソースを作る
	materialResource = directxBase_->CreateBufferResource(sizeof(Material));
	// 書き込むためのアドレスと取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
	//Lighting
	materialData->enableLighting = false;
	//UVTransform行列を単位行列で初期化
	materialData->uvTransform = Math::MakeIdentity4x4();
}