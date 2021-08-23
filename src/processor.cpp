#include "processor.h"

using LinuxParser::CPUStates;
using std::vector;

Processor::Processor() : prevIdle(0.0), prevNonIdle(0.0), prevTotal(0.0) {}

// DONE: Return the aggregate CPU utilization
// CALCULATION FROM:
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
double Processor::Utilization() {
  vector<double> values = LinuxParser::CpuUtilization();

  double idle = values[CPUStates::kIdle_] + values[CPUStates::kIOwait_];

  double nonIdle = values[CPUStates::kUser_] + values[CPUStates::kNice_] +
                   values[CPUStates::kSystem_] + values[CPUStates::kIRQ_] +
                   values[CPUStates::kSoftIRQ_] + values[CPUStates::kSteal_];

  double total = idle + nonIdle;

  double totald = total - prevTotal;
  double idled = idle - prevIdle;

  double utilization = (totald - idled) / totald;

  prevIdle = idle;
  prevNonIdle = nonIdle;
  prevTotal = total;

  return utilization;
}
