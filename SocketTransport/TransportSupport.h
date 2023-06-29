#pragma once
#if defined(__ANDROID__) || defined(ANDROID)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#else
#include <ws2tcpip.h>
#include <winsock.h>
//#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
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
	virtual void LoadConfigFromFile() = 0;
	virtual void LoadConfig(string serverIP, uint32_t serverPort, string clientIP, uint32_t clientPort) = 0;
	virtual int Init(transportModel modelType);
	virtual int SendData(const char* singleBuffer, uint32_t length) = 0;
	virtual int RecvData() = 0;
	virtual int RunRecvDataThread() = 0;
	int SetRecvDataCallback(recvDataBack callback);

protected:
	TransportSupport();
	virtual ~TransportSupport();

	virtual int BindOrConnect(transportModel modelType) = 0;
	virtual int SetAttrParams(); // ģ�����и��ݲ��������Ƿ�����ģʽ
	virtual int StartListen() = 0;

	int mSendSocketFd{ -1 };
	int mDataSocket{ -1 };

	bool mBlockFlag{ true };
	bool mRunFlag{ false };
	recvDataBack mRecvDataCallback = nullptr;
	thread* mRecvDataThread = nullptr;
	transportParams mTransportParams;
};