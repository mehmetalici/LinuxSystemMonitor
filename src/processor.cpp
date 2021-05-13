#include "processor.h"
#include "linux_parser.h"
#include <vector>


using std::vector;
using std::string;

//  Return the aggregate CPU utilization
float Processor::Utilization() {

    long idle = LinuxParser::IdleJiffies();
    long prevIdle = this->idle_;

    long active = LinuxParser::ActiveJiffies();
    long prevActive = this->active_;

    long total = idle + active;
    long prevTotal = prevIdle + prevActive;

    long totald = total - prevTotal;
    long idled = idle - prevIdle;

    float cpuPerc = (totald - idled) / (float) totald;

    this->idle_ = idle;
    this->active_ = active;

    return cpuPerc;


}