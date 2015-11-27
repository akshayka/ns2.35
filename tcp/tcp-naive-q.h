#ifndef ns_tcp_naive_q
#define ns_tcp_naive_q

#include <iomanip>
#include <map>
#include <string>
#include <utility>

#include "debug.h"
#include "ip.h"
#include "q-learn.h"
#include "tcp.h"

class NaiveState : public learning::State {
public:
    NaiveState() : rtt_ratio_(-1), min_rtt_(-1), last_rtt_(-1) {};

    virtual std::map<std::string, double> featurize(int action) const {
        std::map<std::string, double> feature_map;
        std::ostringstream os;
        // It might be a good idea to discretize even more
        os << std::fixed << std::setprecision(1) << rtt_ratio_;
        DEBUG_STDERR("featurized rtt ratio: " << rtt_ratio_
                    << " --> " << os.str());
        feature_map[os.str()] = 1;
        feature_map[std::to_string(action)] = 1;
        return feature_map;
    }

    void set_rtt_ratio(double rtt_ratio) { rtt_ratio_ = rtt_ratio; }
    void set_min_rtt(double min_rtt) { min_rtt_ = min_rtt; }
    void set_last_rtt(double last_rtt) { last_rtt_ = last_rtt; }

    double rtt_ratio() { return rtt_ratio_; }
    double min_rtt() { return min_rtt_; }
    double last_rtt() { return last_rtt_; }

private:
    double rtt_ratio_;
    double min_rtt_;
    double last_rtt_;
};

class NaiveQTcpAgent : public virtual TcpAgent {
private:
    const double kEpsilon = 0.2;
    const double kDiscount = 0.5;
    const int kMinAction = 0;
    const int kMaxAction = 2;
    learning::QLearn q_;
    NaiveState state_;
    int action_;
    std::map<std::pair<double, int>, std::pair<NaiveState, int>>
        ts_to_state_action_;

public:
    NaiveQTcpAgent() : TcpAgent(),
        q_(kEpsilon, kDiscount, kMinAction, kMaxAction) {};

    virtual void recv_helper(Packet *pkt);
    virtual void opencwnd();
    virtual void output_helper(Packet *pkt);
};

#endif
