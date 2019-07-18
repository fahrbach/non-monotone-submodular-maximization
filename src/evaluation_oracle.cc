#include "evaluation_oracle.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::make_pair;
using std::max;
using std::pair;
using std::set;
using std::sqrt;
using std::string;
using std::vector;

EvaluationOracle::EvaluationOracle(string filename, string function_name) {
  // Reads and constructs the 0-index directed multigraph stored in filename.
  assert(function_name == "graph_cut" ||
         function_name == "image_summarization" ||
         function_name == "movie_recommendation" ||
         function_name == "revenue");
  function_name_ = function_name;
  if (function_name == "graph_cut" || 
      function_name == "revenue") {  // Use adjacency list
    ifstream file(filename);
    if (file.is_open()) {
      file >> num_nodes_ >> num_edges_;
      adjacency_list_.resize(num_nodes_);
      reverse_adjacency_list_.resize(num_nodes_);
      int from_node, to_node;
      double weight;
      for (int i = 0; i < num_edges_; i++) {
        file >> from_node >> to_node >> weight;
        assert(0 <= from_node && from_node < num_nodes_);
        assert(0 <= to_node && to_node < num_nodes_);
        adjacency_list_[from_node].push_back(make_pair(to_node, weight));
        reverse_adjacency_list_[to_node].push_back(
            make_pair(from_node, weight));
      }
      file.close();
    } else {
      num_nodes_ = 0;
      num_edges_ = 0;
    }
  } else if (function_name == "image_summarization" ||
             function_name == "movie_recommendation") {  // Use adjacency matrix
    ifstream file(filename);
    if (file.is_open()) {
      file >> num_nodes_ >> num_edges_;
      adjacency_matrix_.resize(num_nodes_);
      for (int i = 0; i < num_nodes_; i++) {
        adjacency_matrix_[i].resize(num_nodes_);
      }
      int from_node, to_node;
      double weight;
      for (int i = 0; i < num_edges_; i++) {
        file >> from_node >> to_node >> weight;
        assert(0 <= from_node && from_node < num_nodes_);
        assert(0 <= to_node && to_node < num_nodes_);
        adjacency_matrix_[from_node][to_node] = weight;
      }
      file.close();
    } else {
      num_nodes_ = 0;
      num_edges_ = 0;
    }
  }
}

const vector<pair<int, double>>& EvaluationOracle::OutgoingEdges(
    int node) const {
  assert(0 <= node && node < num_nodes_);
  return adjacency_list_[node];
}

const vector<pair<int, double>>& EvaluationOracle::IncomingEdges(
    int node) const {
  assert(0 <= node && node < num_nodes_);
  return reverse_adjacency_list_[node];
}

double EvaluationOracle::Value(const set<int>& S) const {
  if (function_name_ == "graph_cut") return GraphCutValue(S);
  if (function_name_ == "image_summarization")
    return ImageSummarizationValue(S);
  if (function_name_ == "movie_recommendation")
    return MovieRecommendationValue(S);
  if (function_name_ == "revenue") return RevenueValue(S);
  assert(false); return 0;
}

double EvaluationOracle::MarginalValue(int node,
                                       const set<int>& S) const {
  if (function_name_ == "graph_cut")
    return GraphCutMarginalValue(node, S);
  if (function_name_ == "image_summarization")
    return ImageSummarizationMarginalValue(node, S);
  if (function_name_ == "movie_recommendation")
    return MovieRecommendationMarginalValue(node, S);
  if (function_name_ == "revenue") return RevenueMarginalValue(node, S);
  assert(false); return 0;
}

double EvaluationOracle::MarginalValue(const set<int>& T,
                                       const set<int>& S) const {
  if (function_name_ == "graph_cut") return GraphCutMarginalValue(T, S);
  if (function_name_ == "image_summarization")
    return ImageSummarizationMarginalValue(T, S);
  if (function_name_ == "movie_recommendation")
    return MovieRecommendationMarginalValue(T, S);
  if (function_name_ == "revenue") return RevenueMarginalValue(T, S);
  assert(false); return 0;
}

// Graph Cuts ------------------------------------------------------------------ 
double EvaluationOracle::GraphCutValue(const set<int>& S) const {
  // Computes the value of the directed cut f(S) from scratch.
  double value = 0;
  for (auto node : S) {
    for (auto kv : OutgoingEdges(node)) {
      if (!S.count(kv.first)) value += kv.second;
    }
  }
  return value;
}

double EvaluationOracle::GraphCutMarginalValue(int node,
                                               const set<int>& S) const {
  // Computes the marginal gain f(S + node) - f(S) for cut functions.
  if (S.count(node)) return 0;
  double value = 0;
  for (auto kv : OutgoingEdges(node)) {
    if (!S.count(kv.first)) value += kv.second;
  }
  for (auto kv : IncomingEdges(node)) {
    if (S.count(kv.first)) value -= kv.second;
  }
  return value;
}

double EvaluationOracle::GraphCutMarginalValue(const set<int>& T,
                                               const set<int>& S) const {
  // Computes the marginal gain f(S + T) - f(S) for cut functions.
  double value = 0;
  for (auto node : T) {
    if (S.count(node)) continue;
    for (auto kv : OutgoingEdges(node)) {
      if (!S.count(kv.first) && !T.count(kv.first)) value += kv.second;
    }
    for (auto kv : IncomingEdges(node)) {
      if (S.count(kv.first)) value -= kv.second;
    }
  }
  return value;
}

