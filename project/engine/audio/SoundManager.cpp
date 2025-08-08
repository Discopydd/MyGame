#include "SoundManager.h"
#include <fstream>
#include <cassert>
#include <iostream>

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

// ----------------- Singleton -------------------
SoundManager* SoundManager::GetInstance() {
    static SoundManager instance;
    return &instance;
}

// ----------------- Init / Finalize -------------
void SoundManager::Initialize() {
    HRESULT hr = XAudio2Create(&xAudio2_, 0);
    assert(SUCCEEDED(hr));
    hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
    assert(SUCCEEDED(hr));
}

void SoundManager::Finalize() {
    for (auto& [_, voice] : activeVoices_) {
        if (voice.sourceVoice) {
            voice.sourceVoice->Stop();
            voice.sourceVoice->DestroyVoice();
        }
    }
    activeVoices_.clear();

    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    if (xAudio2_) {
        xAudio2_->Release();
        xAudio2_ = nullptr;
    }

    sounds_.clear();
}

// ----------------- WAV -------------------------
struct ChunkHeader {
    char id[4];
    uint32_t size;
};

struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};

struct FormatChunk {
    ChunkHeader chunk;
    WAVEFORMATEX fmt;
};

void SoundManager::LoadWav(const std::string& name, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    assert(file.is_open());

    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
    assert(strncmp(riff.type, "WAVE", 4) == 0);

    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    assert(strncmp(format.chunk.id, "fmt ", 4) == 0);
    file.read((char*)&format.fmt, format.chunk.size);

    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
    while (strncmp(data.id, "data", 4) != 0) {
        file.seekg(data.size, std::ios_base::cur);
        file.read((char*)&data, sizeof(data));
    }

    SoundData sound{};
    sound.format = format.fmt;
    sound.buffer.resize(data.size);
    file.read((char*)sound.buffer.data(), data.size);
    file.close();

    sounds_[name] = std::move(sound);
}

// ----------------- MP3 -------------------------
void SoundManager::LoadMp3(const std::string& name, const std::string& filepath) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, filepath.c_str(), nullptr)) {
        assert(0 && "MP3 Load Failed");
        return;
    }

    drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
    std::vector<int16_t> pcmData(frameCount * mp3.channels);
    drmp3_read_pcm_frames_s16(&mp3, frameCount, pcmData.data());

    SoundData sound{};
    sound.format.wFormatTag = WAVE_FORMAT_PCM;
    sound.format.nChannels = mp3.channels;
    sound.format.nSamplesPerSec = mp3.sampleRate;
    sound.format.wBitsPerSample = 16;
    sound.format.nBlockAlign = (sound.format.nChannels * sound.format.wBitsPerSample) / 8;
    sound.format.nAvgBytesPerSec = sound.format.nSamplesPerSec * sound.format.nBlockAlign;
    sound.format.cbSize = 0;

    sound.buffer.resize(pcmData.size() * sizeof(int16_t));
    memcpy(sound.buffer.data(), pcmData.data(), sound.buffer.size());

    drmp3_uninit(&mp3);
    sounds_[name] = std::move(sound);
}

// ----------------- 播放 / 控制 -------------------
int SoundManager::Play(const std::string& name, bool loop, float volume) {
    auto it = sounds_.find(name);
    if (it == sounds_.end()) return -1;

    const SoundData& sound = it->second;
    IXAudio2SourceVoice* voice = nullptr;
    HRESULT hr = xAudio2_->CreateSourceVoice(&voice, &sound.format);
    assert(SUCCEEDED(hr));

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = (UINT32)sound.buffer.size();
    buffer.pAudioData = sound.buffer.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    voice->SetVolume(volume);
    voice->SubmitSourceBuffer(&buffer);
    voice->Start();

    int id = voiceIdCounter_++;
    activeVoices_[id] = { voice, name, true, volume };
    return id;
}

void SoundManager::Pause(int voiceId) {
    auto it = activeVoices_.find(voiceId);
    if (it != activeVoices_.end()) {
        it->second.sourceVoice->Stop();
        it->second.isPlaying = false;
    }
}

void SoundManager::Resume(int voiceId) {
    auto it = activeVoices_.find(voiceId);
    if (it != activeVoices_.end()) {
        it->second.sourceVoice->Start();
        it->second.isPlaying = true;
    }
}

void SoundManager::Stop(int voiceId) {
    auto it = activeVoices_.find(voiceId);
    if (it != activeVoices_.end()) {
        it->second.sourceVoice->Stop();
        it->second.sourceVoice->DestroyVoice();
        activeVoices_.erase(it);
    }
}

void SoundManager::SetVolume(int voiceId, float volume) {
    auto it = activeVoices_.find(voiceId);
    if (it != activeVoices_.end()) {
        it->second.sourceVoice->SetVolume(volume);
        it->second.volume = volume;
    }
}
