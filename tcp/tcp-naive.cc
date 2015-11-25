#include "tcp-naive.h"


MaxThroughputTcpAgent::MaxThroughputTcpAgent() {
    bind_bool("maxcwnd_", &maxcwnd_);
}

void MaxThroughputTcpAgent::recv_newack_helper(Packet *pkt) {
    TcpAgent::newack(pkt);

    /* Don't do anything meaningful, except setting cwnd to maxcwnd.
     * This policy should maximize throughput at the cost of maximizing delay.
     */
    cwnd_ = maxcwnd_;

	/* If the connection is done, call finish() */
	if ((highest_ack_ >= curseq_ - 1) && !closed_) {
		closed_ = 1;
		finish();
	}
    return;
}

static class MaxThroughputTcpClass : public TclClass {
public:
	MaxThroughputTcpClass() : TclClass("Agent/TCP/MaxThroughput") {}
	TclObject* create(int, const char*const*) {
		return (new MaxThroughputTcpAgent());
	}
} class_max_throughput_tcp;

void MinDelayTcpAgent::recv_newack_helper(Packet *pkt) {
    TcpAgent::newack(pkt);

    /* Do not grow the congestion control window. 
     * This policy should minimize delay at the cost of minimizing throughput.
     */
    cwnd_ = 1;

	/* If the connection is done, call finish() */
	if ((highest_ack_ >= curseq_ - 1) && !closed_) {
		closed_ = 1;
		finish();
	}
    return;
}

static class MinDelayTcpClass : public TclClass {
public:
	MinDelayTcpClass() : TclClass("Agent/TCP/MinDelay") {}
	TclObject* create(int, const char*const*) {
		return (new MinDelayTcpAgent());
	}
} class_min_delay_tcp;
