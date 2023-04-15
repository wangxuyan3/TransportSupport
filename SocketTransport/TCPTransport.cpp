#include "TCPTransport.h"

#include <stdio.h>
#include <string>
#include <functional>

int TCPTransport::BindOrConnect(transportModel modelType, string localIp, int localPort, string remoteIp, int remotePort) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//声明sockaddr_in结构体变量，用于构建服务器的IP地址 端口号 地址类型
	struct sockaddr_in serverAddr;

	/*
	 置零函数
	 server_addr:要置零的数据的起始地址
	 sizeof(server_addr):要置零的数据字节个数
	 */
	memset(&serverAddr, 0, sizeof(serverAddr));

	if (modelType == transportModel::SEND_MODEL) {
		// 入参校验
		if (localIp == "" || localPort == 0) {
			printf("params error\n");
			return -1;
		}

		/*
		 socket函数，失败返回-1
		 int socket(int domain, int type, int protocol);
		 第一个参数表示使用的地址类型，AF_INET就是使用IPv4进行通信
		 第二个参数表示套接字类型： SOCK_STREAM则是TCP通信  SOCK_DGRAM则是UDP通信
		 第三个参数设置为0
		 */
		if ((mSendSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			return -1;
		}

		//初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
		serverAddr.sin_family = AF_INET; //设置的使用地址类型
		serverAddr.sin_port = htons(localPort); //设置端口号
		//  //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
		//  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_addr.s_addr = inet_addr(localIp.c_str());//设置IP地址

		/*
		 bind将socket与本机的一个端口绑定,随后就可以在该端口监听服务请求
		 对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
		 bind三个参数：服务器端的套接字的文件描述符，
		 */
		if (::bind(mSendSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("connect error");
			return -1;
		}
	} else if (modelType == transportModel::RECV_MODEL) {
		// 入参校验
		if (remoteIp == "" || remotePort == 0) {
			printf("params error\n");
			return -1;
		}

		/*
		 socket函数，失败返回-1
		 int socket(int domain, int type, int protocol);
		 第一个参数表示使用的地址类型，AF_INET就是使用IPv4进行通信
		 第二个参数表示套接字类型： SOCK_STREAM则是TCP通信  SOCK_DGRAM则是UDP通信
		 第三个参数设置为0
		 */
		if ((mRecvSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//调用 socket() 函数创建套接字
		{
			perror("socket");
			return 1;
		}

		serverAddr.sin_family = AF_INET;  //设置的使用地址类型
		serverAddr.sin_port = htons(remotePort); //设置端口号
		//指定服务器端的ip，本地测试：10.8.116.226
		//inet_addr()函数，将点分十进制IP转换成网络字节序IP
		serverAddr.sin_addr.s_addr = inet_addr(remoteIp.c_str());//设置IP地址

		//建立连接
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

	mClientSocket = accept(mSendSocketFd, (struct sockaddr*)&clientAddr, (socklen_t*)&addrLen);

	if (mClientSocket < 0) {
		perror("accept");
		return -1;
	}

	return 0;
}

int TCPTransport::SendData(const char* singleBuffer) {
	//发送数据
	send(mClientSocket, singleBuffer, strlen(singleBuffer), 0);

	return 0;
}

int TCPTransport::RecvData() {
	char* recvBufferPtr = new char[1024]; //接收缓冲区
	long singleLength;

	while (mRunFlag) {
		singleLength = recv(mRecvSocketFd, recvBufferPtr, 1024, 0); //接收数据
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