#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "logger/Logger.h"
#include "Camera.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <unordered_map>
#include <string>
#include <random>


class ParticleManager
{
public:
	//頂点データ
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};
	//マテリアルデータ
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	struct ModelData
	{
		std::vector<VertexData>vertices;
		MaterialData material;
	};
	//パーティクル
	struct Particle {
		Transform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};
	struct ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
	};
	struct AABB {
		Vector3 min;//最小点
		Vector3 max;//最大点
	};
	struct AccelerationField {
		Vector3 acceleration;//加速度
		AABB area;//範囲
	};
	struct ParticleGroup {
		MaterialData materialData;//マテリアルデータ
		std::list<Particle>particles;//パーティクルリスト
		uint32_t SRVIndex;//インスタンシングデータ用SRVインデックス
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;//インスタンシングリソース
		uint32_t kNumInstance;//インスタンス数
		ParticleForGPU* instancingData ;//インスタンシングデータを書き込むためのポインタ
	};
public:
	//シングルトンインスタンスの取得
	static ParticleManager* GetInstance();

	void Initialize(DirectXCommon* directxBase, SrvManager* srvManager, Camera* camera);
	//終了
	void Finalize();

	void Update();

	void Draw();
	//パーティクルグループの生成
	void CreateparticleGroup(const std::string name, const std::string textureFilePath);
	//パーティクル生成関数
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);
	//パーティクルの発生
	void Emit(const std::string name, const Vector3& position, uint32_t count);
	//ParticleがFieldの範囲内か判定
	bool IsCollision(const AABB& aabb, const Vector3& point);

	void SetBillboard(bool use) { useBillboard = use; }
private:
	//ルートシグネチャの作成
	void GenerateRootSignature();
	//グラフィックスパイプラインの生成
	void GenerategraphicsPipeline();
	//頂点データ作成
	void VertexDataCreate();
	//マテリアルデータ作成
	void MaterialCreate();
private:

	static ParticleManager* instance;
	DirectXCommon* directxBase_;
	SrvManager* srvManager_;
	Camera* camera_;
	ModelData modelData;
	AABB aabb_;
	AccelerationField accelerationfield_;
	HRESULT hr;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//乱数生成器の初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine;
	//グループコンテナ
	std::unordered_map<std::string, ParticleGroup>particleGroups;

	const uint32_t kNumMaxInstance =1024;
	//Δtを定義
	const float kDeltaTime = 1.0f / 60.0f;
	//billboardMatrix切り替え
	bool useBillboard = false;
};

