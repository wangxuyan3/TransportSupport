#pragma once
#include <string>
#include <vector>

using namespace std;
// �ַ�������
class StringTools {
public:
	// �ַ��� targetStr �� splitMark ���в��,
	// ��ֽ������� dstSplit. 
	// splitAll Ϊ false ʱֻ�ָ��״γ����ֶΣ� Ĭ��ȫ���ָ�
	static void SplitString(vector<string>* dstSplit,  string targetStr, string splitMark, bool splitAll = true);
private:

};