#pragma once
#include <string>
#include "Windows.h"
namespace MyEngine {

namespace StringUtility {
	/// <summary>
	/// Stringへ変換します。
	/// </summary>
	/// <param name="str">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	std::wstring ConvertString(const std::string& str);
	/// <summary>
	/// Stringへ変換します。
	/// </summary>
	/// <param name="str">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	std::string ConvertString(const std::wstring& str);

}

} // namespace MyEngine
