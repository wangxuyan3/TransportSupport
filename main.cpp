// TransportSupportCmake.cpp: 定义应用程序的入口点。
//
#pragma once
#include "TCPTransport.h"
#include "Tools.h"

using namespace std;

// 控制启动server或client端
#define USE_AS_SERVER false
TransportSupport* TCPTransportService = nullptr;

void OnReceiveData(char* Buffer, int singleLength);
void SendDataThread(TransportSupport* transportPtr);

void StartTCPTransportService();
void StopTCPTransportService();


int main() {
	StartTCPTransportService();

	thread* sendThread = new thread(SendDataThread, TCPTransportService);
	sendThread->join();

	StopTCPTransportService();
	getchar();
}

void StartTCPTransportService() {
	TCPTransportService = new TCPTransport();
	TCPTransportService->LoadConfig();

	TCPTransportService->SetRecvDataCallback(OnReceiveData);

#if USE_AS_SERVER
	TCPTransportService->Init(transportModel::SEND_MODEL);
#else
	TCPTransportService->Init(transportModel::RECV_MODEL);
#endif

	TCPTransportService->RunRecvDataThread();
}

void StopTCPTransportService() {
	printf("prepare to release source\n");
	delete dynamic_cast<TCPTransport*>(TCPTransportService);
	TCPTransportService = nullptr;
}

void OnReceiveData(char* Buffer, int singleLength) {
	printf("length:%d\n", singleLength);
	return;
}

void SendDataThread(TransportSupport* transportPtr) {
	// TODO 待完成从配置文件读取码率及帧率控制发送
	// 准备数据并发送
	string myData = "1234567890";
	int flag = 10;
	while (flag) {
		transportPtr->SendData(myData.c_str());
		flag--;
		Sleep(1000);
	}
}
