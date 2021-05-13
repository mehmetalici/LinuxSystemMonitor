#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;


//  Return this process's CPU utilization
float Process::CpuUtilization() const { 
     long starttime = LinuxParser::UpTime(pid_) / sysconf(_SC_CLK_TCK);
     long system_uptime = LinuxParser::UpTime();
     long elapsed_time = system_uptime - starttime;

     long active_time = LinuxParser::ActiveJiffies(pid_) / sysconf(_SC_CLK_TCK);
     return active_time / (float) elapsed_time;
}

//  Return the command that generated this process
string Process::Command() const { 
    return LinuxParser::Command(pid_);
}

int Process::Pid() const { return pid_; }; 

//  Return this process's memory utilization
string Process::Ram() const { 
    return LinuxParser::Ram(pid_);
}

//  Return the user (name) that generated this process
string Process::User() const { 
    return LinuxParser::User(pid_);
}

//  Return the age of this process (in seconds)
long int Process::UpTime() const { 
    return LinuxParser::UpTime(pid_);
}

//  Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return CpuUtilization() < a.CpuUtilization();
}