// Image Summarization --------------------------------------------------------- 
double EvaluationOracle::ImageSummarizationValue(const set<int>& S) const {
  if (S.size() == 0) return 0;
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) {
    double max_similarity = 0;
    for (auto j : S) {
      max_similarity = max(max_similarity, adjacency_matrix_[i][j]);
    }
    coverage += max_similarity;
  }
  double diversity = 0;
  for (auto i : S) {
    for (auto j : S) {
      diversity += adjacency_matrix_[i][j];
    }
  }
  assert(num_nodes_ > 0);
  double value = coverage - diversity/num_nodes_;
  return value;
}

double EvaluationOracle::ImageSummarizationMarginalValue(
    int node, const set<int>& S) const {
  if (S.count(node)) return 0;
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) {
    double max_similarity = 0;
    for (auto j : S) {
      max_similarity = max(max_similarity, adjacency_matrix_[i][j]);
    }
    double new_max_similarity = max_similarity;
    new_max_similarity = max(new_max_similarity, adjacency_matrix_[i][node]);
    coverage += new_max_similarity - max_similarity;
  }
  double diversity = 0;
  for (auto i : S) {
    diversity += adjacency_matrix_[i][node];
    diversity += adjacency_matrix_[node][i];
  }
  diversity += adjacency_matrix_[node][node];
  assert(num_nodes_ > 0);
  double value = coverage - diversity/num_nodes_;
  return value;
}

double EvaluationOracle::ImageSummarizationMarginalValue(
    const set<int>& T, const set<int>& S) const {
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) {
    double max_similarity = 0;
    for (auto j : S) {
      max_similarity = max(max_similarity, adjacency_matrix_[i][j]);
    }
    double new_max_similarity = max_similarity;
    for (auto j : T) {
      new_max_similarity = max(new_max_similarity, adjacency_matrix_[i][j]);
    }
    coverage += new_max_similarity - max_similarity;
  }
  double diversity = 0;
  for (auto i : S) {
    for (auto j : T) {
      diversity += adjacency_matrix_[i][j];
      diversity += adjacency_matrix_[j][i];
    }
  }
  for (auto i : T) {
    for (auto j : T) {
      diversity += adjacency_matrix_[i][j];
    }
  }
  assert(num_nodes_ > 0);
  double value = coverage - diversity/num_nodes_;
  return value;
}

// Movie Recommendation -------------------------------------------------------- 
double EvaluationOracle::MovieRecommendationValue(const set<int>& S) const {
  if (S.size() == 0) return 0;
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) {
    for (auto j : S) {
      coverage += adjacency_matrix_[i][j];
    }
  }
  double diversity = 0;
  for (auto i : S) {
    for (auto j : S) {
      diversity += adjacency_matrix_[i][j];
    }
  }
  const double lambda = 0.95;
  double value = coverage - lambda * diversity;
  return value;
}
double EvaluationOracle::MovieRecommendationMarginalValue(int node,
                                        const set<int>& S) const {
  if (S.count(node)) return 0;
  assert(0 <= node && node < num_nodes_);
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) coverage += adjacency_matrix_[i][node];
  double diversity = 0;
  for (auto i : S) {
    diversity += adjacency_matrix_[i][node];
    diversity += adjacency_matrix_[node][i];
  }
  diversity += adjacency_matrix_[node][node];
  const double lambda = 0.95;
  double value = coverage - lambda * diversity;
  return value;
}
double EvaluationOracle::MovieRecommendationMarginalValue(const set<int>& T,
                                        const set<int>& S) const {
  double coverage = 0;
  for (int i = 0; i < num_nodes_; i++) {
    for (auto j : T) {
      if (S.count(j)) continue;
      coverage += adjacency_matrix_[i][j];
    }
  }
  double diversity = 0;
  for (auto i : S) {
    for (auto j : T) {
      if (S.count(j)) continue;
      diversity += adjacency_matrix_[i][j];
      diversity += adjacency_matrix_[j][i];
    }
  }
  for (auto i : T) {
    if (S.count(i)) continue;
    for (auto j : T) {
      if (S.count(j)) continue;
      diversity += adjacency_matrix_[i][j];
    }
  }
  const double lambda = 0.95;
  double value = coverage - lambda * diversity;
  return value;
}

// YouTube Revenue -------------------------------------------------------- 
double EvaluationOracle::RevenueValue(const set<int>& S) const {
  if (S.size() == 0) return 0;  // Speedup
  double value = 0;
  for (int i = 0; i < num_nodes_; i++) {
    if (S.count(i)) continue;
    double crossing_degree = 0;
    for (const auto& kv : OutgoingEdges(i)) {
      if (S.count(kv.first)) crossing_degree += kv.second;
    }
    value += sqrt(crossing_degree);
  }
  return value;
}
double EvaluationOracle::RevenueMarginalValue(int node,
                                              const set<int>& S) const {
  set<int> query_set;
  for (auto i : S) query_set.insert(i);
  query_set.insert(node);
  return RevenueValue(query_set) - RevenueValue(S);
}
double EvaluationOracle::RevenueMarginalValue(const set<int>& T,
                                              const set<int>& S) const {
  set<int> query_set;
  for (auto i : S) query_set.insert(i);
  for (auto j : T) query_set.insert(j);
  return RevenueValue(query_set) - RevenueValue(S);
}
