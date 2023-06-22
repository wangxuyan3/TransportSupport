#pragma once
#include <ws2tcpip.h>
#include <winsock.h>
//#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include <string>
#include <thread>

using namespace std;

typedef void(*recvDataBack)(char*, int);

struct transportParams {
	string serverIP;
	string clientIP;

	uint32_t serverPort{ 0 };
	uint32_t clientPort{ 0 };

	uint32_t sendBitRate{ 0 };
	uint32_t sendFrameRate{ 0 };
};

struct transportAttrParams {
	recvDataBack* recvDataCallback = nullptr;
};

enum class transportModel {
	SEND_MODEL = 1,
	RECV_MODEL = 2
};

class TransportSupport {
public:
	virtual void LoadConfig() = 0;
	virtual int Init(transportModel modelType);
	virtual int SendData(const char* singleBuffer) = 0;
	virtual int RecvData() = 0;
	virtual int RunRecvDataThread() = 0;
	int SetRecvDataCallback(recvDataBack callback);

protected:
	TransportSupport();
	virtual ~TransportSupport();

	virtual int BindOrConnect(transportModel modelType, string serverIP, int serverPort, string clientIP, int clientPort) = 0;
	virtual int SetAttrParams(); // 模板类中根据参数设置是否阻塞模式
	virtual int StartListen() = 0;

	int mSendSocketFd{ -1 };
	int mDataSocket{ -1 };

	bool mBlockFlag{ true };
	bool mRunFlag{ false };
	recvDataBack mRecvDataCallback = nullptr;
	thread* recvDataThread = nullptr;
	transportParams mTransportParams;
};