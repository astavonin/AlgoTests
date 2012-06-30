#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>

template<typename T>
void print_buffer(const std::vector<T> buffer)
{
    std::cout << std::hex;
    std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
}

template<typename T>
void isSorted(const std::string fileName)
{
    const size_t inBuffMax = 256*1000*1000;
    std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
    if(in.is_open())
    {
        in.seekg(0, std::ios::end);
        size_t inLen = in.tellg();
        in.seekg(0, std::ios::beg);
        size_t inBuffLen = std::min(inLen, inBuffMax);
        size_t readed = 0;
        while(readed < inBuffLen)
        {
            std::vector<T> buffer(inBuffLen / sizeof(T));
            in.read((char*)&buffer[0], inBuffLen);
            readed += in.gcount();

            T previous = buffer[0];
            for(int i=1; i<buffer.size(); ++i)
            {
                if(previous > buffer[i])
                {
                    std::cout << "File: " << fileName << " is UNSORTED " << std::hex << buffer[i-1] 
                        << " " << buffer[i] << " " << buffer[i+1] << std::endl;
                    return;
                }
                previous = buffer[i];
            }
        }
        std::cout << "File: " << fileName << " is sorted" << std::endl;
        return;
    }
    std::cout << "Can not open: " << fileName << std::endl;
}

template<typename T>
void isSorted(const std::vector<T> buffer, size_t size)
{
    T previous = buffer[0];
    for(int i=1; i<size; ++i)
    {
        if(previous > buffer[i])
        {
            std::cout << "UNSORTED " << std::hex << buffer[i-1] 
                << " " << buffer[i] << std::endl;
            return;
        }
        previous = buffer[i];
    }
}

const std::string genNewTempFileName();

