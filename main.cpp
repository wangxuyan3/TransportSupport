// TransportSupportCmake.cpp: 定义应用程序的入口点。
//
#pragma once
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
#define USE_AS_SERVER false
TransportSupport* TCPTransportService = nullptr;

void LoadConfig();
void OnReceiveData(char* Buffer, int singleLength);
void SendDataThread(TransportSupport* transportPtr);

void StartTCPTransportService();
void StopTCPTransportService();


int main() {
	LoadConfig();
	StartTCPTransportService();

	thread* sendThread = new thread(SendDataThread, TCPTransportService);
	sendThread->join();

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
	char* sendData = new char[g_sendParams.sendBytePerFrame];
	memset(sendData, 0, g_sendParams.sendBytePerFrame);
	int flag = 10;
	while (flag) {
		transportPtr->SendData(sendData, g_sendParams.sendBytePerFrame);
		flag--;
		Sleep(g_sendParams.sendSleepTime);
	}
}
