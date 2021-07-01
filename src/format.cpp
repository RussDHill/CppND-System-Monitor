#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

#include "format.h"

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// CODE FROM:
// https://stackoverflow.com/questions/60046147/how-to-convert-chronoseconds-to-string-in-hhmmss-format-in-c
string Format::ElapsedTime(long s) {
  seconds secs(s);
  auto h = duration_cast<hours>(secs);
  secs -= h;
  auto m = duration_cast<minutes>(secs);
  secs -= m;
  string result;
  chrono::hours h10(10);
  if (h < 10h) result.push_back('0');
  result += std::to_string(h / 1h);
  result += ':';
  if (m < 10min) result.push_back('0');
  result += to_string(m / 1min);
  result += ':';
  if (secs < 10s) result.push_back('0');
  result += to_string(secs / 1s);

  return result;
}