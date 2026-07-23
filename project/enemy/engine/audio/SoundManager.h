#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdint.h>

namespace MyEngine {

/// <summary>
/// 音声ファイルの読み込み、再生、停止、音量調整、解放を一元管理するクラス。
/// </summary>
class SoundManager {
public:
    static SoundManager* GetInstance();

    void Initialize();
    void Finalize();

    void LoadWav(const std::string& name, const std::string& filepath);
    void LoadMp3(const std::string& name, const std::string& filepath);

    int Play(const std::string& name, bool loop = false, float volume = 1.0f);
    void Pause(int voiceId);
    void Resume(int voiceId);
    void Stop(int voiceId);
    void SetVolume(int voiceId, float volume);

private:
    /// <summary>
    /// 読み込んだ音声の波形フォーマットとバッファを保持する構造体。
    /// </summary>
    struct SoundData {
        WAVEFORMATEX format{};
        std::vector<BYTE> buffer;
    };

    /// <summary>
    /// 再生中のXAudio2ボイスと音量などの状態を保持する構造体。
    /// </summary>
    struct VoiceInstance {
        IXAudio2SourceVoice* sourceVoice = nullptr;
        std::string name;
        bool isPlaying = false;
        float volume = 1.0f;
    };

    IXAudio2* xAudio2_ = nullptr;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;
    std::unordered_map<std::string, SoundData> sounds_;
    std::unordered_map<int, VoiceInstance> activeVoices_;
    int voiceIdCounter_ = 0;
};


} // namespace MyEngine
