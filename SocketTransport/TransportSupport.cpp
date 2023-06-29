#include "TransportSupport.h"
//#include <stdlib.h>
//#include <io.h>

TransportSupport::TransportSupport() {
	mRunFlag = true;
};

TransportSupport::~TransportSupport() {
	mRunFlag = false;
	if (mRecvDataThread->joinable()) {
		mRecvDataThread->join();
	}

#if defined(__ANDROID__) || defined(ANDROID)
	close(mSendSocketFd);
	close(mDataSocket);
#else
	closesocket(mSendSocketFd);
	closesocket(mDataSocket);
#endif
	mRecvDataCallback = nullptr;
	printf("release resource success\n");
}

int TransportSupport::Init(transportModel modelType) {

	if (BindOrConnect(modelType) < 0) { return -1; }
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
#if defined(__ANDROID__) || defined(ANDROID)
		int flags = fcntl(mDataSocket, F_GETFL, 0);
		fcntl(mDataSocket, F_SETFL, flags | O_NONBLOCK);
#else
		ioctlsocket(mDataSocket, FIONBIO, &argp);
#endif
	}

	return 0;
}

int TransportSupport::SetRecvDataCallback(recvDataBack callback) {
	mRecvDataCallback = callback;
	return 0;
}

