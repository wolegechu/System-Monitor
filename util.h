#ifndef UTIL
#define UTIL
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>

using namespace std;
// Classic helper function
class Util {

public:

static std::string convertToTime ( long int input_seconds );
static std::string getProgressBar(std::string percent);
static std::ifstream getStream(const std::string &path);
static vector<string> getValuesFromString(const std::string &s);
};
#endif