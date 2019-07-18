#include "evaluation_oracle.h"
#include "maximization_result.h"

MaximizationResult Fantom(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, bool debug=false);

void TestFantom(const EvaluationOracle& oracle,
    int size_constraint, double epsilon, std::string output_path);
