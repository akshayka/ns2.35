#ifndef ns_tcp_naive_q
#define ns_tcp_naive_q

#include <iomanip>
#include <map>
#include <string>
#include <utility>

#include "ip.h"
#include "q-learn.h"
#include "tcp.h"

class NaiveQTcpAgent : public virtual TcpAgent {
private:
    const double kEpsilon = 0.2;
    const double kDiscount = 0.5;
    const int kMinAction = 0;
    const int kMaxAction = 1;
    NaiveQLearn q_;
    NaiveState state_;
    int action_;
    std::map<double, std::pair<NaiveState, int>> ts_to_state_action_;

public:
    NaiveQTcpAgent() : TcpAgent(),
        q_(kEpsilon, kDiscount, kMinAction, kMaxAction) {};

    virtual void recv(Packet *pkt, Handler*);
    virtual void opencwnd();
    virtual void output_helper(Packet *pkt);
};

#endif
