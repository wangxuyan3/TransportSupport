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
	string localIp;
	string remoteIp;

	int localPort{ 0 };
	int remotePort{ 0 };
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
	virtual int Init(transportParams params, transportModel modelType);
	virtual int SendData(const char* singleBuffer) = 0;
	virtual int RecvData() = 0;
	virtual int RunRecvDataThread() = 0;
	int SetRecvDataCallback(recvDataBack callback);

protected:
	TransportSupport();
	virtual ~TransportSupport();

	virtual int BindOrConnect(transportModel modelType, string localIp, int localPort, string remoteIp, int remotePort) = 0;
	virtual int SetAttrParams(); // 模板类中根据参数设置是否阻塞模式
	virtual int StartListen() = 0;

	int mSendSocketFd{ -1 };
	int mRecvSocketFd{ -1 };
	int mClientSocket{ -1 };
	bool mBlockFlag{ true };
	bool mRunFlag{ false };
	recvDataBack mRecvDataCallback = nullptr;
	thread* recvDataThread = nullptr;
};