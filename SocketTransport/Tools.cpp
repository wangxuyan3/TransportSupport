#include "Tools.h"

void StringTools::SplitString(vector<string>* dstSplit, string targetStr, string splitMark, bool splitAll) {
	size_t markLength = splitMark.length();
	size_t res = targetStr.find(splitMark);
	if (res != string::npos) {
		dstSplit->push_back(targetStr.substr(0, res));
		if (!splitAll) {
			dstSplit->push_back(targetStr.substr(res + markLength, targetStr.length() - res - markLength));
			return;
		}
		StringTools::SplitString(dstSplit, targetStr.substr(res + markLength, targetStr.length() - res - markLength), splitMark, splitAll);
	} else {
		dstSplit->push_back(targetStr);
		return;
	}
}