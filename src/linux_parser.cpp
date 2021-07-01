#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  int value, total, free, avail;
  bool read = true;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && read) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          total = value;
        } else if (key == "MemFree:") {
          free = value;
        } else if (key == "MemAvailable:") {
          avail = value;
        } else {
          read = false;
          break;
        }
      }
    }
  }
  int used = total + free - avail;
  return (float)used / (float)total;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  float uptime, idle;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idle;
  }
  return (long)uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value;
  long jiffies[4];
  string strPid = to_string(pid);
  std::ifstream stream(kProcDirectory + strPid + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    int count = 0;
    while (linestream >> value) {
      count++;
      if (count == 13) break;
    }
    linestream >> jiffies[0] >> jiffies[1] >> jiffies[2] >> jiffies[3];
  }
  return jiffies[0] + jiffies[1] + jiffies[2] + jiffies[3];
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  string line;
  string key;
  vector<long> jiffies(10, 0);
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> jiffies[CPUStates::kUser_] >>
             jiffies[CPUStates::kNice_] >> jiffies[CPUStates::kSystem_] >>
             jiffies[CPUStates::kIdle_] >> jiffies[CPUStates::kIOwait_] >>
             jiffies[CPUStates::kIRQ_] >> jiffies[CPUStates::kSoftIRQ_] >>
             jiffies[CPUStates::kSteal_] >> jiffies[CPUStates::kGuest_] >>
             jiffies[CPUStates::kGuestNice_]) {
        if (key == "cpu") {
          long totalActiveJiffies = 0;
          for (auto& jiffy : jiffies) {
            totalActiveJiffies += jiffy;
          }
          return totalActiveJiffies;
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string line;
  string key;
  vector<long> jiffies(10, 0);
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> jiffies[CPUStates::kUser_] >>
             jiffies[CPUStates::kNice_] >> jiffies[CPUStates::kSystem_] >>
             jiffies[CPUStates::kIdle_] >> jiffies[CPUStates::kIOwait_] >>
             jiffies[CPUStates::kIRQ_] >> jiffies[CPUStates::kSoftIRQ_] >>
             jiffies[CPUStates::kSteal_] >> jiffies[CPUStates::kGuest_] >>
             jiffies[CPUStates::kGuestNice_]) {
        if (key == "cpu") {
          return jiffies[CPUStates::kIdle_] + jiffies[CPUStates::kIOwait_];
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return CPU utilization
vector<long> LinuxParser::CpuUtilization() {
  string line;
  string key;
  vector<long> values(10, 0);
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> values[CPUStates::kUser_] >>
             values[CPUStates::kNice_] >> values[CPUStates::kSystem_] >>
             values[CPUStates::kIdle_] >> values[CPUStates::kIOwait_] >>
             values[CPUStates::kIRQ_] >> values[CPUStates::kSoftIRQ_] >>
             values[CPUStates::kSteal_] >> values[CPUStates::kGuest_] >>
             values[CPUStates::kGuestNice_]) {
        if (key == "cpu") {
          return values;
        }
      }
    }
  }
  return {};
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return value;
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;
  string line;
  string strPid = to_string(pid);
  std::ifstream stream(kProcDirectory + strPid + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cmdline;
  }
  return cmdline;
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  int value;
  string strPid = to_string(pid);
  std::ifstream filestream(kProcDirectory + strPid + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          int ram = value / 1000;
          string strRam = to_string(ram);
          return strRam;
        }
      }
    }
  }
  return string();
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key, value;
  string strPid = to_string(pid);
  std::ifstream filestream(kProcDirectory + strPid + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string();
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(string uid) {
  string line;
  string name, hash, value;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> name >> hash >> value) {
        if (value == uid) {
          return name;
        }
      }
    }
  }
  return string();
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  string strPid = to_string(pid);
  std::ifstream stream(kProcDirectory + strPid + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    int count = 0;
    while (linestream >> value) {
      count++;
      if (count == 22) return stol(value);
    }
  }
  return 0;
}
