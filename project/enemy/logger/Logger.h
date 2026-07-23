#pragma once
#include <string>
#include "Windows.h"
namespace Logger {
	/// <summary>
	/// Log処理を実行します。
	/// </summary>
	/// <param name="message">出力するログメッセージ。</param>
	void Log(const std::string& message);
}
