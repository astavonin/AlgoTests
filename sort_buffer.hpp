#pragma once

template<typename T>
class SortBuffer
{
public:

	SortBuffer(size_t buffSize)
        : pos_(buffSize)
	{
		data_.resize(buffSize);
	}
    virtual ~SortBuffer() {}

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
    std::vector<T>& data()
    {
        return data_;
    }
protected:
	std::vector<T> data_;
	size_t pos_;
};

template<typename T>
class SortInputBuffer : protected SortBuffer<T>
{
public:
    SortInputBuffer(size_t buffSize) 
        : SortBuffer<T>(buffSize)
        , isFileEmpty_(false)
        , storedCount_(0)
        , capacity_(buffSize)
    {
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
    T curValue()
    {
        return data_[pos_];
    }
    inline size_t storedCount()
    {
        return storedCount_;
    }
    void loadBuffer()
    {
        assert(inputStream_.is_open());

        inputStream_.read((char*)&data_[0], capacity_ * sizeof(T));
        pos_ = 0;
        isFileEmpty_ = inputStream_.eof();
        storedCount_ = inputStream_.gcount() / sizeof(T);
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
    std::vector<T>& data()
    {
        return data_;
    }
    void close()
    {
        if(inputStream_.is_open())
            inputStream_.close();
    }
private:
    std::ifstream inputStream_;
    bool isFileEmpty_;
	size_t storedCount_;
    size_t capacity_;
    using SortBuffer<T>::pos_;
    using SortBuffer<T>::data_;
};

template<typename T>
class SortOutputBuffer : public SortBuffer<T>
{
public:
    SortOutputBuffer(size_t buffSize) 
        : SortBuffer<T>(buffSize)
    {
    }
    bool setOutFile(const std::string fileName)
    {
        outputStream_.open(fileName.c_str(), std::ios::out | std::ios::binary);
        return outputStream_.is_open();
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
    void writeItem(T newVal)
    {
        assert(outputStream_.is_open());

        data_[pos_] = newVal;
        pos_++;
    }
    void close()
    {
        if(outputStream_.is_open())
            outputStream_.close();
    }
private:
    std::ofstream outputStream_;
    using SortBuffer<T>::pos_;
    using SortBuffer<T>::data_;
};

