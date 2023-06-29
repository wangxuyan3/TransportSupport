#include "TCPTransport.h"

#include <stdio.h>
#include <string>
#include <functional>
#include <iostream>
#include <fstream>

#include "Tools.h"

TCPTransport::~TCPTransport() {
//	mRunFlag = false;
//
//	if (mRecvDataThread->joinable()) {
//		mRecvDataThread->join();
//	}
}

void TCPTransport::LoadConfigFromFile() {
	ifstream configFile("config.txt");

	if (configFile.is_open()) {
		// read context
		string singleLine;
		while (getline(configFile, singleLine)) {
			vector<string> singleInfo;
			StringTools::SplitString(&singleInfo, singleLine, " ");

			auto headerInfo = singleInfo[0];
			if (headerInfo == "ServerIP") { mTransportParams.serverIP = singleInfo[1]; continue; };
			if (headerInfo == "ClientIP") { mTransportParams.clientIP = singleInfo[1]; continue; };
			if (headerInfo == "ServerPort") { mTransportParams.serverPort = stoi(singleInfo[1]); continue; };
			if (headerInfo == "ClientPort") { mTransportParams.clientPort = stoi(singleInfo[1]); continue; };
		}

		configFile.close();
	}
	else {
		printf("open config file error");
	}
}

void TCPTransport::LoadConfig(string serverIP, uint32_t serverPort, string clientIP, uint32_t clientPort) {
	mTransportParams.serverIP = serverIP;
	mTransportParams.clientIP = clientIP;
	mTransportParams.serverPort = serverPort;
	mTransportParams.clientPort = clientPort;
}

int TCPTransport::BindOrConnect(transportModel modelType) {
#if !(defined(__ANDROID__) || defined(ANDROID))
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	//声明sockaddr_in结构体变量，用于构建服务器的IP地址 端口号 地址类型
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));

	if (modelType == transportModel::SEND_MODEL) {
		// 入参校验
		if (mTransportParams.serverIP.empty() || mTransportParams.serverPort == 0) {
			printf("params error\n");
			return -1;
		}

		if ((mSendSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			return -1;
		}

		//初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
		serverAddr.sin_family = AF_INET; //设置的使用地址类型
		serverAddr.sin_port = htons(mTransportParams.serverPort); //设置端口号
		//  //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
		//  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_addr.s_addr = inet_addr(mTransportParams.serverIP.c_str());//设置IP地址

		if (::bind(mSendSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("connect error");
			return -1;
		}
	} else if (modelType == transportModel::RECV_MODEL) {
		// 入参校验
		if (mTransportParams.serverIP == "" || mTransportParams.serverPort == 0) {
			printf("params error\n");
			return -1;
		}

		if ((mDataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)//调用 socket() 函数创建套接字
		{
			perror("socket");
			return 1;
		}

		serverAddr.sin_family = AF_INET;  //设置的使用地址类型
		serverAddr.sin_port = htons(mTransportParams.serverPort); //设置端口号
		//指定服务器端的ip，本地测试：10.8.116.226
		//inet_addr()函数，将点分十进制IP转换成网络字节序IP
		serverAddr.sin_addr.s_addr = inet_addr(mTransportParams.serverIP.c_str());//设置IP地址

		//建立连接
		int flag = 10;
		while (flag) {
			if (connect(mDataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
			{
				perror("connecting...");
				flag--;
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
			else {
				break;
			}
		}

		if (flag != 0) {
			printf("connect to host success\n");
			return 0;
		}
		else {
			return -1;
		}
	} else
	{
		return -1;
	}

	return 0;
}

int TCPTransport::StartListen() {
	//服务器端调用listen()函数后，套接字进入listen状态，开始监听客户端请求
	if (mSendSocketFd < 0) {
		return -1;
	}

	printf("listen client...\n");
	if (listen(mSendSocketFd, 5) < 0)
	{
		perror("listen");
		return -1;
	}

	struct sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr); //缓冲区的clientAddr的长度

	mDataSocket = accept(mSendSocketFd, (struct sockaddr*)&clientAddr, (socklen_t*)&addrLen);

	if (mDataSocket < 0) {
		perror("accept");
		return -1;
	}

	return 0;
}

int TCPTransport::SendData(const char* singleBuffer, uint32_t length) {
	//发送数据
	string lengthStr = to_string(length);
	send(mDataSocket, lengthStr.c_str(), 8, 0);
	send(mDataSocket, singleBuffer, length, 0);
	return 0;
}

int TCPTransport::RecvData() {
	// TODO 准备处理TCP粘包
	while (mRunFlag) {
		// 接收长度信息
		uint32_t lengthInfoTotalLength = 8;
		uint32_t recvLengthInfoLength = 0;
		int32_t singleLengthInfoLength = 0;
		memset(m_lengthInfoBufferPtr, 0, 8);
		while (mRunFlag && (lengthInfoTotalLength - recvLengthInfoLength) != 0) {

			singleLengthInfoLength = recv(mDataSocket, m_lengthInfoBufferPtr + recvLengthInfoLength, lengthInfoTotalLength - recvLengthInfoLength, 0);
			if (singleLengthInfoLength <= 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			recvLengthInfoLength += singleLengthInfoLength;
		}
		uint32_t dataTotalLength = strtoul(m_lengthInfoBufferPtr, nullptr, 10);

		uint32_t recvDataLength = 0;
		int32_t singleDataLength = 0;
		while (mRunFlag && (dataTotalLength - recvDataLength) != 0) {
			singleDataLength = recv(mDataSocket, m_recvBufferPtr + recvDataLength, dataTotalLength - recvDataLength, 0); //接收数据
			if (singleDataLength <= 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			recvDataLength += singleDataLength;
		}

		if (mRunFlag) {
			mRecvDataCallback(m_recvBufferPtr, dataTotalLength);
		}
	}

	return 0;
}

int TCPTransport::RunRecvDataThread() {
	mRecvDataThread = new thread(std::bind(&TCPTransport::RecvData, this));

	return 0;
}