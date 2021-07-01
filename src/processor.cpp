#include "processor.h"

using LinuxParser::CPUStates;
using std::vector;

Processor::Processor() : prevIdle(0), prevNonIdle(0), prevTotal(0) {}

// DONE: Return the aggregate CPU utilization
// CALCULATION FROM: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
float Processor::Utilization() {

  vector<long> values = LinuxParser::CpuUtilization();
  
  long idle = values[CPUStates::kIdle_] + values[CPUStates::kIOwait_];

  long nonIdle = values[CPUStates::kUser_] + values[CPUStates::kNice_] +
                 values[CPUStates::kSystem_] + values[CPUStates::kIRQ_] +
                 values[CPUStates::kSoftIRQ_] + values[CPUStates::kSteal_];

  long total = idle + nonIdle;

  float totald = total - prevTotal;
  float idled = idle - prevIdle;

  float utilization = (totald - idled) / totald;

  prevIdle = idle;
  prevNonIdle = nonIdle;
  prevTotal = totald;

  return utilization;
}
