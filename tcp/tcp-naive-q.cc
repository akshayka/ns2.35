#include "tcp-q-learning.h"
#include <cstdlib>

void NaiveQTcpAgent::output_helper(Packet *pkt) {
	hdr_tcp *tcph = hdr_tcp::access(pkt);
    q_.ts_to_state_action[tcph->ts()] = std::make_pair(state_, action_);
}

void NaiveQTcpAgent::recv(Packet *pkt, Handler*) {
    // Update state
	hdr_tcp *tcph = hdr_tcp::access(pkt);
	double now = Scheduler::instance().clock();
    double rtt = now - tcph->ts_echo(); // TODO(akshayka): multipy by 1000?
    if (state_.min_rtt == -1) {
        state_.min_rtt = rtt;
        state_.last_rtt = rtt;
    } else if (rtt < state_.min_rtt) {
        state_.min_rtt = rtt;
    }
    state_.rtt_ratio = rtt / state_.min_rtt;
    state_.last_rtt = rtt;

    // SARS update
    // positive rewards are good
    // This is a silly reward function because it doesn't take history
    // into account. Is it? Maybe just the state is silly.
    auto state_action = q_.ts_to_state_action[tcph->ts_echo()];
        state_action.first.last_rtt;
    q_.ts_to_state_action.erase(tcph->ts_echo());
    q_.incorporate_feedback(state_action.first, state_action.second,
                            reward, t_rtt_);

    // adjust cwnd and record action chosen!
    TcpAgent::recv(pkt);
}

void NaiveQTcpAgent::opencwnd() {
    // Only two actions: += 1 or linear increase
    action_ = q_.get_action(state_);
    if (action_ == 0) {
        ++cwnd_;
    } else {
        cwnd_ += 1.0 / cwnd_;
    }
}

static class NaiveQTcpClass : public TclClass {
public:
	NaiveQTcpClass() : TclClass("Agent/TCP/NaiveQ") {}
	TclObject* create(int, const char*const*) {
		return (new NaiveQTcpAgent());
	}
} class_naive_q_tcp;
