#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>

#include "blits.h"
#include "utilities.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::set;
using std::string;
using std::vector;

double DeltaEstimate(int a, const set<int>& S, const set<int>& X,
    const EvaluationOracle& oracle, int k, int r, MaximizationResult& result) {
  std::mt19937 rng; rng.seed(std::random_device()());
  const int number_of_samples = 100;
  vector<int> v;
  for (auto x : X) v.push_back(x);
  int size_of_R = min(k/r, (int)X.size());

  set<int> mutable_S;
  for (auto u : S) mutable_S.insert(u);
  double running_sum = 0;
  for (int i = 0; i < number_of_samples; i++) {
    if (S.count(a)) continue;  // 0 marginal gain here
    shuffle(v.begin(), v.end(), rng);
    for (int j = 0; j < size_of_R; j++) mutable_S.insert(v[j]);
    if (mutable_S.count(a)) mutable_S.erase(a);
    double gain = oracle.MarginalValue(a, mutable_S);
    running_sum += gain;
    for (int j = 0; j < size_of_R; j++) {
      if (S.count(v[j])) continue;
      if (mutable_S.count(v[j])) mutable_S.erase(v[j]);
    }
  }
  double estimate = running_sum / number_of_samples;
  result.num_queries[result.num_rounds] += number_of_samples;
  return estimate;
}

double FunctionEstimate(const set<int>& S, const set<int>& X,
    const set<int>& X_pos, const EvaluationOracle& oracle, int k, int r,
    MaximizationResult& result) {
  std::mt19937 rng; rng.seed(std::random_device()());
  const int number_of_samples = 100;
  double running_sum = 0;
  int size_of_R = k/r;
  vector<int> v;
  for (auto x : X) v.push_back(x);
  for (int i = 0; i < number_of_samples; i++) {
    shuffle(v.begin(), v.end(), rng);
    set<int> T;
    for (int j = 0; j < size_of_R; j++) {
      int x = v[j];
      if (X_pos.count(x)) T.insert(x);
    }
    double gain = oracle.MarginalValue(T, S);
    running_sum += gain;
  }
  double estimate = running_sum / number_of_samples;
  result.num_queries[result.num_rounds] += number_of_samples;
  return estimate;
}

