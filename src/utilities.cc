#include <sstream>
#include "utilities.h"

using std::string;
using std::stringstream;

string int_to_str(int n) {
  stringstream ss;
  ss << n;
  string ans;
  ss >> ans;
  return ans;
}
