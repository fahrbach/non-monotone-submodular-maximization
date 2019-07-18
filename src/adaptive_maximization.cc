#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <vector>

#include "adaptive_maximization.h"
#include "utilities.h"

using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::set;
using std::string;
using std::vector;

bool ReducedMean(const EvaluationOracle& oracle, const set<int>& S,
    vector<int> A, double tau, int t, double epsilon, double delta,
    MaximizationResult& result) {
  std::mt19937 rng; rng.seed(std::random_device()());
  int m = 16 * ceil(log(2 / delta) / pow(epsilon, 2));
  m = min(m, 100);  // Reduce sample complexity
  assert(m > 0);
  assert(t > 0); assert(A.size() >= t);
  int num_above_threshold = 0;
  set<int> T;
  for (auto u : S) T.insert(u);
  for (int i = 0; i < m; i++) {
    shuffle(A.begin(), A.end(), rng);
    for (int j = 0; j < t - 1; j++) {
      assert(!S.count(A[j]));  // T is expected to be disjoint from S.
      T.insert(A[j]);
    }
    int x = A[t - 1];
    double gain = oracle.MarginalValue(x, T);
    if (gain >= tau) num_above_threshold++;
    for (int j = 0; j < t - 1; j++) T.erase(A[j]);
  }
  double mu_hat = (double)num_above_threshold / m;
  // Assumes that result was increment for the new round.
  result.num_queries[result.num_rounds] += m;  
  if (mu_hat <= 1 - 1.5*epsilon) return true;
  return false;
}

pair<set<int>, set<int>> ThresholdSampling(
    const EvaluationOracle& oracle, const set<int>& old_S,
    int k, double tau, double epsilon, double delta, double c3,
    MaximizationResult& result, bool debug) {
  std::mt19937 rng; rng.seed(std::random_device()());
  double hat_epsilon = epsilon / 3;
  int n = oracle.num_nodes() - old_S.size();  // Oracle relative to S
  int r = ceil(log(2 * n / delta) / (-log(1 - hat_epsilon)));
  int m = ceil(log(k) * (1/hat_epsilon + 0.5));  // Tighter upper bound
  double hat_delta = delta / (2 * r * (m + 1));
  set<int> A;  // New ground set relative to S
  for (int u = 0; u < oracle.num_nodes(); u++) {
    if (!old_S.count(u)) A.insert(u);
  }
  set<int> S, S_for_queries;
  for (auto u : old_S) S_for_queries.insert(u);
  for (int round = 0; round < r; round++) {
    // Update maximization result
    result.num_rounds++;
    result.elements_added.push_back(set<int>());
    result.marginal_gains.push_back(0);
    result.function_values.push_back(result.function_values.back());
    result.num_queries.push_back(result.num_queries.back());
    if (n < c3 * k) break;
    // Filter remaining elements
    vector<int> filtered_A;
    result.num_queries[result.num_rounds] += A.size();
    for (auto u : A) {
      if (oracle.MarginalValue(u, S_for_queries) >= tau) {
        filtered_A.push_back(u);
      }
    }
    if (debug) {
      cout << "round: " << round << "\t";
      cout << "candidates: " << filtered_A.size() << endl;
    }
    A.clear();
    for (auto u : filtered_A) A.insert(u);
    if (A.size() == 0 || A.size() < c3 * k) break;
    std::map<int, bool> values_of_t;
    int t_star = -1, t = 0;
    for (int i = 0; i <= m; i++) {
      t = min((int)ceil(pow(1 + hat_epsilon, i)), (int)A.size());
      if (values_of_t.count(t)) continue;
      bool estimate = ReducedMean(oracle, S_for_queries, filtered_A, tau, t,
          hat_epsilon, hat_delta, result);
      values_of_t[t] = estimate;
      if (!estimate) {
        t_star = t;
      }
    }
    t = t_star;
    assert(t >= 1);
    t = min(t, k - (int)S.size());
    shuffle(filtered_A.begin(), filtered_A.end(), rng);
    if (debug) {
      cout << "subset size: " << t << "\t" << "|S|: " << S.size() + t << endl;
    }
    // Update the state of the algorithm and the result struct.
    set<int> T;
    for (int i = 0; i < t; i++) T.insert(filtered_A[i]);
    result.elements_added[result.num_rounds] = T;
    double gain = oracle.MarginalValue(T, S_for_queries);
    result.marginal_gains[result.num_rounds] = gain;
    result.function_values[result.num_rounds] += gain;
    if (debug) {
      cout << "gain: " << gain << endl;
    }
    for (auto u : T) {
      S.insert(u);
      S_for_queries.insert(u);
    }
    if (S.size() == k) break;
  }
  return make_pair(S, A);
}

set<int> UnconstrainedMaximization(const EvaluationOracle& oracle,
    const set<int>& old_S, vector<int> A, double epsilon, double delta,
    MaximizationResult& result) {
  std::mt19937 rng; rng.seed(std::random_device()());
  std::uniform_int_distribution<std::mt19937::result_type> dist(0, 1);
  int t = ceil(-log(delta) / log(1 + (4.0/3)*epsilon));
  t = min(t, 100);
  set<int> S;
  const double INF = 1e100;
  double max_gain = -INF;
  for (int i = 0; i < t; i++) {
    set<int> R;
    for (auto u : A) {
      if (dist(rng)) R.insert(u);
    }
    double gain = oracle.MarginalValue(R, old_S);
    if (gain > max_gain) {
      max_gain = gain;
      S = R;
    }
  }
  // Assumes results have been incremented for this round.
  result.num_queries[result.num_rounds] += t;  
  return S;
}

