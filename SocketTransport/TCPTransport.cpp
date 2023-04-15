#include "TCPTransport.h"

#include <stdio.h>
#include <string>
#include <functional>

int TCPTransport::BindOrConnect(transportModel modelType, string localIp, int localPort, string remoteIp, int remotePort) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//����sockaddr_in�ṹ����������ڹ�����������IP��ַ �˿ں� ��ַ����
	struct sockaddr_in serverAddr;

	/*
	 ���㺯��
	 server_addr:Ҫ��������ݵ���ʼ��ַ
	 sizeof(server_addr):Ҫ����������ֽڸ���
	 */
	memset(&serverAddr, 0, sizeof(serverAddr));

	if (modelType == transportModel::SEND_MODEL) {
		// ���У��
		if (localIp == "" || localPort == 0) {
			printf("params error\n");
			return -1;
		}

		/*
		 socket������ʧ�ܷ���-1
		 int socket(int domain, int type, int protocol);
		 ��һ��������ʾʹ�õĵ�ַ���ͣ�AF_INET����ʹ��IPv4����ͨ��
		 �ڶ���������ʾ�׽������ͣ� SOCK_STREAM����TCPͨ��  SOCK_DGRAM����UDPͨ��
		 ��������������Ϊ0
		 */
		if ((mSendSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			return -1;
		}

		//��ʼ���������˵��׽��֣�����htons��htonl���˿ں͵�ַת�������ֽ���
		serverAddr.sin_family = AF_INET; //���õ�ʹ�õ�ַ����
		serverAddr.sin_port = htons(localPort); //���ö˿ں�
		//  //ip�����Ǳ���������ip��Ҳ�����ú�INADDR_ANY���棬����0.0.0.0���������е�ַ
		//  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_addr.s_addr = inet_addr(localIp.c_str());//����IP��ַ

		/*
		 bind��socket�뱾����һ���˿ڰ�,���Ϳ����ڸö˿ڼ�����������
		 ����bind��accept֮��ĺ����������׽��ֲ���������Ҫǿ��ת����(struct sockaddr *)
		 bind�����������������˵��׽��ֵ��ļ���������
		 */
		if (::bind(mSendSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("connect error");
			return -1;
		}
	} else if (modelType == transportModel::RECV_MODEL) {
		// ���У��
		if (remoteIp == "" || remotePort == 0) {
			printf("params error\n");
			return -1;
		}

		/*
		 socket������ʧ�ܷ���-1
		 int socket(int domain, int type, int protocol);
		 ��һ��������ʾʹ�õĵ�ַ���ͣ�AF_INET����ʹ��IPv4����ͨ��
		 �ڶ���������ʾ�׽������ͣ� SOCK_STREAM����TCPͨ��  SOCK_DGRAM����UDPͨ��
		 ��������������Ϊ0
		 */
		if ((mRecvSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//���� socket() ���������׽���
		{
			perror("socket");
			return 1;
		}

		serverAddr.sin_family = AF_INET;  //���õ�ʹ�õ�ַ����
		serverAddr.sin_port = htons(remotePort); //���ö˿ں�
		//ָ���������˵�ip�����ز��ԣ�10.8.116.226
		//inet_addr()�����������ʮ����IPת���������ֽ���IP
		serverAddr.sin_addr.s_addr = inet_addr(remoteIp.c_str());//����IP��ַ

		//��������
		int flag = 10;
		while (flag) {
			if (connect(mRecvSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
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

	mClientSocket = accept(mSendSocketFd, (struct sockaddr*)&clientAddr, (socklen_t*)&addrLen);

	if (mClientSocket < 0) {
		perror("accept");
		return -1;
	}

	return 0;
}

int TCPTransport::SendData(const char* singleBuffer) {
	//��������
	send(mClientSocket, singleBuffer, strlen(singleBuffer), 0);

	return 0;
}

int TCPTransport::RecvData() {
	char* recvBufferPtr = new char[1024]; //���ջ�����
	long singleLength;

	while (mRunFlag) {
		singleLength = recv(mRecvSocketFd, recvBufferPtr, 1024, 0); //��������
		if (singleLength <= 0) {
			Sleep(1);
			continue;
		}

		mRecvDataCallback(recvBufferPtr, singleLength);
	}

	return 0;
}

int TCPTransport::RunRecvDataThread() {
	recvDataThread = new thread(std::bind(&TCPTransport::RecvData, this));

	return 0;
}