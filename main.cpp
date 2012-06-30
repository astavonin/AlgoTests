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

using namespace std::chrono;

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
    std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
    if(in.is_open())
    {
        in.seekg(0, std::ios::end);
        size_t inLen = in.tellg();
        in.seekg(0, std::ios::beg);
        std::vector<T> buffer(inLen / sizeof(T));
        in.read((char*)&buffer[0], inLen);

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


template<typename T>
int calcPivot(const std::vector<T> &arr, int l, int h)
{
    return arr[ (l + h) / 2 ];
}

template<typename T>
void intQsort(std::vector<T> &arr, int left, int right)
{
    int i = left, j = right;
    T pivot = calcPivot( arr, left, right);
    while( i <= j )
    {
        while( arr[ i ] < pivot ) i++;
        while( arr[ j ] > pivot ) j--;
        if( i <= j )
        {
            std::swap( arr[ i ], arr[ j ] );
            i++;
            j--;
        }
    };

    if( left < j )
        intQsort( arr, left, j );
    if( i < right )
        intQsort( arr, i, right );
}

void qsort(std::vector<uint32_t> &arr)
{
    intQsort( arr, 0, arr.size()-1 );
}

template<typename T>
struct SortBuffer
{
	size_t pos;
	size_t count;
	std::vector<T> data;
	
	SortBuffer(size_t buffSize)
	{
		data.resize(buffSize);
		pos = 0;
	}
};

template<typename T>
void mergeBuffers(SortBuffer<T> &l, SortBuffer<T> &r, SortBuffer<T> &out)
{
    size_t delta = l.pos+r.pos;
    size_t writePos = 0;
    while(l.pos+r.pos < l.count + r.count)
    {
        if((l.pos < l.count) && (r.pos < r.count))
        {
            if(l.data[l.pos] <= r.data[r.pos])
            {
                out.data[writePos] = l.data[l.pos];
                l.pos++;
            } else
            {
                out.data[writePos] = r.data[r.pos];
                r.pos++;
            }
            writePos++;
        } else {
			break;
        }
    }
    out.pos = l.pos+r.pos-delta;
}

const std::string genNewTempFileName()
{
    static int lastUsedFileNumber = 0;

    std::stringstream tmpBuffer;
    tmpBuffer << "tmp_" << std::setfill('0') << std::setw(2) << lastUsedFileNumber;
    lastUsedFileNumber++;

    return tmpBuffer.str();
}

typedef std::vector<std::string> FilesList;

template<typename T>
void mergeFiles(const FilesList &list)
{
    size_t inBuffLen = 50*1000*1000 / sizeof(T);
    //size_t inBuffLen = 200;
    size_t outBuffLen = inBuffLen*2;

    SortBuffer<T> inBufferL(inBuffLen);
    SortBuffer<T> inBufferR(inBuffLen);
    SortBuffer<T> outBuffer(outBuffLen);

    FilesList resultList;

	std::cout << "List size " << list.size() << std::endl;

    for(int i=0; i<list.size(); i=i+2)
    {
        std::ifstream fileLeft(list[i].c_str(), std::ios::in | std::ios::binary);
        std::ifstream fileRight(list[i+1].c_str(), std::ios::in | std::ios::binary);

        if(i+1 >= list.size())
        {
            resultList.push_back(list[i]);
            break;
        }
        std::cout << "Left buffer: " << list[i] << ", Right buffer: " << list[i+1] << std::endl;
        bool isEmptyLeft = false;
        bool isEmptyRight = false;
        inBufferL.pos = inBufferR.pos = outBuffer.pos = inBuffLen;
        inBufferL.count = inBufferR.count = inBuffLen;
        const std::string outFileName = genNewTempFileName();
        std::cout << "Out file: " << outFileName << std::endl;
        std::ofstream out(outFileName.c_str(), std::ios::out | std::ios::binary);
        do
        {
            if(!isEmptyLeft && inBufferL.pos == inBuffLen)
            {
                fileLeft.read((char*)&inBufferL.data[0], inBuffLen * sizeof(T));
                inBufferL.pos = 0;
                if(isEmptyLeft = fileLeft.eof())
                    inBufferL.count = fileLeft.gcount() / sizeof(T);
                std::cout << "Left: " << inBufferL.count << std::endl;
            }
            if(!isEmptyRight && inBufferR.pos == inBuffLen)
            {
                fileRight.read((char*)&inBufferR.data[0], inBuffLen * sizeof(T));
                inBufferR.pos = 0;
                if(isEmptyRight = fileRight.eof())
                    inBufferR.count = fileRight.gcount() / sizeof(T);
                std::cout << "Right: " << inBufferR.count << std::endl;
            }
            outBuffer.pos = 0;
            mergeBuffers(inBufferL, inBufferR, outBuffer);
            if(outBuffer.pos)
            {
                //isSorted(outBuffer.data, outBuffer.pos);
                out.write((char*)&outBuffer.data[0], outBuffer.pos * sizeof(T));
                std::cout << "To out: " << outBuffer.pos
                    << " Left: " << inBufferL.count - inBufferL.pos << " " << isEmptyLeft
                    << " Right: " << inBufferR.count - inBufferR.pos << " " <<isEmptyRight
                    << std::endl;
            }
            if((inBufferL.count - inBufferL.pos > 0 || inBufferR.count - inBufferR.pos > 0) &&
                (isEmptyLeft && isEmptyRight) || outBuffer.pos == 0)
            {
                auto &currentBuffer = (inBufferL.count - inBufferL.pos > 0) ?
                    inBufferL : inBufferR;
                std::cout << "To out (1): " << currentBuffer.count - currentBuffer.pos
                    << " Left: " << inBufferL.count - inBufferL.pos
                    << " Right: " << inBufferR.count - inBufferR.pos
                    << std::endl;
                out.write((char*)&currentBuffer.data[currentBuffer.pos], (currentBuffer.count - currentBuffer.pos) * sizeof(T));
            }
        }while((!isEmptyLeft || !isEmptyRight) && outBuffer.pos != 0);
        //isSorted<T>(outFileName);
        resultList.push_back(outFileName);
    }
    if(resultList.size() > 1)
        mergeFiles<T>(resultList);
}

template<typename T>
void sort(const std::string &inFile, const std::string &outFile)
{
   size_t inBuffLen = 100*1000*1000;
    //size_t inBuffLen = 1000;
    std::vector<T> inBuffer(inBuffLen / sizeof(T));

    std::ifstream in(inFile.c_str(), std::ios::in | std::ios::binary);
    if(in.is_open())
    {
        in.seekg(0, std::ios::end);
        size_t inLen = in.tellg();
        //size_t inLen = 10 * 1000;
        in.seekg(0, std::ios::beg);

        int fragments = inLen / inBuffLen;
        FilesList files(fragments);
        std::cout << "Total fragments: " << fragments << std::endl;
        for(int i=0; i < fragments; i++)
        {
            std::cout << "Current fragment: " << i << std::endl;
            in.read((char*)&inBuffer[0], inBuffLen);

            qsort(inBuffer);
            files[i] = genNewTempFileName();
            std::ofstream out(files[i].c_str(), std::ios::out | std::ios::binary);
            out.write((char*)&inBuffer[0], inBuffLen);
        }
        mergeFiles<T>(files);
    }
}

void rwTest(const std::string &inFile, const std::string &outFile)
{
    size_t inBuffLen = 256*1000*1000;
    std::vector<uint32_t> inBuffer(inBuffLen / sizeof(uint32_t));

    std::ifstream in(inFile.c_str(), std::ios::in | std::ios::binary);
    if(in.is_open())
    {
        in.seekg(0, std::ios::end);
        size_t inLen = in.tellg();
        in.seekg(0, std::ios::beg);

        int fragments = inLen / inBuffLen;
        std::ofstream out(outFile.c_str(), std::ios::out | std::ios::binary);
        for(int i=0; i < fragments; i++)
        {
            in.read((char*)&inBuffer[0], inBuffLen);
            inBuffer[10] = 100;

            out.write((char*)&inBuffer[0], inBuffLen);
        }
    }
}

void printDiff(const time_point<high_resolution_clock> &start, const time_point<high_resolution_clock> &stop)
{
    std::cout << duration_cast<milliseconds>(stop-start).count() << " millisecond" << std::endl;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage:\n\tyandex file_name" << std::endl;
        return -1;
    }

    const std::string input(argv[1]);

    auto start = high_resolution_clock::now();
    //rwTest(input, input+"_out");
    auto stop = high_resolution_clock::now();
    std::cout << "R/W test: ";
    printDiff(start, stop);

    start = high_resolution_clock::now();
    sort<uint32_t>(input, input+"_out");
    stop = high_resolution_clock::now();
    std::cout << "sort test: ";
    printDiff(start, stop);

    return 0;
}

