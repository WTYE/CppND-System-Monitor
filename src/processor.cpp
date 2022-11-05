#include "processor.h"

#include "linux_parser.h"
#include "process.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float idle = LinuxParser::IdleJiffies();
  float total = LinuxParser::Jiffies();
  float deltaTotal = total - prevTotalJiffies;
  float deltaIdle = idle - prevIdleJiffies;
  float util = deltaTotal > 0.0 ? ((deltaTotal - deltaIdle) / deltaTotal) : 0.0;
  prevTotalJiffies = total;
  prevIdleJiffies = idle;
  return util;
}