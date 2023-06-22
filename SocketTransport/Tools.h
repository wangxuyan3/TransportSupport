#pragma once
#include <string>
#include <vector>

using namespace std;
// 字符串处理
class StringTools {
public:
	// 字符串 targetStr 按 splitMark 进行拆分,
	// 拆分结果输出于 dstSplit. 
	// splitAll 为 false 时只分割首次出现字段， 默认全部分割
	static void SplitString(vector<string>* dstSplit,  string targetStr, string splitMark, bool splitAll = true);
private:

};