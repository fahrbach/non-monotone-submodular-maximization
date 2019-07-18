#include <cassert>
#include <cmath>
#include <iostream>

#include "adaptive_maximization.h"
#include "fantom.h"
#include "utilities.h"

using std::cout;
using std::endl;
using std::set;
using std::string;
using std::vector;

set<int> GDT(const EvaluationOracle&  oracle, const set<int>& omega, double rho,
    int size_constraint, MaximizationResult& result, bool debug) {
  int num_queries = result.num_queries.back();
  // Maximum marginal
  set<int> empty_set;
  double maximum_marginal = -1;
  int best_element = -1;
  for (auto x : omega) {
    double gain = oracle.MarginalValue(x, empty_set);
    if (gain > maximum_marginal) {
      maximum_marginal = gain;
      best_element = x;
    }
  }
  num_queries += omega.size();
  assert(best_element != -1);
  set<int> best_element_set; best_element_set.insert(best_element);

  // Density greedy step
  set<int> S;
  double function_value = 0;
  for (int i = 0; i < size_constraint; i++) {
    double best_marginal = -1;
    int best_element = -1;
    for (auto x : omega) {
      if (S.count(x)) continue;
      double gain = oracle.MarginalValue(x, S);
      num_queries++;
      if (gain > best_marginal) {
        best_marginal = gain;
        best_element = x;
      }
    }
    if (best_marginal < rho) break;  
    S.insert(best_element);
    function_value += best_marginal;

    // Update maximization results.
    result.num_rounds++;
    set<int> T; T.insert(best_element);
    result.elements_added.push_back(T);
    result.marginal_gains.push_back(best_marginal);
    // For FANTOM only track improvements
    result.function_values.push_back(result.function_values.back()); 
    if (function_value > result.function_values.back()) {
      result.function_values[result.num_rounds] = function_value;
    }
    result.num_queries.push_back(num_queries);
  }

  set<int> ans = best_element_set;
  if (maximum_marginal < function_value) {
    ans = S;
    if (function_value > result.function_values[result.num_rounds]) {
      result.function_values[result.num_rounds] = function_value;
    }
  } else {
    result.function_values[result.num_rounds] = maximum_marginal;
    if (maximum_marginal > result.function_values[result.num_rounds]) {
      result.function_values[result.num_rounds] = maximum_marginal;
    }
  }
  return ans;
}

set<int> IGDT(const EvaluationOracle&  oracle, double rho, int size_constraint,
    MaximizationResult& result, bool debug) {
  int n = oracle.num_nodes();
  set<int> omega;
  for (int i = 0; i < n; i++) omega.insert(i);

  set<int> ans;
  double max_function_value = -1;
  for (int i = 1; i <= 2; i++) {  // p = 1
    set<int> S = GDT(oracle, omega, rho, size_constraint, result, debug);
    double S_value = oracle.Value(S);
    if (S_value > max_function_value) {
      ans = S;
      max_function_value = S_value;
    }
    set<int> empty_set; // For oracle marginals
    vector<int> S_vector;
    for (auto x : S) S_vector.push_back(x);
    const double epsilon = 0.25;
    const double delta = 0.01;

    // Update maximization results.
    result.num_rounds++;
    set<int> T;
    result.elements_added.push_back(T);
    result.marginal_gains.push_back(0);
    result.function_values.push_back(result.function_values.back());
    result.num_queries.push_back(result.num_queries.back());
    set<int> S_prime = UnconstrainedMaximization(oracle, empty_set, S_vector,
        epsilon, delta, result);
    double unconstrained_value = oracle.Value(S_prime);
    if (unconstrained_value > max_function_value) {
      ans = S_prime;
      max_function_value = unconstrained_value;
      result.function_values[result.num_rounds] = unconstrained_value;
    }
    for (auto x : S) {
      omega.erase(x);
    }
  }
  assert(max_function_value != -1);
  return ans;
}

MaximizationResult Fantom(const EvaluationOracle& oracle,
                          int size_constraint, double epsilon, bool debug) {
  // Compute maximum marginal
  int n = oracle.num_nodes();
  double max_marginal = -1;
  set<int> empty_set;
  for (int i = 0; i < n; i++) {
    double gain = oracle.MarginalValue(i, empty_set);
    if (gain > max_marginal) {
      max_marginal = gain;
    }
  }

  double gamma = 2.0 * max_marginal / (2.0 * 5.0);  // p = 1 for cardinality

  double max_function_value = -1;
  MaximizationResult ans;

  int rounds = ceil(log(n) / log(1 + epsilon));
  cout << "rounds: " << rounds << endl;
  for (int i = 0; i <= rounds; i++) {
    double rho = gamma * pow(1.0 + epsilon, i);
    cout << "round: " << i << "/" << rounds << "\trho: " << rho << endl;
    MaximizationResult result;
    set<int> S = IGDT(oracle, rho, size_constraint, result, debug);
    cout << "f(S): " << result.function_values.back() << "\t";
    cout << "|S|: " << S.size() << endl << endl;
    if (result.function_values.back() > max_function_value) {
      max_function_value = result.function_values.back();
      ans = result;
    }
  }
  assert(max_function_value != -1);
  return ans;
}

void TestFantom(const EvaluationOracle& oracle,
                int size_constraint, double epsilon, string output_path) {
  cout << "Running fantom...\n";
  const bool debug = true;
  auto result = Fantom(oracle, size_constraint, epsilon, debug);
  string output_filename = output_path;
  output_filename += "constraint_" + int_to_str(size_constraint) + "-";
  output_filename += "epsilon_" + int_to_str(100*epsilon) + "-";
  output_filename += "fantom.txt";
  result.Write(output_filename);
}
