#include <sstream>
#include <iomanip>
#include "support.hpp"

const std::string genNewTempFileName()
{
    static int lastUsedFileNumber = 0;

    std::stringstream tmpBuffer;
    tmpBuffer << "tmp_" << std::setfill('0') << std::setw(2) << lastUsedFileNumber;
    lastUsedFileNumber++;

    return tmpBuffer.str();
}
