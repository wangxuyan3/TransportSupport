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

	//����sockaddr_in�ṹ����������ڹ�����������IP��ַ �˿ں� ��ַ����
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));

	if (modelType == transportModel::SEND_MODEL) {
		// ���У��
		if (serverIP == "" || serverPort == 0) {
			printf("params error\n");
			return -1;
		}

		if ((mSendSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			return -1;
		}

		//��ʼ���������˵��׽��֣�����htons��htonl���˿ں͵�ַת�������ֽ���
		serverAddr.sin_family = AF_INET; //���õ�ʹ�õ�ַ����
		serverAddr.sin_port = htons(serverPort); //���ö˿ں�
		//  //ip�����Ǳ���������ip��Ҳ�����ú�INADDR_ANY���棬����0.0.0.0���������е�ַ
		//  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());//����IP��ַ

		if (::bind(mSendSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("connect error");
			return -1;
		}
	} else if (modelType == transportModel::RECV_MODEL) {
		// ���У��
		if (serverIP == "" || serverPort == 0) {
			printf("params error\n");
			return -1;
		}

		if ((mDataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)//���� socket() ���������׽���
		{
			perror("socket");
			return 1;
		}

		serverAddr.sin_family = AF_INET;  //���õ�ʹ�õ�ַ����
		serverAddr.sin_port = htons(serverPort); //���ö˿ں�
		//ָ���������˵�ip�����ز��ԣ�10.8.116.226
		//inet_addr()�����������ʮ����IPת���������ֽ���IP
		serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());//����IP��ַ

		//��������
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
	//�������˵���listen()�������׽��ֽ���listen״̬����ʼ�����ͻ�������
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
	int addrLen = sizeof(clientAddr); //��������clientAddr�ĳ���

	mDataSocket = accept(mSendSocketFd, (struct sockaddr*)&clientAddr, (socklen_t*)&addrLen);

	if (mDataSocket < 0) {
		perror("accept");
		return -1;
	}

	return 0;
}

int TCPTransport::SendData(const char* singleBuffer, uint32_t length) {
	//��������
	string lengthStr = to_string(length);
	send(mDataSocket, lengthStr.c_str(), 8, 0);
	send(mDataSocket, singleBuffer, length, 0);
	return 0;
}

int TCPTransport::RecvData() {
	// TODO ׼������TCPճ��
	while (mRunFlag) {
		// ���ճ�����Ϣ
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
			dataLength = recv(mDataSocket, m_recvBufferPtr + recvLength, 8 * 1024 * 1024, 0); //��������
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