#include <cassert>
#include <iostream>
#include <fstream>
#include "maximization_result.h"

using std::cerr;
using std::endl;
using std::ofstream;

MaximizationResult::MaximizationResult() {
  num_rounds = 0;
  elements_added.resize(1);
  marginal_gains.resize(1);
  function_values.resize(1);
  num_queries.resize(1);
}

bool MaximizationResult::Write(std::string filename) {
  ofstream file(filename);
  if (file.is_open()) {
    file << "num_rounds num_elements_added marginal_gains ";
    file << "function_values num_queries" << std::endl; 
    for (int i = 0; i <= num_rounds; i++) {
      file << i << " ";
      file << elements_added[i].size() << " ";
      file << marginal_gains[i] << " ";
      file << function_values[i] << " ";
      file << num_queries[i] << std::endl;
    }
    return true;
  }
  cerr << "filepath does not exist: " << filename << endl;
  assert(false);
  return false;
}
