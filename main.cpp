// TransportSupportCmake.cpp: 定义应用程序的入口点。
//
#include <iostream>
#include <fstream>

#include "TCPTransport.h"
#include "Tools.h"

using namespace std;

struct sendParams {
	uint32_t sendBitRate = 0;
	uint32_t sendFrameRate = 30;

	uint32_t sendBytePerFrame = 0;
	uint32_t sendSleepTime = 33; // 默认30帧
} g_sendParams;

// 控制启动server或client端
bool g_serverTag = true;
bool g_runFlag = true;
TransportSupport* TCPTransportService = nullptr;
thread* g_TCPServiceThread = nullptr;
thread* g_sendThread = nullptr;

void LoadConfig();
void OnReceiveData(char* Buffer, int singleLength);
void SendDataThread();

void StartTCPTransportService();
void StopTCPTransportService();


int main() {
	LoadConfig();
	g_TCPServiceThread = new thread(StartTCPTransportService);
	g_sendThread = new thread(SendDataThread);

	if (g_sendThread->joinable()) {
		g_sendThread->join();
	}

	StopTCPTransportService();
	getchar();
}

void LoadConfig() {
	ifstream configFile("config.txt");

	if (configFile.is_open()) {
		// read context
		string singleLine;
		while (getline(configFile, singleLine)) {
			vector<string> singleInfo;
			StringTools::SplitString(&singleInfo, singleLine, " ");

			auto headerInfo = singleInfo[0];
			if (headerInfo == "SendBitRate") { g_sendParams.sendBitRate = stoi(singleInfo[1]); continue; };
			if (headerInfo == "SendFrameRate") { g_sendParams.sendFrameRate = stoi(singleInfo[1]); continue; };
		}

		configFile.close();
	}
	else {
		printf("open config file error");
	}

	printf("g_sendParams.sendBitRate: %lu\n", g_sendParams.sendBitRate);
	printf("g_sendParams.sendFrameRate: %lu\n", g_sendParams.sendFrameRate);

	g_sendParams.sendSleepTime = 1000 / g_sendParams.sendFrameRate;
	g_sendParams.sendBytePerFrame = g_sendParams.sendBitRate * 1000 / 8 / g_sendParams.sendFrameRate;
}

// 传输模块
void StartTCPTransportService() {
	TCPTransportService = new TCPTransport();
	TCPTransportService->LoadConfigFromFile();
	TCPTransportService->SetRecvDataCallback(OnReceiveData);
	if (g_serverTag) {
		TCPTransportService->Init(transportModel::SEND_MODEL);
	}
	else {
		TCPTransportService->Init(transportModel::RECV_MODEL);
	}
	TCPTransportService->RunRecvDataThread();
}

void StopTCPTransportService() {
	printf("prepare to release source\n");
	delete dynamic_cast<TCPTransport*>(TCPTransportService);
	TCPTransportService = nullptr;
}

void OnReceiveData(char* Buffer, int singleLength) {
    static uint32_t recvIndex = 0;

    if (recvIndex % g_sendParams.sendFrameRate == 0) {
        printf("Receive Index: %u, length:%d\n", recvIndex, singleLength);
    }

    ++recvIndex;
}

void SendDataThread() {
	if (g_TCPServiceThread->joinable()) {
		g_TCPServiceThread->join();
	}
	// 准备数据并发送
	char* sendData = new char[g_sendParams.sendBytePerFrame];
	memset(sendData, 0, g_sendParams.sendBytePerFrame);
	//    int flag = 10;
	while (g_runFlag) {
		TCPTransportService->SendData(sendData, g_sendParams.sendBytePerFrame);
		//        flag--;
		std::this_thread::sleep_for(std::chrono::milliseconds(g_sendParams.sendSleepTime));
	}
}
