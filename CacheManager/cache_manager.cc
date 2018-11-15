#include "cache_manager.h"
#include <cstring>
#include <cstdlib>

CacheManager::CacheManager() :
	max_size_(0),
	real_size_(0),
	free_size_(0)
{
	bin_data_ = (char*)malloc(1024 * 1024 * 2); //����2M�ڴ�
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

	if (free_size_ > size)  // �ܹ�����
	{
		memcpy(bin_data_ + real_size_, data, size);
		real_size_ += size;
		free_size_ = max_size_ - real_size_;
	}
	else  // �����ڴ�
	{
		//size_t expand_size = size * 2;      // ��� size = 0����ô�൱�ڵ��� free(bin_data_)
		//// bin_data_ = realloc(bin_data_, max_size_);	// relloc����ʧ�ܻὫbin_data_�ÿգ�bin_data_�ò����ͷ�
		//char* new_data = (char*)realloc(bin_data_, expand_size);	// ����2��
		//if (new_data)
		//{
		//	bin_data_ = new_data;
		//	max_size_ += expand_size;
		//	memcpy(bin_data_ + real_size_, data, size);  // FIXME: ����
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

	bin_data_ = (char*)malloc(1024 * 1024 * 2); //����2M�ڴ�
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
