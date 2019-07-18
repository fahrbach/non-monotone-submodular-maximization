#ifndef MAXIMIZATION_RESULT_H_
#define MAXIMIZATION_RESULT_H_

#include <set>
#include <vector>

struct MaximizationResult {
  MaximizationResult();
  bool Write(std::string filename);

  int num_rounds;
  std::vector<std::set<int>> elements_added;
  std::vector<double> marginal_gains;
  std::vector<double> function_values;
  std::vector<int> num_queries;
};

#endif  // MAXIMIZATION_RESULT_H_
