#ifndef learning_q_learn
#define learning_q_learn

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace learning {

// A dummy state class.
class State {
public:
    virtual std::map<std::string, double> featurize(int action) const {
        std::map<std::string, double> feature_map;
        feature_map[std::to_string(action)] = 1;
        return feature_map;
    }
};

class QLearn {
public:
    QLearn(double epsilon, double discount, int min_action, int max_action)
    : epsilon_(epsilon),
      discount_(discount),
      kMinAction(min_action),
      kMaxAction(max_action),
      epsilon_distribution_(0, 1),
      action_distribution_(kMinAction, kMaxAction),
      generator_(std::chrono::system_clock::now().time_since_epoch().count()),
      num_iters_(0) {
        assert(kMinAction <= kMaxAction); 
        assert(epsilon >= 0 && epsilon <= 1);
        assert(discount >= 0 && discount <= 1);
    };

    virtual int get_action(State state);
    virtual void incorporate_feedback(State state, int action,
      double reward, State state_prime);

private:
    // We use greedy-epsilon search, with 0 <= epsilon_ <= 1
    double epsilon_;
    double discount_;
    const int kMinAction;
    const int kMaxAction;
    std::uniform_real_distribution<double> epsilon_distribution_;
    std::uniform_int_distribution<int> action_distribution_;
    std::default_random_engine generator_;
    int num_iters_;
    std::map<std::string, double> weights_;

    double get_step_size() const { return 1.0 / std::sqrt(num_iters_); }
    double get_q(State state, int action) const;
    double get_v_opt(State state) const;
};
}; // namespace learning

#endif
