#include "TransportSupport.h"
//#include <stdlib.h>
//#include <io.h>

TransportSupport::TransportSupport() {
	mRunFlag = true;
};

TransportSupport::~TransportSupport() {
	mRunFlag = false;
	closesocket(mSendSocketFd); // 有问题
	closesocket(mDataSocket);
	mRecvDataCallback = nullptr;
	printf("release resource success\n");
}

int TransportSupport::Init(transportModel modelType) {

	if (BindOrConnect(modelType, mTransportParams.serverIP, mTransportParams.serverPort, mTransportParams.clientIP, mTransportParams.clientPort) < 0) { return -1; }
	if (SetAttrParams() < 0) { return -1; }

	if (modelType == transportModel::SEND_MODEL) {
		if (StartListen() < 0) { return -1; }
	}

	printf("Prepare to transport success\n");
	return 0;
}


int TransportSupport::SetAttrParams() {
	// 设置非阻塞模式
	if (!mBlockFlag) {
		u_long argp = 1; // 置0为阻塞模式
		ioctlsocket(mDataSocket, FIONBIO, &argp);
	}

	return 0;
}

int TransportSupport::SetRecvDataCallback(recvDataBack callback) {
	mRecvDataCallback = callback;
	return 0;
}

