#ifndef q_learn
#define q_learn

#include <cassert>
#include <chrono>
#include <cmath>
#include <map>
#include <random>
#include <utility>

template <class S>
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

    virtual int get_action(S state);
    virtual void incorporate_feedback(S state, int action,
      double reward, S state_prime);
    virtual std::map<std::string, double>
        featurize(S state, int action) const = 0;

private:
    // We use greedy-epsilon search, with 0 <= epsilon_ <= 1
    std::uniform_int_distribution<int> action_distribution_;
    std::uniform_real_distribution<double> epsilon_distribution_;
    std::default_random_engine generator_;
    double epsilon_;
    double discount_;
    int num_iters_;
    const int kMinAction;
    const int kMaxAction;
    std::map<std::string, double> weights_;

    double get_step_size() const { return 1.0 / std::sqrt(num_iters_); }
    double get_q(S state, int action) const;
    double get_v_opt(S state) const;
};

#endif
