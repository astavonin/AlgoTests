#pragma once

#include <errno.h>
#include <cstring>

template<typename T>
class BuffersPool
{
public:
    BuffersPool(size_t buffSize, size_t buffersCount)
        : buffSize_(buffSize)
        , buffersCount_(buffersCount)
    {
        for(int i = 0; i < buffersCount_; ++i)
            buffers_.push_back(BuffInfo(buffSize_));
    }
    T& getBuffer()
    {
        std::unique_lock<std::mutex> lock(mutex_);
 
        int idx = -1;
        do
        {
            for(int i = 0; i < buffers_.size(); ++i)
            {
                if(buffers_[i].isFree_)
                {
                    idx = i;
                    buffers_[i].isFree_ = false;
                    break;
                }
            }
            if(idx == -1)
                cv_.wait(lock);
        }
        while(idx == -1);
        std::cout << "using buffer with idx " << idx << std::endl;
        return *buffers_[idx].buffer_;
    }
    void freeBuffer(const T* buffer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for(auto &buf : buffers_)
        {
            if(buf.buffer_.get() == buffer)
            {
                buf.isFree_ = true;
                cv_.notify_all();
                break;
            }
        }
    }
private:
    struct BuffInfo
    {
        BuffInfo(size_t size)
            : buffer_(new T(size))
            , isFree_(true)
        { }
        std::unique_ptr<T> buffer_;
        bool isFree_;
    };
    std::condition_variable cv_;
    std::mutex mutex_;
    std::vector<BuffInfo> buffers_;
    size_t buffSize_;
    size_t buffersCount_;
};

template<typename T>
class AsyncWriter
{
public:
    AsyncWriter(BuffersPool<std::vector<T>> &pool)
        : buffers_(pool)
    {
    }
    ~AsyncWriter()
    {
        isActive_ = false;
        cv_.notify_all();
        thread_.join();
    }
    void start()
    {
        thread_ = std::thread(&AsyncWriter<T>::svc, this);
    }
    void svc()
    {
        while(isActive_)
        {
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk);
            if(!isActive_)
                break;

            std::ofstream out(fileName_.c_str(), std::ios::out | std::ios::binary);
            out.write((char*)&(*data_)[0], data_->size() * sizeof(T));
            buffers_.freeBuffer(data_);
        }
    }
    void writeData(const std::vector<T> *data, const std::string &fileName)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        fileName_ = fileName;
        data_ = data;
        cv_.notify_all();
    }
private:
    const std::vector<T>* data_;
    std::string fileName_;
    std::thread thread_;
    std::condition_variable cv_;
    std::mutex mutex_;
    bool isActive_;
    BuffersPool<std::vector<T>> &buffers_;
};
