#include "TransportSupport.h"
//#include <stdlib.h>
//#include <io.h>

TransportSupport::TransportSupport() {
	mRunFlag = true;
};

TransportSupport::~TransportSupport() {
	mRunFlag = false;
	closesocket(mSendSocketFd); // ������
	closesocket(mRecvSocketFd);
	mRecvDataCallback = nullptr;
	printf("release resource success\n");
}


int TransportSupport::Init(transportParams params, transportModel modelType) {


	if (BindOrConnect(modelType, params.localIp, params.localPort, params.remoteIp, params.remotePort) < 0) { return -1; }

	if (modelType == transportModel::SEND_MODEL) {
		if (StartListen() < 0) { return -1; }
	} else if (modelType == transportModel::RECV_MODEL) {
		if (SetAttrParams() < 0) { return -1; }
	} else {
		return -1;
	}

	printf("Prepare to transport success\n");
	return 0;
}


int TransportSupport::SetAttrParams() {
	// ���÷�����ģʽ
	if (!mBlockFlag) {
		u_long argp = 1; // ��0Ϊ����ģʽ
		ioctlsocket(mRecvSocketFd, FIONBIO, &argp);
	}

	return 0;
}

int TransportSupport::SetRecvDataCallback(recvDataBack callback) {
	mRecvDataCallback = callback;

	return 0;
}

