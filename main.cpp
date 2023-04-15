// TransportSupportCmake.cpp: 定义应用程序的入口点。
//
#pragma once
#include "./SocketTransport/TCPTransport.h"

using namespace std;

// 控制启动server或client端
bool g_serverFlag = true;
bool g_clientFlag = false;
TCPTransport* TCPTransportExec = nullptr;

void StartTCPServer();
void StartTCPClient();

int main() {
	if (g_serverFlag) {
		StartTCPServer();
	}

	if (g_clientFlag) {
		StartTCPClient();
	}
}

void StartTCPServer() {
	TCPTransportExec = new TCPTransport();
	transportParams selfParams;
	selfParams.localIp = "127.0.0.1";
	selfParams.remoteIp = "127.0.0.1";
	selfParams.localPort = 13000;
	selfParams.remotePort = 14000;

	// 发送端初始化
	TCPTransportExec->Init(selfParams, transportModel::SEND_MODEL);

	// 准备数据并发送
	string myData = "1234567890";
	int flag = 10;
	while (flag) {
		TCPTransportExec->SendData(myData.c_str());
		flag--;
		Sleep(1000);
	}

	// 资源释放
	printf("prepare to release source\n");
	delete TCPTransportExec;
	TCPTransportExec = nullptr;
}

void OnReceiveData(char* Buffer, int singleLength) {
	printf("length:%d\n", singleLength);
	return;
}

void StartTCPClient() {
	TCPTransportExec = new TCPTransport();

	// 接收端
	transportParams selfParams;
	selfParams.localIp = "127.0.0.1";
	selfParams.remoteIp = "127.0.0.1";
	selfParams.localPort = 14000;
	selfParams.remotePort = 13000;
	TCPTransportExec->SetRecvDataCallback(OnReceiveData);
	TCPTransportExec->Init(selfParams, transportModel::RECV_MODEL);
	TCPTransportExec->RunRecvDataThread();

	// 执行5s后退出
	Sleep(5000);

	// 资源释放
	printf("prepare to release source\n");
	delete TCPTransportExec;
	TCPTransportExec = nullptr;
}
