#include <bits/stdc++.h>

#include "adaptive_maximization.h"
#include "blits.h"
#include "evaluation_oracle.h"
#include "fantom.h"
#include "random_greedy.h"
#include "maximization_result.h"
#include "utilities.h"

int main() {
  std::string input_filename = "data/image-summarization/images_500_graph.txt";
  int size_constraint = 80;
  auto oracle = EvaluationOracle(input_filename, "image_summarization");
  std::string output_path = "output/image-summarization/images_500_graph/";

  /*
  std::string input_filename = "data/movie-recommendation/movies_graph_500.txt";
  int size_constraint = 250;
  auto oracle = EvaluationOracle(input_filename, "movie_recommendation");
  std::string output_path = "output/movie-recommendation/movies_graph_500/";
  */

  /*
  std::string input_filename = "data/youtube-revenue/youtube_graph_1329.txt";
  int size_constraint = 100;
  auto oracle = EvaluationOracle(input_filename, "revenue");
  std::string output_path = "output/youtube-revenue/youtube_graph_1329/";
  */

  std::cout << "Running: " << input_filename << std::endl;
  std::cout << "with cardinality constraint: " << size_constraint << std::endl;
  std::cout << std::endl;

  TestRandom(oracle, size_constraint, output_path);
  TestRandomPrefix(oracle, size_constraint, output_path);
  TestGreedy(oracle, size_constraint, output_path);
  TestRandomGreedy(oracle, size_constraint, output_path);

  const double epsilon = 0.25;
  const double delta = 0.1;
  const int rounds = 10;

  TestRandomLazyGreedyImproved(oracle, size_constraint, output_path);
  TestAdaptiveNonmonotoneMaximization(oracle, size_constraint, epsilon, delta, output_path);
  //TestAdaptiveMaximization(oracle, size_constraint, epsilon, delta, output_path);
  
  TestBlits(oracle, size_constraint, rounds, epsilon, output_path);
  TestFantom(oracle, size_constraint, epsilon, output_path);

  return 0;
}