set<int> Sieve(const set<int>& S, int k, int i, int r, double epsilon,
    double opt, const EvaluationOracle& oracle, MaximizationResult& result) {
  std::mt19937 rng; rng.seed(std::random_device()());
  int n = oracle.num_nodes();
  set<int> X;
  for (int j = 0; j < n; j++) {
    if (!S.count(j)) X.insert(j);  // Only consider unchosen nodes!
  }
  double last_function_value = result.function_values.back();
  double t = (1 - epsilon/2)/2 *
      (pow(1-1.0/(double)r, i-1) * (1-epsilon/2)*opt - last_function_value);
  int sieve_loop_counter = 0;
  while (X.size() > k) {
    // Update maximization result
    result.num_rounds++;
    result.elements_added.push_back(set<int>());
    result.marginal_gains.push_back(0);
    result.function_values.push_back(result.function_values.back());
    result.num_queries.push_back(result.num_queries.back());

    sieve_loop_counter++;
    // Need to write Delta(a, S, X) function
    set<int> X_pos;
    for (auto a : X) {
      if (DeltaEstimate(a, S, X, oracle, k, r, result) >= 0) {
        X_pos.insert(a);
      }
    }
    double function_estimate =
        FunctionEstimate(S, X, X_pos, oracle, k, r, result);
    if (function_estimate >= t/r) {
      // Return random sample
      vector<int> v;
      for (auto x : X) v.push_back(x);
      shuffle(v.begin(), v.end(), rng);
      int size_of_R = k/r;
      set<int> T;
      for (int j = 0; j < size_of_R; j++) {
        if (X_pos.count(v[j]) && !S.count(v[j])) T.insert(v[j]);
      }
      // Update result
      double gain = oracle.MarginalValue(T, S);
      result.elements_added[result.num_rounds] = T;
      result.marginal_gains[result.num_rounds] = gain;
      result.function_values[result.num_rounds] += gain;
      return T;
    }
    set<int> new_X;
    for (auto a : X) {
      if (DeltaEstimate(a, S, X, oracle, k, r, result) >= (1 + epsilon/4)*t/k) {
        new_X.insert(a);
      }
    }
    if (new_X == X) break;   // Needed condition to avoid their bug.
    X = new_X;
  }
  // Outside of while loop
  // Update maximization result
  result.num_rounds++;
  result.elements_added.push_back(set<int>());
  result.marginal_gains.push_back(0);
  result.function_values.push_back(result.function_values.back());
  result.num_queries.push_back(result.num_queries.back());

  set<int> new_X_pos;
  for (auto a : X) {
    if (DeltaEstimate(a, S, X, oracle, k, r, result) >= 0) {
      new_X_pos.insert(a);
    }
  }
  set<int> X_pos = new_X_pos;
  int fake_node_counter = -1;
  int X_size = X.size();  // X may change size
  for (int j = 0; j < k - X_size; j++) {
    X.insert(fake_node_counter);
    fake_node_counter--;
  }
  vector<int> v;
  for (auto x : X) v.push_back(x);
  shuffle(v.begin(), v.end(), rng);
  int size_of_R = k/r;
  set<int> T;
  for (int j = 0; j < size_of_R; j++) {
    if (X_pos.count(v[j]) && !S.count(v[j])) T.insert(v[j]);
  }
  // Update result
  double gain = oracle.MarginalValue(T, S);
  result.elements_added[result.num_rounds] = T;
  result.marginal_gains[result.num_rounds] = gain;
  result.function_values[result.num_rounds] += gain;
  return T;
}

MaximizationResult Blits(const EvaluationOracle& oracle,
    int k, int r, double epsilon, bool debug) {
  epsilon *= 0.5;  // Adjust epsilon since we're searching for OPT.

  MaximizationResult final_result;
  int n = oracle.num_nodes();
//  int r = 20 * log(n) / log(1 + epsilon/2) / epsilon;
//  r = min(r, 30); // Need to set manually because the number of rounds is > 1k
  cout << "number of rounds: " << r << "\n";
  assert(k/r > 0);   // Their setting of r can fail sometimes?

  set<int> S;
  const double INF = 1e100;
  double ans_so_far = -INF;
  double delta_star = -INF;
  for (int i = 0; i < n; i++) {
    delta_star = max(delta_star, oracle.MarginalValue(i, S));
  }
  int number_of_opt_guesses = ceil(log(k) / log(1 + epsilon));
  for (int j = 0; j <= number_of_opt_guesses; j++) {
    double opt_guess = delta_star * pow(1 + epsilon, j);
    cout << j << "/" << number_of_opt_guesses << ": opt=" << opt_guess << endl;
    set<int> S;
    MaximizationResult result;
    for (int i = 1; i <= r; i++) {
      set<int> T = Sieve(S, k, i, r, epsilon, opt_guess, oracle, result);
      for (auto u : T) S.insert(u);
      cout << " - inner round: " << i << "/" << r 
           << ": |S| = " << S.size() << ", ans = "
           << result.function_values.back() << endl;
    }
    if (result.function_values.back() > ans_so_far) {
      ans_so_far = result.function_values.back();
      final_result = result;
      cout << "new maximizer: " << ans_so_far << endl;
    }
  }
  return final_result;
}

void TestBlits(const EvaluationOracle& oracle,
    int size_constraint, int rounds, double epsilon, string output_path) {
  const int TRIALS = 5;
  cout << "Running blits...\n";
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    bool debug = true;
    auto result = Blits(oracle, size_constraint, rounds, epsilon, debug);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "epsilon_" + int_to_str(100*epsilon) + "-";
    output_filename += "rounds_" + int_to_str(rounds) + "-";
    output_filename += "blits-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}
