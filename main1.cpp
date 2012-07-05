#include <sys/time.h>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <cassert>
#include <thread>
#include "support.hpp"

using namespace std::chrono;

void printDiff(const time_point<high_resolution_clock> &start, const time_point<high_resolution_clock> &stop)
{
    std::cout << duration_cast<milliseconds>(stop-start).count() << " millisecond" << std::endl;
}

template<typename T>
void sort(const std::string &inFile, const std::string &outFile)
{
    size_t inBuffLen = 1024 * 1024;
    size_t outBuffLen = 256 * 1024 * 1024;
    std::vector<T> inBuffer(inBuffLen / sizeof(T));
    std::vector<size_t> outBuffer(outBuffLen / sizeof(size_t));
    size_t iterCount = std::numeric_limits<T>::max() / (outBuffLen / sizeof(T)) + 
                    (std::numeric_limits<T>::max() % (outBuffLen / sizeof(T)) ? 1 : 0);
    std::cout << "Iteration count: " << iterCount << std::endl;
    T fromVal = 0;
    T toVal = std::min((size_t)(outBuffLen / sizeof(T)), (size_t)std::numeric_limits<T>::max());
    std::cout << std::numeric_limits<T>::max() << std::endl;
    std::ofstream out(outFile.c_str(), std::ios::out | std::ios::binary);
    size_t readed = 0;
    while(iterCount > 0)
    {
        std::ifstream in(inFile.c_str(), std::ios::in | std::ios::binary);
        std::fill(outBuffer.begin(), outBuffer.end(), 0);
        do
        {
            in.read((char*)&inBuffer[0], inBuffLen);
            readed = in.gcount() / sizeof(T);
            for(size_t i=0; i < readed; ++i)
            {
                if(fromVal <= inBuffer[i] && inBuffer[i] <= toVal)
                    outBuffer[inBuffer[i]-fromVal]++;
            }
        }while(!in.eof());

        for(size_t i = 0; i < outBuffLen / sizeof(size_t); ++i)
        {
            if(outBuffer[i] > 0)
            {
                T val = fromVal + i;
                for(size_t j=0; j<outBuffer[i]; ++j)
                    out.write((char*)&val, sizeof(T));
            }
        }
        fromVal = toVal + 1;
        toVal += outBuffLen / sizeof(size_t);
        if(toVal == 0)
            toVal = std::numeric_limits<T>::max();
        iterCount--;
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage:\n\talgo_test file_name" << std::endl;
        return -1;
    }

    const std::string input(argv[1]);

    auto start = high_resolution_clock::now();
    sort<uint32_t>(input, input+"_out");
    auto stop = high_resolution_clock::now();
    std::cout << "sort test: ";
    printDiff(start, stop);

    return 0;
}
