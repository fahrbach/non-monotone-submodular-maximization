#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <vector>

#include "random_greedy.h"
#include "utilities.h"

using std::cout;
using std::endl;
using std::greater;
using std::make_pair;
using std::max;
using std::min;
using std::mt19937;
using std::pair;
using std::random_device;
using std::set;
using std::sort;
using std::string;
using std::uniform_int_distribution;
using std::vector;

MaximizationResult Random(const EvaluationOracle& oracle,
                          int size_constraint, bool prefix, bool debug) {
  const int num_samples = 25;
  MaximizationResult result;
  int ground_set_size = oracle.num_nodes();
  mt19937 rng; rng.seed(random_device()());
  vector<int> elements(ground_set_size);
  for (int i = 0; i < ground_set_size; i++) elements[i] = i;
  set<int> S;
  double max_function_value = 0;
  int num_queries = 0;

  vector<pair<double, set<int>>> samples(num_samples);
  for (int r = 0; r < num_samples; r++) {
    double cur_function_value = 0;
    set<int> cur_S;
    shuffle(elements.begin(), elements.end(), rng);
    double best_value_for_round = 0;
    set<int> best_S_for_round;
    if (prefix) {  // Consider prefixes
      for (int i = 0; i < ground_set_size; i++) {
        int x = elements[i];
        double gain = oracle.MarginalValue(x, cur_S);
        num_queries++;
        cur_S.insert(x);
        cur_function_value += gain;
        if (cur_function_value > best_value_for_round) {
          best_value_for_round = cur_function_value;
          best_S_for_round = cur_S;
        }
        if (cur_S.size() >= size_constraint) break;
      }
    } else { // Just take random set of size min(n, k)
      for (int i = 0; i < min(ground_set_size, size_constraint); i++) {
        best_S_for_round.insert(elements[i]);
      }
      best_value_for_round = oracle.Value(best_S_for_round);
    }
    samples[r].first = best_value_for_round;
    samples[r].second = best_S_for_round;
  }
  sort(samples.begin(), samples.end());

  max_function_value = samples[num_samples/2].first;
  S = samples[num_samples/2].second;
  // Update maximization results.
  result.num_rounds = 1;
  result.elements_added.push_back(S);
  result.marginal_gains.push_back(max_function_value);
  result.function_values.push_back(max_function_value);
  result.num_queries.push_back(num_queries / num_samples);
  if (debug) {
    cout << result.num_rounds << ":\t";
    cout << result.elements_added.back().size() << "\t";
    cout << result.function_values.back() << "\t";
    cout << result.marginal_gains.back() << "\t";
    cout << result.num_queries.back() << endl;
  }
  return result;
}

MaximizationResult Greedy(const EvaluationOracle& oracle,
                          int size_constraint, bool debug) {
  const double k_INF = 1e100;
  MaximizationResult result;
  mt19937 rng; rng.seed(random_device()());
  int ground_set_size = oracle.num_nodes();
  set<int> S;
  int num_rounds = 0;
  int num_queries = 0;
  while ((int)S.size() < size_constraint) {
    num_rounds += 1;
    // Find maximum marginal gain among all elements not in S.
    vector<int> candidates;
    double max_gain = -k_INF;  // INF
    for (int u = 0; u < ground_set_size; u++) {
      if (S.count(u)) continue;
      num_queries += 1;
      double gain = oracle.MarginalValue(u, S);
      if (gain > max_gain) {
        max_gain = gain;
        candidates.clear();
      }
      if (gain == max_gain) candidates.push_back(u);
    }
    // Choose random element with maximum marginal gain.
    assert(candidates.size() >= 1);
    uniform_int_distribution<mt19937::result_type> dist(0, candidates.size()-1);
    int u = candidates[dist(rng)];
    S.insert(u);
    // Update maximization results.
    result.num_rounds = num_rounds;
    set<int> T; T.insert(u);
    result.elements_added.push_back(T);
    result.marginal_gains.push_back(max_gain);
    result.function_values.push_back(result.function_values.back() + max_gain);
    result.num_queries.push_back(num_queries);
    if (debug) {
      cout << result.num_rounds << ":\t";
      cout << result.elements_added.back().size() << "\t";
      cout << result.function_values.back() << "\t";
      cout << result.marginal_gains.back() << "\t";
      cout << result.num_queries.back() << endl;
    }
  }
  return result;
}

