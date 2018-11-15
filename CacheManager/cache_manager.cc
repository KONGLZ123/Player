#include "cache_manager.h"
#include <cstring>
#include <cstdlib>

CacheManager::CacheManager() :
	max_size_(0),
	real_size_(0),
	free_size_(0)
{
	bin_data_ = (char*)malloc(1024 * 1024 * 2); //分配2M内存
	if (bin_data_)
	{
		max_size_ = 1024 * 1024 * 2;
		real_size_ = 0;
		free_size_ = max_size_;
		memset(bin_data_, 0, max_size_);
	}
}

CacheManager::~CacheManager()
{
    FreeMemory();
}

int CacheManager::CacheToMemory(char* data, size_t size)
{
	if (!bin_data_)
		return 0;

	if (free_size_ > size)  // 能够放下
	{
		memcpy(bin_data_ + real_size_, data, size);
		real_size_ += size;
		free_size_ = max_size_ - real_size_;
	}
	else  // 扩大内存
	{
		//size_t expand_size = size * 2;      // 如果 size = 0，那么相当于调用 free(bin_data_)
		//// bin_data_ = realloc(bin_data_, max_size_);	// relloc分配失败会将bin_data_置空，bin_data_得不到释放
		//char* new_data = (char*)realloc(bin_data_, expand_size);	// 扩大2倍
		//if (new_data)
		//{
		//	bin_data_ = new_data;
		//	max_size_ += expand_size;
		//	memcpy(bin_data_ + real_size_, data, size);  // FIXME: 崩溃
		//	real_size_ += (size);
		//	free_size_ = max_size_ - real_size_;
		//}
		//else
		//{
		//	return 0;
		//}
        size_t expand_size = size * 2;
        char* tmp = (char*)malloc(max_size_ + expand_size);
        if (tmp)
        {
            memcpy(tmp, bin_data_, real_size_);
            free(bin_data_);
            bin_data_ = tmp;
            max_size_ += expand_size;
            memcpy(bin_data_ + real_size_, data, size);
            real_size_ += size;
            free_size_ = max_size_ - real_size_;
        }
        else
        {
            return 0;
        }
	}

	return size;
}

void CacheManager::FreeMemory()
{
    if (bin_data_)
    {
        free(bin_data_);
        bin_data_ = nullptr;

        max_size_ = 0;
        real_size_ = 0;
        free_size_ = 0;
    }
}

void* CacheManager::GetAddress()
{
	if (bin_data_)
		return bin_data_;

	bin_data_ = (char*)malloc(1024 * 1024 * 2); //分配2M内存
	if (bin_data_)
	{
		max_size_ = 1024 * 1024 * 2;
		real_size_ = 0;
		free_size_ = max_size_;
		memset(bin_data_, 0, max_size_);
	}
	else
	{
		max_size_ = 0;
		real_size_ = 0;
		free_size_ = 0;
	}

	return bin_data_;
}
