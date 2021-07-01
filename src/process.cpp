#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), cpu_usage(0.f) {}

// DONE: Return this process's ID
int Process::Pid() { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() { 

  float uptime = UpTime();
  float total_time = LinuxParser::ActiveJiffies(pid_);
  float starttime = LinuxParser::UpTime(pid_);
  float hertz = sysconf(_SC_CLK_TCK);

  float seconds = uptime - (starttime / hertz);

  cpu_usage = ((total_time / hertz) / seconds);

  return cpu_usage; 
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// DONE: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// DONE: Return the user (name) that generated this process
string Process::User() {
  string uid = LinuxParser::Uid(pid_);
  return LinuxParser::User(uid);
}

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
  return a.cpu_usage < this->cpu_usage;
}
