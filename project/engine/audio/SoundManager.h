#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdint.h>

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
    struct SoundData {
        WAVEFORMATEX format{};
        std::vector<BYTE> buffer;
    };

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
