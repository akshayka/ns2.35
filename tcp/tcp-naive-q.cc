#include "tcp-naive-q.h"
#include <cstdlib>

void NaiveQTcpAgent::output_helper(Packet *pkt) {
	hdr_tcp *tcph = hdr_tcp::access(pkt);
    DEBUG_STDERR("Mapping ts: " << tcph->ts() << " seqno: " << tcph->seqno());
    ts_to_state_action_[std::make_pair(tcph->ts(), tcph->seqno())] =
        std::make_pair(state_, action_);
}

void NaiveQTcpAgent::recv_helper(Packet *pkt) {
    static int zero_reward = 0;
    static int nonzero_reward = 0;
	hdr_tcp *tcph = hdr_tcp::access(pkt);
    if (q_.num_iters() == 0) {
        DEBUG_STDERR("recv called with zero iterations; seqno: "
                     << tcph->seqno());
        return;
    }

    // Update state
	double now = Scheduler::instance().clock();
    double rtt = now - tcph->ts_echo();
    if (state_.min_rtt() == -1) {
        state_.set_min_rtt(rtt);
    } else if (rtt < state_.min_rtt()) {
        state_.set_min_rtt(rtt);
    }
    DEBUG_STDERR("setting rtt ratio; rtt: " << rtt
                 << " min rtt: " << state_.min_rtt());
    state_.set_rtt_ratio(rtt / state_.min_rtt());
    state_.set_last_rtt(rtt);
    DEBUG_STDERR("rtt ratio: " << state_.rtt_ratio());

    // SARS update
    // positive rewards are good
    // This is a silly reward function because it doesn't take history
    // into account. Is it? Maybe just the state is silly.
    // TODO What about the first packet's ACK we receive?
    auto key = std::make_pair(tcph->ts_echo(), tcph->seqno());
    if (ts_to_state_action_.find(key) == ts_to_state_action_.end()) {
        DEBUG_STDERR("could not find packet in map. ts echo: "
                    << tcph->ts_echo() << " seqno: " << tcph->seqno());
    }
    auto state_action = ts_to_state_action_[key];
    NaiveState state = state_action.first;
    int action = state_action.second;
    double reward;
    if (state.last_rtt() == -1) {
        reward = 0;
        ++zero_reward;
        DEBUG_STDERR("previous state was not set; giving zero reward. "
                     << "Seqno: " << tcph->seqno()
                     << " Count: " << zero_reward);
    } else {
        ++nonzero_reward;
        DEBUG_STDERR("nonzero reward count: " << nonzero_reward << " ts echo: "
                     << tcph->ts_echo() << " seqno: " << tcph->seqno());
        reward = (state.last_rtt() - state_.last_rtt()) /
            state.last_rtt();
    }
    ts_to_state_action_.erase(key);
    q_.incorporate_feedback(state, action, reward, state_);
    DEBUG_STDERR("num features: " << q_.num_features());
}

void NaiveQTcpAgent::opencwnd() {
    int action = q_.get_action(state_);
    switch (action) {
    case 0:
        ++cwnd_;
        break;
    case 1:
        assert(cwnd_ != 0);
        cwnd_ += 1.0 / cwnd_;
        break;
    case 2:
        assert(cwnd_ != 0);
        {
            double decrement = 1.0 / cwnd_;
            double update = cwnd_ - decrement;
            if (update > 0) {
                cwnd_ = update;
            } else {
                std::cerr << "cannot decrease cwnd_: " << cwnd_ << std::endl;
            }
        }
        break;
    default:
        std::cerr << "invalid action: " << action;
        std::abort();
    }
}

static class NaiveQTcpClass : public TclClass {
public:
	NaiveQTcpClass() : TclClass("Agent/TCP/NaiveQ") {}
	TclObject* create(int, const char*const*) {
		return (new NaiveQTcpAgent());
	}
} class_naive_q_tcp;
