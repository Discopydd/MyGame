#pragma once
#include "ModelCommon.h"
#include "MaterialData.h"
#include"ModelData.h"
#include"Material.h"
#include"VertexData.h"
namespace MyEngine {

/// <summary>
/// Modelに関する処理と状態を管理するクラスです。
/// </summary>
class Model
{
public:
	// 初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="modeleCommon">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="directorypath">対象ファイルまたはリソースのパス。</param>
	/// <param name="filename">処理に使用する参照値。</param>
	void Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename);

	// 描画
	/// <summary>
	/// 現在の状態を画面へ描画します。
	/// </summary>
	void Draw();

	/// <summary>
	/// Material Template Fileを読み込みます。
	/// </summary>
	/// <param name="directorypath">対象ファイルまたはリソースのパス。</param>
	/// <param name="filename">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
	/// <summary>
	/// Object Fileを読み込みます。
	/// </summary>
	/// <param name="ditrectoryPath">対象ファイルまたはリソースのパス。</param>
	/// <param name="filename">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	static ModelData LoadObjectFile(const std::string& ditrectoryPath, const std::string& filename);
	/// <summary>
	/// Enable Lightingを設定します。
	/// </summary>
	/// <param name="enable">機能を有効にする場合は true。</param>
	void SetEnableLighting(bool enable) {
		if (materialData) {
			materialData->enableLighting = enable;
		}
	}
	/// <summary>
	/// Enable Lightingを設定します。
	/// </summary>
	/// <param name="enable">機能を有効にする場合は true。</param>
	void SetEnableLighting(int enable)
	{
		if (materialData) {
			materialData->enableLighting = enable;
		}
	}
	/// <summary>
	/// Enable Lightingを取得します。
	/// </summary>
	/// <returns>判定結果。</returns>
	bool GetEnableLighting() const {
		return materialData ? materialData->enableLighting : false;
	}
	/// <summary>
	/// Colorを設定します。
	/// </summary>
	/// <param name="color">処理に使用する参照値。</param>
	void SetColor(const Vector4& color) {
		if (materialData) {
			materialData->color = color;
		}
	}

	/// <summary>
	/// Colorを取得します。
	/// </summary>
	/// <returns>計算または取得した結果。</returns>
	Vector4 GetColor() const {
		if (materialData) {
			return materialData->color;
		}
		return { 1.0f, 1.0f, 1.0f, 1.0f };
	}
private:
	// モデル
	ModelCommon* modelCommon_ = nullptr;
	//objファイルのデータ
	ModelData modelData;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// マテリアル用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルにデータを書き込む	
	Material* materialData = nullptr;
};

} // namespace MyEngine