MaximizationResult RandomGreedy(const EvaluationOracle& oracle,
                                int size_constraint, bool debug) {
  int ground_set_size = oracle.num_nodes();
  MaximizationResult result;
  int new_ground_set_size = ground_set_size + 2*size_constraint;  // Add fakes
  mt19937 rng; rng.seed(random_device()());
  set<int> S, true_S;
  int num_rounds = 0;
  int num_queries = 0;
  while ((int)S.size() < size_constraint) {
    num_rounds += 1;
    vector<pair<double, int>> gains_and_elements;
    for (int u = 0; u < new_ground_set_size; u++) {
      if (S.count(u)) continue;
      double gain = 0;  // Default for fake elements
      if (u < ground_set_size) {
        gain = oracle.MarginalValue(u, true_S);
        num_queries += 1;
      }
      gains_and_elements.push_back(make_pair(gain, u));
    }
    sort(gains_and_elements.begin(), gains_and_elements.end(),
         greater<pair<double, int>>());
    assert((int)gains_and_elements.size() >= size_constraint);
    // Choose random element from the top k with largest marginal gain.
    uniform_int_distribution<mt19937::result_type> dist(0, size_constraint - 1);
    int idx = dist(rng);
    double gain = gains_and_elements[idx].first;
    int u = gains_and_elements[idx].second;
    S.insert(u);
    if (u < ground_set_size) true_S.insert(u);
    // Update maximization results.
    result.num_rounds = num_rounds;
    set<int> T;
    if (u < ground_set_size) T.insert(u);  // Only record original elements.
    result.elements_added.push_back(T);
    result.marginal_gains.push_back(gain);
    result.function_values.push_back(result.function_values.back() + gain);
    result.num_queries.push_back(num_queries);
    if (debug) {
      cout << result.num_rounds << ":\t";
      cout << result.elements_added.back().size() << "\t";
      cout << result.function_values.back() << "\t";
      cout << result.marginal_gains.back() << "\t";
      cout << result.num_queries.back() << endl;
    }
  }
  return result;
}

MaximizationResult RandomLazyGreedyImproved(const EvaluationOracle& oracle,
                                            int size_constraint,
                                            double delta, bool debug) {
  // Initialization
  int ground_set_size = oracle.num_nodes();
  MaximizationResult result;
  int new_ground_set_size = ground_set_size + 2*size_constraint;  // Add fakes
  mt19937 rng; rng.seed(random_device()());
  int num_rounds = 0;
  int num_queries = 0;
  set<int> S, true_S, M;  // Init empty
  double W = 0, w = 0;
  for (int u = 0; u < ground_set_size; u++) {
    W = max(W, oracle.MarginalValue(u, S));
  }
  num_queries += ground_set_size;  // To compute W
  FillM(oracle, S, true_S, M, size_constraint, delta, w, W, result, debug);
  num_queries += ground_set_size;  // To fill M
  for (int i = 0; i < size_constraint; i++) {
    num_rounds += 1;
    vector<int> elements_in_M;
    for (auto u : M) elements_in_M.push_back(u);
    assert((int)elements_in_M.size() >= size_constraint);
    uniform_int_distribution<mt19937::result_type> dist(0, size_constraint - 1);
    int idx = dist(rng);
    int u_hat = elements_in_M[idx];
    int u_chosen = -1;
    num_queries++;
    if (u_hat >= ground_set_size ||
        oracle.MarginalValue(u_hat, true_S) > (1 - delta)*w) {
      u_chosen = u_hat;
    } else {
      set<int> new_M;
      for (auto u : M) {
        if (u < ground_set_size &&
            oracle.MarginalValue(u, true_S) <= w*(1 - delta)) {
          // Remove u from M (implicitly)
          num_queries++;
        } else {
          new_M.insert(u);  // keep this element
        }
      }
      M = new_M;
      FillM(oracle, S, true_S, M, size_constraint, delta, w, W, result, debug);
      num_queries += ground_set_size;  // To fill M
      set<int> M_hat;  // Elements added to M
      for (auto u : M) {
        if (!new_M.count(u)) M_hat.insert(u);
      }
      vector<int> elements_in_M_hat;
      for (auto u : M_hat) elements_in_M_hat.push_back(u);
      assert(elements_in_M_hat.size() > 0);
      uniform_int_distribution<mt19937::result_type>
          dist2(0, (int)elements_in_M_hat.size() - 1);
      idx = dist2(rng);
      assert(idx < (int)elements_in_M_hat.size());
      u_chosen = elements_in_M_hat[idx];
      assert(!S.count(u_chosen));
    }
    assert(u_chosen != -1);
    double gain = 0;  // For recording results
    S.insert(u_chosen);
    if (u_chosen < ground_set_size) {
      gain = oracle.MarginalValue(u_chosen, true_S);
      true_S.insert(u_chosen);
    }
    // Update maximization results
    result.num_rounds = num_rounds;
    set<int> T;
    if (u_chosen < ground_set_size) T.insert(u_chosen);  // Record originals
    result.elements_added.push_back(T);
    result.marginal_gains.push_back(gain);
    result.function_values.push_back(result.function_values.back() + gain);
    result.num_queries.push_back(num_queries);
    if (debug) {
      cout << result.num_rounds << ":\t";
      cout << result.elements_added.back().size() << "\t";
      cout << result.function_values.back() << "\t";
      cout << result.marginal_gains.back() << "\t";
      cout << result.num_queries.back() << endl;
    }
  }
  return result;
}

