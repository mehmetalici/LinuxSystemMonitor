#include <string>
#include <sstream>
#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    int minutes = seconds / 60;
    seconds = seconds % 60;
    int hours = minutes / 60;
    minutes = minutes % 60;

    std::ostringstream stringStream;
    stringStream << hours << ":" << minutes << ":" << seconds;
    return stringStream.str();
}