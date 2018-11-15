#pragma once
class CacheManager
{
public:
	CacheManager();
	~CacheManager();

	int CacheToMemory(char* ptr, size_t size);
	void FreeMemory();
	void* GetAddress();
	size_t GetBufferLength() { return real_size_; }
	size_t GetFreeSize() { return free_size_; }

private:
	char* bin_data_;
	size_t real_size_;
	size_t free_size_;
	size_t max_size_;
};

