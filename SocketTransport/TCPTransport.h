#pragma once

#include "TransportSupport.h"

class TCPTransport : public TransportSupport {
public:
	TCPTransport() {};
	~TCPTransport();
	virtual void LoadConfigFromFile() final;
	virtual void LoadConfig(string serverIP, uint32_t serverPort, string clientIP, uint32_t clientPort) final;
	virtual int SendData(const char* singleBuffer, uint32_t length) final;
	virtual int RecvData() final;
	virtual int RunRecvDataThread() final;
private:
	virtual int BindOrConnect(transportModel modelType) final;
	//virtual int SetAttrParams() final; // 模板类中根据参数设置是否阻塞模式
	virtual int StartListen() final;
	char* m_recvBufferPtr = new char[8 * 1024 * 1024]; //接收缓冲区
	char* m_lengthInfoBufferPtr = new char[8]; //接收缓冲区

};