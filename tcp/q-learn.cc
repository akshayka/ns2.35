#include "q-learn.h"

#include <chrono>
#include <cstdlib>
#include <random>
#include <vector>

namespace learning {
int QLearn::get_action(const State& state) {
    ++num_iters_;
    if (epsilon_distribution_(generator_) < epsilon_) {
        return action_distribution_(generator_);
    } else {
        double curr_q = get_q(state, kMinAction);
        double q_opt = curr_q;
        std::vector<int> best_actions = { kMinAction };
        for (int a = kMinAction + 1; a <= kMaxAction; ++a) {
            curr_q = get_q(state, a);
            if (curr_q == q_opt) {
                best_actions.push_back(a);
            } else if (curr_q > q_opt) {
                q_opt = curr_q;
                best_actions.clear();
                best_actions.push_back(a);
            }
        }
        if (best_actions.size() > 1) {
            std::uniform_int_distribution<int> distribution(0,
                best_actions.size() - 1);
            int choice = distribution(generator_);
            return best_actions[choice];
        } else {
            return best_actions[0];
        }
    }
}

void QLearn::incorporate_feedback(const State& state, int action,
    double reward, const State& state_prime) {
    double eta = get_step_size();
    double feature_scale = eta * (
        get_q(state, action) - (
            reward + discount_ * get_v_opt(state_prime)));
    std::map<std::string, double> features = state.featurize(action);
    for (const auto& kv : features) {
        weights_[kv.first] -= feature_scale * kv.second;
    }

#ifdef DEBUG
    DEBUG_STDERR("features in weights_ vector");
    for (const auto& kv : weights_) {
        DEBUG_STDERR(kv.first);
    }
#endif
}

double QLearn::get_q(const State& state, int action) {
    std::map<std::string, double> features = state.featurize(action);
    double score = 0;
    for (const auto& kv : features) {
        // Implicitly creates an entry for the feature if it does not
        // yet exist
        score += weights_[kv.first] * kv.second;
    }
    return score;
}

double QLearn::get_v_opt(const State& state) {
    double curr_v = get_q(state, kMinAction);
    double v_opt = curr_v;
    for (int a = kMinAction + 1; a <= kMaxAction; ++a) {
        curr_v = get_q(state, a);
        if (curr_v > v_opt) {
            v_opt = curr_v;
        }
    }
    return v_opt;
}
}; // namespace learning
