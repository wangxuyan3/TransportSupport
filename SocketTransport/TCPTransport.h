#pragma once

#include "TransportSupport.h"

class TCPTransport : public TransportSupport {
public:
	virtual int SendData(const char* singleBuffer) final;
	virtual int RecvData() final;
	virtual int RunRecvDataThread() final;
private:
	virtual int BindOrConnect(transportModel modelType, string localIp, int localPort, string remoteIp, int remotePort) final;
	//virtual int SetAttrParams() final; // 模板类中根据参数设置是否阻塞模式
	virtual int StartListen() final;
};