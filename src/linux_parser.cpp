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
          filestream.close();
          return value;
        }
      }
    }
    filestream.close();
  }
  return "Linux";
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
    stream.close();
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
  float value, total, free;
  bool read = true;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line) && read) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == filterMemTotalString) {
          total = value;
        } else if (key == filterMemFreeString) {
          free = value;
        } else {
          read = false;
          filestream.close();
          break;
        }
      }
    }
  }
  float used = total - free;
  return used / total;
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
    stream.close();
  }
  return static_cast<long>(uptime);
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
    stream.close();
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
        if (key == filterCpu) {
          long totalActiveJiffies = 0;
          for (auto& jiffy : jiffies) {
            totalActiveJiffies += jiffy;
          }
          filestream.close();
          return totalActiveJiffies;
        }
      }
    }
    filestream.close();
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
          filestream.close();
          return jiffies[CPUStates::kIdle_] + jiffies[CPUStates::kIOwait_];
        }
      }
    }
    filestream.close();
  }
  return 0;
}

// DONE: Read and return CPU utilization
vector<double> LinuxParser::CpuUtilization() {
  string line;
  string key;
  vector<double> values(10, 0.0);
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
          filestream.close();
          return values;
        }
      }
    }
    filestream.close();
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
        if (key == filterProcesses) {
          filestream.close();
          return value;
        }
      }
    }
    filestream.close();
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
        if (key == filterRunningProcesses) {
          filestream.close();
          return value;
        }
      }
    }
    filestream.close();
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
    stream.close();
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
        // I've used VmData to gives the exact physical memory being used as a
        // part of Physical RAM
        if (key == filterProcMem) {  // filterProcMem("VmData:")
          int ram = value / 1000;
          string strRam = to_string(ram);
          filestream.close();
          return strRam;
        }
      }
    }
    filestream.close();
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
        if (key == filterUID) {
          filestream.close();
          return value;
        }
      }
    }
    filestream.close();
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
          filestream.close();
          return name;
        }
      }
    }
    filestream.close();
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
      if (count == 22) {
        stream.close();
        int upTimePid = UpTime() - stol(value) / sysconf(_SC_CLK_TCK);
        return upTimePid;
      }
    }
    stream.close();
  }
  return 0;
}
