#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using std::all_of;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::replace;
using std::stof;
using std::stoi;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), '=', ' ');
      replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          replace(value.begin(), value.end(), '_', ' ');
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
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
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
      if (all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float memUtil = 0.0, memTotal, memFree;
  string line, key, value;
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") memTotal = stof(value);
        if (key == "MemFree:") memFree = stof(value);
      }
    }
    memUtil = (memTotal - memFree) / memTotal;
  }
  return memUtil;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string line, uptimeStr;
  ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> uptimeStr;
  }
  return stol(uptimeStr);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  /*string line, cpuStr;
  long value, totalJiffies = 0;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> cpuStr;
    while (linestream >> value) {
      totalJiffies += value;
    }
  }
  return totalJiffies;*/
  return (ActiveJiffies() + IdleJiffies());
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line, utimeStr, stimeStr, cutimeStr, cstimeStr;
  long value, activeJiffies = 0;
  ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    int count = 0;
    while ((linestream >> utimeStr) && ++count < 14)
      ;
    linestream >> stimeStr >> cutimeStr >> cstimeStr;
    activeJiffies =
        stol(utimeStr) + stol(stimeStr) + stol(cutimeStr) + stol(cstimeStr);
  }
  return activeJiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpuUtil = CpuUtilization();
  long user = stol(cpuUtil[CPUStates::kUser_]);
  long nice = stol(cpuUtil[CPUStates::kNice_]);
  long system = stol(cpuUtil[CPUStates::kSystem_]);
  long irq = stol(cpuUtil[CPUStates::kIRQ_]);
  long softirq = stol(cpuUtil[CPUStates::kSoftIRQ_]);
  long steal = stol(cpuUtil[CPUStates::kSteal_]);
  return (user + nice + system + irq + softirq + steal);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpuUtil = CpuUtilization();
  long idle = stol(cpuUtil[CPUStates::kIdle_]);
  long iowait = stol(cpuUtil[CPUStates::kIOwait_]);
  return (idle + iowait);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpuUtil;
  string line, key, value;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        while (linestream >> value) {
          cpuUtil.push_back(value);
        }
      }
    }
  }
  return cpuUtil;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  int totalProc = 0;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") totalProc = stoi(value);
      }
    }
  }
  return totalProc;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, value;
  int runProc = 0;
  ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") runProc = stoi(value);
      }
    }
  }
  return runProc;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line, cmd;
  ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    getline(stream, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line, key, value;
  ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") return to_string(std::round(stof(value) / 1024));
      }
    }
  }
  return value;
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line, key, value;
  ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") return value;
      }
    }
  }
  return value;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line, key, value, id;
  ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value >> id) {
        if (uid == id) return key;
      }
    }
  }
  return value;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long int LinuxParser::UpTime(int pid) {
  string line, value;
  ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    int count = 0;
    while ((linestream >> value) && ++count < 22)
      ;
    return UpTime() - stol(value) / sysconf(_SC_CLK_TCK);
  }
  return 0;
}
