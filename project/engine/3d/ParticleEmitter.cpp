#include "ParticleEmitter.h"

//初期化
void ParticleEmitter::Initialize(std::string name) {
	name_ = name;
	emitter_.count = 1;
	emitter_.frequency = 0.1f;//0.1秒ごとに発生
	emitter_.frequencyTime = 0.0f;
	emitter_.transform.translate = { 0.0f,0.0f,0.0f };
	emitter_.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter_.transform.scale = { 0.5f,0.5f,0.5f };
}

//更新
void ParticleEmitter::Update() {
	//時刻を進める
	emitter_.frequencyTime += kDeltaTime;
	//発生頻度より大きければ発生
	if (isAutoEmit &&emitter_.frequency <= emitter_.frequencyTime) {
		ParticleManager::GetInstance()->Emit(name_, emitter_.transform.translate, emitter_.count);
		//余計に過ぎた時間を加味して頻度計算
		emitter_.frequencyTime -= emitter_.frequency;
	}
}
//パーティクルの発生
void ParticleEmitter::Emit() {
	ParticleManager::GetInstance()->Emit(name_, emitter_.transform.translate, emitter_.count);
}