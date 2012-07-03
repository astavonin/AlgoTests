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
#include "support.hpp"

using namespace std::chrono;


template<typename T>
class SortBuffer
{
public:

	SortBuffer(size_t buffSize)
        : isFileEmpty_(false)
        , pos_(buffSize)
        , storedCount_(0)
        , capacity_(buffSize)
	{
		data_.resize(buffSize);
	}

    bool setOutFile(const std::string fileName)
    {
        outputStream_.open(fileName.c_str(), std::ios::out | std::ios::binary);
        return outputStream_.is_open();
    }

    bool setInFile(const std::string fileName)
    {
        inputStream_.open(fileName.c_str(), std::ios::in | std::ios::binary);
        if(inputStream_.is_open())
        {
            pos_ = 0;
            storedCount_ = 0;
            isFileEmpty_ = false;
            return true;
        }
        return false;
    }

    void loadBuffer()
    {
        assert(inputStream_.is_open());

        inputStream_.read((char*)&data_[0], capacity_ * sizeof(T));
        pos_ = 0;
        isFileEmpty_ = inputStream_.eof();
        storedCount_ = inputStream_.gcount() / sizeof(T);
    }
    
    void saveBuffer(std::vector<T> &newData, size_t from, size_t count)
    {
        assert(outputStream_.is_open());

        outputStream_.write((char*)&newData[from], count * sizeof(T));
    }
    void saveBuffer()
    {
        assert(outputStream_.is_open());

        outputStream_.write((char*)&data_[0], pos_ * sizeof(T));
    }
    inline bool isFileEmpty()
    {
        return isFileEmpty_;
    }
    inline bool isAllReaded()
    {
        return pos_ >= storedCount_;
    }

    inline size_t unreadCount()
    {
        return storedCount_ - pos_;
    }

    inline size_t pos()
    {
        return pos_;
    }
    inline void pos(size_t newPos)
    {
        pos_ = newPos;
    }
    inline void incPos(size_t count = 1)
    {
        assert(pos_+count <= storedCount_);

        pos_ += count;
    }
    inline size_t storedCount()
    {
        return storedCount_;
    }
    std::vector<T>& data()
    {
        return data_;
    }
    T curValue()
    {
        return data_[pos_];
    }
    void writeItem(T newVal)
    {
        assert(outputStream_.is_open());

        data_[pos_] = newVal;
        pos_++;
    }
    void closeFiles()
    {
        if(inputStream_.is_open())
            inputStream_.close();
        if(outputStream_.is_open())
            outputStream_.close();
    }
private:
    std::ofstream outputStream_;
    std::ifstream inputStream_;
	std::vector<T> data_;
    bool isFileEmpty_;
	size_t pos_;
	size_t storedCount_;
    size_t capacity_;
};

template<typename T>
T calcPivot(const std::vector<T> &arr, int l, int h)
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
void mergeBuffers(SortBuffer<T> &l, SortBuffer<T> &r, SortBuffer<T> &out)
{
    while(l.pos()+r.pos() < l.storedCount() + r.storedCount())
    {
        if(!l.isAllReaded() && !r.isAllReaded())
        {
            if(l.curValue() <= r.curValue())
            {
                out.writeItem(l.curValue());
                l.incPos();
            } else
            {
                out.writeItem(r.curValue());
                r.incPos();
            }
        } else {
			break;
        }
    }
}

typedef std::vector<std::string> FilesList;

template<typename T>
void mergeFiles(const FilesList &list)
{
    size_t inBuffLen = 40*1024*1024 / sizeof(T);
    //size_t inBuffLen = 200;
    size_t outBuffLen = inBuffLen*2;

    SortBuffer<T> inBufferL(inBuffLen);
    SortBuffer<T> inBufferR(inBuffLen);
    SortBuffer<T> outBuffer(outBuffLen);

    FilesList resultList;

	std::cout << "List size " << list.size() << std::endl;

    for(int i=0; i<list.size(); i=i+2)
    {
        if(i+1 >= list.size())
        {
            resultList.push_back(list[i]);
            break;
        }
        inBufferL.setInFile(list[i]);
        inBufferR.setInFile(list[i+1]);
        const std::string outFileName = genNewTempFileName();
        outBuffer.setOutFile(outFileName);

        std::cout << "Left buffer: " << list[i] << ", Right buffer: " << list[i+1] 
                << " Out file: " << outFileName << std::endl;
        do
        {
            if(!inBufferL.isFileEmpty() && inBufferL.isAllReaded())
                inBufferL.loadBuffer();
            if(!inBufferR.isFileEmpty() && inBufferR.isAllReaded())
                inBufferR.loadBuffer();
            outBuffer.pos(0);
            mergeBuffers(inBufferL, inBufferR, outBuffer);
            if(outBuffer.pos() > 0)
                outBuffer.saveBuffer();
            if((!inBufferL.isAllReaded() || !inBufferR.isAllReaded()) &&
                (inBufferL.isFileEmpty() && inBufferR.isFileEmpty()) || outBuffer.pos() == 0)
            {
                auto &currentBuffer = inBufferL.isAllReaded() ? inBufferR : inBufferL ;
                outBuffer.saveBuffer(currentBuffer.data(), currentBuffer.pos(), currentBuffer.unreadCount());
            }
        }while((!inBufferL.isFileEmpty() || !inBufferR.isFileEmpty()) && outBuffer.pos() != 0);

        inBufferL.closeFiles();
        inBufferR.closeFiles();
        outBuffer.closeFiles();

        resultList.push_back(outFileName);
    }
    if(resultList.size() > 1)
        mergeFiles<T>(resultList);
}

void printDiff(const time_point<high_resolution_clock> &start, const time_point<high_resolution_clock> &stop)
{
    std::cout << duration_cast<milliseconds>(stop-start).count() << " millisecond" << std::endl;
}

template<typename T>
FilesList sortFragments(std::ifstream &in)
{
   size_t inBuffLen = 128*1024*1024;
    //size_t inBuffLen = 1000;
    std::vector<T> inBuffer(inBuffLen / sizeof(T));

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

    return files;
}

template<typename T>
void sort(const std::string &inFile, const std::string &outFile)
{
    std::ifstream in(inFile.c_str(), std::ios::in | std::ios::binary);
    if(in.is_open())
    {
        FilesList files = sortFragments<T>(in);
        mergeFiles<T>(files);
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

