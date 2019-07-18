#include "evaluation_oracle.h"
#include "maximization_result.h"

std::set<int> UnconstrainedMaximization(const EvaluationOracle& oracle,
    const std::set<int>& old_S, std::vector<int> A, double epsilon,
    double delta, MaximizationResult& result);

MaximizationResult AdaptiveNonmonotoneMaximization(
  const EvaluationOracle& oracle, int k, double epsilon, double delta,
  double c1, double c2, double c3, bool debug=false);

void TestAdaptiveNonmonotoneMaximization(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, double delta, std::string output_path);

void TestAdaptiveMaximization(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, double delta, std::string output_path);
