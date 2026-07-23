#pragma once
#include <xaudio2.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdint.h>

namespace MyEngine {

/// <summary>
/// SoundManagerに関する処理と状態を管理するクラスです。
/// </summary>
class SoundManager {
public:
    /// <summary>
    /// Instanceを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    static SoundManager* GetInstance();

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    void Initialize();
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// Wavを読み込みます。
    /// </summary>
    /// <param name="name">処理に使用する参照値。</param>
    /// <param name="filepath">対象ファイルまたはリソースのパス。</param>
    void LoadWav(const std::string& name, const std::string& filepath);
    /// <summary>
    /// Mp 3を読み込みます。
    /// </summary>
    /// <param name="name">処理に使用する参照値。</param>
    /// <param name="filepath">対象ファイルまたはリソースのパス。</param>
    void LoadMp3(const std::string& name, const std::string& filepath);

    /// <summary>
    /// Play処理を実行します。
    /// </summary>
    /// <param name="name">処理に使用する参照値。</param>
    /// <param name="loop">処理に使用するloopの値。</param>
    /// <param name="volume">処理に使用するvolumeの値。</param>
    /// <returns>計算または取得した数値。</returns>
    int Play(const std::string& name, bool loop = false, float volume = 1.0f);
    /// <summary>
    /// Pause処理を実行します。
    /// </summary>
    /// <param name="voiceId">処理に使用するvoiceIdの値。</param>
    void Pause(int voiceId);
    /// <summary>
    /// Resume処理を実行します。
    /// </summary>
    /// <param name="voiceId">処理に使用するvoiceIdの値。</param>
    void Resume(int voiceId);
    /// <summary>
    /// Stop処理を実行します。
    /// </summary>
    /// <param name="voiceId">処理に使用するvoiceIdの値。</param>
    void Stop(int voiceId);
    /// <summary>
    /// Volumeを設定します。
    /// </summary>
    /// <param name="voiceId">処理に使用するvoiceIdの値。</param>
    /// <param name="volume">処理に使用するvolumeの値。</param>
    void SetVolume(int voiceId, float volume);

private:
    /// <summary>
    /// SoundDataで使用する関連データをまとめて保持する構造体です。
    /// </summary>
    struct SoundData {
        WAVEFORMATEX format{};
        std::vector<BYTE> buffer;
    };

    /// <summary>
    /// VoiceInstanceで使用する関連データをまとめて保持する構造体です。
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