MaximizationResult AdaptiveNonmonotoneMaximization(
    const EvaluationOracle& oracle, int k, double epsilon, double delta,
    double c1, double c2, double c3, bool debug) {
  std::mt19937 rng; rng.seed(std::random_device()());
  double hat_epsilon = epsilon / 6;
  MaximizationResult final_result;
  int n = oracle.num_nodes();
  set<int> S;
  const double INF = 1e100;
  double delta_star = -INF;
  for (int i = 0; i < n; i++) {
    delta_star = max(delta_star, oracle.MarginalValue(i, S));
  }
  int r = ceil(log(k) * (1/hat_epsilon + 0.5));  // Tighter upper bound
  double hat_delta = delta / (2 * (r + 1));
  if (debug) {
    cout << delta_star << " " << hat_epsilon << " ";
    cout << r << " " << hat_delta << endl;
  }
  double max_function_value = 0;
  set<int> R;  // Stores final output set
  for (int i = 0; i <= r; i++) {
    double tau = c1 * pow(1 + hat_epsilon, i) * delta_star / k;
    MaximizationResult result;
    set<int> empty_set;
    int new_constraint = c2 * k;
    if (debug) {
      cout << i << "/" << r << ": " << tau << " " << new_constraint << endl;
    }
    auto SA = ThresholdSampling(oracle, empty_set, new_constraint, tau,
        hat_epsilon, hat_delta, c3, result, debug);
    set<int> S = SA.first;
    if (debug) {
      cout << "f(S): " << result.function_values.back() << endl;
    }
    // See if we can use the remaining elements.
    vector<int> A;
    for (auto x : SA.second) {
      A.push_back(x);
    }
    set<int> U, U_prime;
    if (A.size() < c3 * k) {
      // Update maximization result
      result.num_rounds++;
      result.elements_added.push_back(set<int>());
      result.marginal_gains.push_back(0);
      result.function_values.push_back(result.function_values.back());
      result.num_queries.push_back(result.num_queries.back());
      U = UnconstrainedMaximization(oracle, empty_set, A, hat_epsilon,
          hat_delta, result);
      vector<int> U_vec;
      for (auto u : U) U_vec.push_back(u);
      shuffle(U_vec.begin(), U_vec.end(), rng);
      double current_value = 0;
      set<int> U_prefix;
      set<int> best_prefix;
      double best_value = 0;
      for (int j = 0; j < min(k, (int)U_vec.size()); j++) {
        double gain = oracle.MarginalValue(U_vec[j], U_prefix);
        result.num_queries[result.num_rounds]++;
        U_prefix.insert(U_vec[j]);
        current_value += gain;
        if (current_value > best_value) {
          best_value = current_value;
          best_prefix = U_prefix;
        }
      }
      for (auto u : best_prefix) U_prime.insert(u);

      double S_value = oracle.Value(S);
      double U_value = oracle.Value(U_prime);
      if (U_value > S_value) {
        if (debug) {
          cout << "Take random: " << U_value << " > " << S_value << endl;
        }
        S = U_prime;
        result.elements_added[result.num_rounds] = S;
        result.marginal_gains[result.num_rounds] = U_value;
        result.function_values[result.num_rounds] = U_value;
      }
    }
    if (debug) {
      cout << i << "/" << r << ": " << tau;
      cout << " --> " << result.function_values.back() << endl;
    }
    // Update final answer
    if (result.function_values.back() > final_result.function_values.back()) {
      if (debug) cout << "found new best answer." << endl;
      final_result = result;
      R = S;
    }
    if (debug) cout << endl;
  }
  return final_result;
}

void TestAdaptiveNonmonotoneMaximization(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, double delta, string output_path) {
  const int TRIALS = 10;
  cout << "Running adaptive_nonmonotone_maximization...\n";
  const double c1 = 1.0/7.0;
  const double c2 = 1.0;
  const double c3 = 3.0;
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    const bool debug = true;
    auto result = AdaptiveNonmonotoneMaximization(
      oracle, size_constraint, epsilon, delta, c1, c2, c3, debug);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "epsilon_" + int_to_str(100*epsilon) + "-";
    output_filename += "adaptive_nonmonotone_maximization-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}

void TestAdaptiveMaximization(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, double delta, string output_path) {
  cout << "Running adaptive_maximization...\n";
  const double c1 = 1.0;
  const double c2 = 1.0;
  const double c3 = 0.0;
  const bool debug = true;
  auto result = AdaptiveNonmonotoneMaximization(
    oracle, size_constraint, epsilon, delta, c1, c2, c3, debug);
  string output_filename = output_path;
  output_filename += "constraint_" + int_to_str(size_constraint) + "-";
  output_filename += "epsilon_" + int_to_str(100*epsilon) + "-";
  output_filename += "adaptive_maximization.txt";
  result.Write(output_filename);
}
