#include "evaluation_oracle.h"
#include "maximization_result.h"

MaximizationResult Random(const EvaluationOracle& oracle, int size_constraint,
    bool prefix=true, bool debug=false);

MaximizationResult Greedy(const EvaluationOracle& oracle, int size_constraint,
    bool debug=false);

MaximizationResult RandomGreedy(const EvaluationOracle& oracle,
    int size_constraint, bool debug=false);

// Comparing Apples and Oranges: Query Trade-off in Submodular Maximization
MaximizationResult RandomLazyGreedyImproved(const EvaluationOracle& oracle,
    int size_constraint, double delta, bool debug=false);

void FillM(const EvaluationOracle& oracle, const std::set<int>& S,
    const std::set<int>& true_S, std::set<int>& M, int size_constraint,
    double delta, double& w, const double W, MaximizationResult& result,
    bool debug=false);

void TestRandom(const EvaluationOracle& oracle,
    int size_constraint, std::string output_path);

void TestRandomPrefix(const EvaluationOracle& oracle,
    int size_constraint, std::string output_path);

void TestGreedy(const EvaluationOracle& oracle,
    int size_constraint, std::string output_path);

void TestRandomGreedy(const EvaluationOracle& oracle,
    int size_constraint, std::string output_path);

void TestRandomLazyGreedyImproved(const EvaluationOracle& oracle,
    int size_constraint, std::string output_path);
