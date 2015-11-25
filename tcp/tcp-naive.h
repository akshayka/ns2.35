#ifndef ns_tcp_naive
#define ns_tcp_naive

#include "tcp.h"
#include "ip.h"

class MaxThroughputTcpAgent : public virtual TcpAgent {
private:
public:
    MaxThroughputTcpAgent();
	virtual void recv_newack_helper(Packet* pkt);
};

class MinDelayTcpAgent : public virtual TcpAgent {
private:
public:
	virtual void recv_newack_helper(Packet* pkt);
};

#endif
