#ifndef EVALUATION_ORACLE_H_
#define EVALUATION_ORACLE_H_

#include <map>
#include <set>
#include <vector>

class EvaluationOracle {
 public:
  EvaluationOracle() : num_nodes_(0), num_edges_(0) {}
  EvaluationOracle(std::string filename, std::string function_name);
  int num_nodes() const { return num_nodes_; }
  int num_edges() const { return num_edges_; }
  std::string function_name() const { return function_name_; }
  const std::vector<std::pair<int, double>>& OutgoingEdges(int node) const;
  const std::vector<std::pair<int, double>>& IncomingEdges(int node) const;
  double Value(const std::set<int>& S) const;
  double MarginalValue(int node, const std::set<int>& S) const;
  double MarginalValue(const std::set<int>& T, const std::set<int>& S) const;
  double GraphCutValue(const std::set<int>& S) const;
  double GraphCutMarginalValue(int node, const std::set<int>& S) const;
  double GraphCutMarginalValue(
      const std::set<int>& T, const std::set<int>& S) const;
  double ImageSummarizationValue(const std::set<int>& S) const;
  double ImageSummarizationMarginalValue(
      int node, const std::set<int>& S) const;
  double ImageSummarizationMarginalValue(
      const std::set<int>& T, const std::set<int>& S) const;
  double MovieRecommendationValue(const std::set<int>& S) const;
  double MovieRecommendationMarginalValue(
      int node, const std::set<int>& S) const;
  double MovieRecommendationMarginalValue(
      const std::set<int>& T, const std::set<int>& S) const;
  double RevenueValue(const std::set<int>& S) const;
  double RevenueMarginalValue(int node, const std::set<int>& S) const;
  double RevenueMarginalValue(
      const std::set<int>& T, const std::set<int>& S) const;
 private:
  int num_nodes_;
  int num_edges_;
  std::vector<std::vector<std::pair<int, double>>> adjacency_list_;
  std::vector<std::vector<std::pair<int, double>>> reverse_adjacency_list_;
  std::vector<std::vector<double>> adjacency_matrix_;
  std::string function_name_;
};

#endif  // EVALUATION_ORACLE_H_
