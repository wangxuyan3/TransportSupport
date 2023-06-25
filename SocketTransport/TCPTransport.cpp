#include "TCPTransport.h"

#include <stdio.h>
#include <string>
#include <functional>
#include <iostream>
#include <fstream>

#include "Tools.h"

void TCPTransport::LoadConfig() {
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

int TCPTransport::BindOrConnect(transportModel modelType, string serverIP, int serverPort, string clientIP, int clientPort) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//声明sockaddr_in结构体变量，用于构建服务器的IP地址 端口号 地址类型
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));

	if (modelType == transportModel::SEND_MODEL) {
		// 入参校验
		if (serverIP == "" || serverPort == 0) {
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
		serverAddr.sin_port = htons(serverPort); //设置端口号
		//  //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
		//  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());//设置IP地址

		if (::bind(mSendSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("connect error");
			return -1;
		}
	} else if (modelType == transportModel::RECV_MODEL) {
		// 入参校验
		if (serverIP == "" || serverPort == 0) {
			printf("params error\n");
			return -1;
		}

		if ((mDataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)//调用 socket() 函数创建套接字
		{
			perror("socket");
			return 1;
		}

		serverAddr.sin_family = AF_INET;  //设置的使用地址类型
		serverAddr.sin_port = htons(serverPort); //设置端口号
		//指定服务器端的ip，本地测试：10.8.116.226
		//inet_addr()函数，将点分十进制IP转换成网络字节序IP
		serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());//设置IP地址

		//建立连接
		int flag = 10;
		while (flag) {
			if (connect(mDataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
			{
				perror("connecting...");
				flag--;
				Sleep(1000);
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
		int32_t lengthInfoLength = 0;
		memset(m_lengthInfoBufferPtr, 0, 8);
		while (lengthInfoLength <= 0) {
			lengthInfoLength = recv(mDataSocket, m_lengthInfoBufferPtr, 8, 0);
			Sleep(1);
			continue;
		}
		uint32_t dataTotalLength = strtoul(m_lengthInfoBufferPtr, nullptr, 10);

		uint32_t recvLength = 0;
		int32_t dataLength = 0;
		while (dataTotalLength - recvLength) {
			dataLength = recv(mDataSocket, m_recvBufferPtr + recvLength, 8 * 1024 * 1024, 0); //接收数据
			if (dataLength <= 0) {
				Sleep(1);
				continue;
			}
			recvLength += dataLength;
		}

		mRecvDataCallback(m_recvBufferPtr, dataTotalLength);
	}

	return 0;
}

int TCPTransport::RunRecvDataThread() {
	recvDataThread = new thread(std::bind(&TCPTransport::RecvData, this));

	return 0;
}