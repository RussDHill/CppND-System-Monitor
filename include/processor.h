#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

class Processor {
 public:
  Processor();
  float Utilization();  // DONE: See src/processor.cpp

  // DONE: Declare any necessary private members
 private:
  std::vector<long> values;
  long prevIdle;
  long prevNonIdle;
  long prevTotal;
};

#endif