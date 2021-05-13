#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>

class Processor {
 public:
  float Utilization();  //  See src/processor.cpp

  //  Declare any necessary private members
 private:
  std::vector<int> SampleCpuUtil() const;
  int idle_{0};
  int active_{0};

};

#endif