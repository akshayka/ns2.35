#ifndef ns_tcp_naive_q
#define ns_tcp_naive_q

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "ip.h"
#include "q-learn.h"
#include "tcp.h"

class NaiveState {
public:
    double rtt_ratio = 1;
    double min_rtt = -1;
    double last_rtt = -1;
};

class NaiveQLearn : public QLearn<NaiveState> {
public:
    NaiveQLearn(double epsilon, double discount,
        int min_action, int max_action) :
        QLearn(epsilon, discount, min_action, max_action) {};

    std::map<std::string, double> featurize(NaiveState state,
        int action) const {
        std::map<std::string, double> feature_map;
        std::ostringstream os;
        // Might be a good idea to discretize even more
        os << std::fixed << std::setprecision(1) << state.rtt_ratio;
        feature_map[os.str()] = 1;
        feature_map[std::to_string(action)] = 1;
        return feature_map;
    }
};

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