void FillM(const EvaluationOracle& oracle, const set<int>& S,
           const set<int>& true_S, set<int>& M, int size_constraint,
           double delta, double& w, const double W, MaximizationResult& result,
           bool debug) {
  int ground_set_size = oracle.num_nodes();
  vector<double> current_marginal(ground_set_size);
  for (int u = 0; u < ground_set_size; u++) {
    if (S.count(u)) continue;
    current_marginal[u] = oracle.MarginalValue(u, true_S);
  }
  for (w = W; w > delta*W/size_constraint; w *= (1 - delta)) {
    for (int u = 0; u < ground_set_size; u++) {
      // If u \in S: continue
      double gain = current_marginal[u];  // precomputed
      if (gain > w*(1 - delta)) {
        M.insert(u);
        if (M.size() == size_constraint) return;
      }
    }
  }
  int new_ground_set_size = ground_set_size + 2*size_constraint;
  for (int u = ground_set_size; u < new_ground_set_size; u++) {
    if (S.count(u)) continue;
    M.insert(u);
    if (M.size() == size_constraint) return;
  }
}

void TestRandom(const EvaluationOracle& oracle,
                int size_constraint, string output_path) {
  const int TRIALS = 10;
  cout << "Running random...\n";
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    bool prefix = false;
    auto result = Random(oracle, size_constraint, prefix);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "random-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}

void TestRandomPrefix(const EvaluationOracle& oracle,
                      int size_constraint, string output_path) {
  const int TRIALS = 10;
  cout << "Running random_prefix...\n";
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    bool prefix = true;
    auto result = Random(oracle, size_constraint, prefix);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "random_prefix-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}

void TestGreedy(const EvaluationOracle& oracle,
                int size_constraint, string output_path) {
  cout << "Running greedy...\n";
  auto result = Greedy(oracle, size_constraint);
  string output_filename = output_path;
  output_filename += "constraint_" + int_to_str(size_constraint) + "-";
  output_filename += "greedy.txt";
  result.Write(output_filename);
}

void TestRandomGreedy(const EvaluationOracle& oracle,
                      int size_constraint, string output_path) {
  const int TRIALS = 10;
  cout << "Running random_greedy...\n";
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    auto result = RandomGreedy(oracle, size_constraint);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "random_greedy-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}

void TestRandomLazyGreedyImproved(const EvaluationOracle& oracle,
                      int size_constraint, string output_path) {
  const int TRIALS = 10;
  cout << "Running random_lazy_greedy_improved...\n";
  for (int trial = 1; trial <= TRIALS; trial++) {
    cout << " - trial: " << trial << "/" << TRIALS << endl;
    const double epsilon = 0.01;  // Comparable to Random-Greedy at this value
    auto result = RandomLazyGreedyImproved(oracle, size_constraint, epsilon);
    string output_filename = output_path;
    output_filename += "constraint_" + int_to_str(size_constraint) + "-";
    output_filename += "random_lazy_greedy_improved-";
    output_filename += "trial_" + int_to_str(trial) + "_" + int_to_str(TRIALS) + ".txt";
    result.Write(output_filename);
  }
}
