#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <experimental/filesystem>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
namespace fs = std::experimental::filesystem;

// An example of how to read data from the filesystem
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

//  Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  vector<int> mem_elts(4);

  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  if (stream.is_open()) {
    for (auto& elt : mem_elts) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream.ignore(256, ' ');
      linestream >> elt;
    }
  }
  // For readibility:
  int mem_total = mem_elts[0];
  // int mem_free = mem_elts[1];
  int mem_available = mem_elts[2];
  // int buffers = mem_elts[3];

  int mem_used = mem_total - mem_available;
  return mem_used / (float) mem_total; 
}

//  Read and return the system uptime
long LinuxParser::UpTime() {
  string line, uptime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return (long) std::stoi(uptime);
}

//  Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

//  Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line, token;
  string utime, stime, cutime, cstime, starttime;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);

  constexpr int UNTIL_JIFFIES = 13;

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for(int i = 0; i < UNTIL_JIFFIES; i++) linestream >> token;
    linestream >> utime >> stime >> cutime >> cstime;
  }
  return std::stoi(utime) + std::stoi(stime) + std::stoi(cutime) + std::stoi(cstime);
}



//  Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_utilization_as_str = CpuUtilization();
  vector<int> cpu_utilization;
  for (auto const& u : cpu_utilization_as_str) {
    cpu_utilization.push_back(std::stoi(u));
  }
  return cpu_utilization[kUser_] + cpu_utilization[kNice_] + cpu_utilization[kSystem_] + 
         cpu_utilization[kIRQ_] + cpu_utilization[kSoftIRQ_] + cpu_utilization[kSteal_];

}

//  Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu_utilization = CpuUtilization();
  return std::stoi(cpu_utilization[kIdle_]) + std::stoi(cpu_utilization[kIOwait_]);
}

//  Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> cpu_utilizations(10);
  string line, cpu;
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> cpu;
  for (auto& utilization : cpu_utilizations){
    linestream >> utilization;
  }
  return cpu_utilizations;
}

//  Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, first_word;
  int total_processes{0};
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()){
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> first_word;
      if (first_word == "processes"){
        linestream >> total_processes;
        break;
      }
    }
  }
  return total_processes;
}

//  Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line, key;
  int procs_running{0};
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()){
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running"){
        linestream >> procs_running;
      }
    }
  }
  return procs_running;
}

//  Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

//  Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line, first_word, mem_util{"0"};
  int mem_usg_in_mb{0};

  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> first_word;
      if (first_word == "VmSize:") {
        linestream >> mem_util;
      }
    }
  }

  mem_usg_in_mb = std::stoi(mem_util) / (float) 1024;
  return std::to_string(mem_usg_in_mb);
}

//  Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line, key, uid;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
        break;
      }
    }    
  }
  return uid;
}

//  Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  std::ifstream stream(kPasswordPath);
  string uid_desired = Uid(pid);
  string line, uname, passwd, uid;
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> uname >> passwd >> uid;
      if (uid == uid_desired) {
        break;
      }
    }
  }
  return uname;
}

//  Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  string line, token, uptime;
  constexpr int UNTIL_UPTIME = 21;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < UNTIL_UPTIME; i++) linestream >> token;
    linestream >> uptime;
  }
  return std::stoi(uptime);
}
