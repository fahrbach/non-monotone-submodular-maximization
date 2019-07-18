#include "evaluation_oracle.h"
#include "maximization_result.h"

MaximizationResult Blits(const EvaluationOracle& oracle, int k, double epsilon,
    bool debug=false);

void TestBlits(const EvaluationOracle& oracle, int size_constraint, int rounds,
    double epsilon, std::string output_path);
