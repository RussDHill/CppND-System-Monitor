#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

class Processor {
 public:
  Processor();
  double Utilization();  // DONE: See src/processor.cpp

  // DONE: Declare any necessary private members
 private:
  std::vector<double> values;
  double prevIdle;
  double prevNonIdle;
  double prevTotal;
};

#endif