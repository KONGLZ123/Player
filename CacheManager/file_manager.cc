#include "file_manager.h"
#include <io.h>
#include <fstream>
#include <direct.h>

#pragma warning(disable:4996)

FileManager::FileManager(const std::string& filename) :
    filename_(filename)
{
	//root_path_ = "E:\\project_code\\CacheManager\\CacheManager\\cache\\";
    path_ = "E:\\project_code\\CacheManager\\CacheManager\\cache\\" + filename_;
    Mkdir();
    //file_.open(path_, std::ios::binary | std::ios::app | std::ios::out | std::ios::in); // app | ate
}


FileManager::~FileManager()
{
    if (file_.is_open())
    {
        file_.close();
    }
}


std::string FileManager::ParseFilename(const std::string url)
{
	if (url.empty())
		return "";

	return url.substr(url.rfind("/") + 1, url.size());
}

bool FileManager::FileIsExist()
{
	if (filename_.empty() || access(path_.c_str(), ExistenceOnly) == -1)
		return false;

	return true;
}

bool FileManager::FileIsPermissionRW()
{
	if (filename_.empty() || access(path_.c_str(), ReadWritePermission) == -1)
		return false;

	return true;
}

int FileManager::GetFileSize()
{
    if (filename_.empty())
        return 0;

	struct stat file_info;
	if (stat(path_.c_str(), &file_info) == 0)
	{
		return file_info.st_size;
	}

	return 0;
}


bool FileManager::AppendToFile(void * bin_data, const size_t size)
{
	if (!bin_data || filename_.empty() || size <= 0)
		return false;

    if (!file_.is_open())
    {
        file_.open(path_, std::ios::binary | std::ios::app | std::ios::out | std::ios::in);
        if (!file_.is_open())
            return false;
    }
        
	//if (GetFileSize() != cur_file_size)
	//	return false;

    // TODO: 是否全部都是定位到文件末尾
    file_.seekp(0, std::ios::end);  // 定位到文件末尾
	file_.write((char*)bin_data, size);
	//file_.close();

	//采用追加方式打开文件
	//FILE* file = nullptr;
	//std::string path = root_path_ + filename;
	//fopen_s(&file, path.c_str(), "ab+");
	//if (file == nullptr) {
	//    perror(nullptr);
	//	printf("open file failed!\n");
	//	return false;
	//}
	//else
	//{
	//	fwrite(bin_data, 1, size, file);
	//}

	return true;
}

bool FileManager::ReadFromFile(void * bin_data, std::streamoff off, const size_t size)
{
	if (!bin_data || filename_.empty() || size <= 0)
		return false;

    if (!file_.is_open())
    {
        file_.open(path_, std::ios::binary | std::ios::app | std::ios::out | std::ios::in);
        if (!file_.is_open())
            return false;
    }
	
    file_.seekg(off, std::ios::beg);
    //std::streampos pos = file_.tellg();
    //if (off != pos)
    //    return false;

    file_.read((char*)bin_data, size);
    //file_.close();

	//FILE* file = nullptr;
	//std::string path = root_path_ + filename;
	//fopen_s(&file, path.c_str(), "rb+");
	//if (file == nullptr) {
	//	perror(nullptr);
	//	printf("open file failed!\n");
	//	return false;
	//}
	//else
	//{
	//	fread(bin_data, 1, size, file);
	//}

	return true;
}

bool FileManager::WriteToFile(void * bin_data, std::streamoff off, const size_t size)
{
	if (!bin_data || size <= 0)
		return false;

    if (!file_.is_open())
    {
        file_.open(path_, std::ios::binary | std::ios::app | std::ios::out /*| std::ios::in*/);
        if (!file_.is_open())
            return false;
    }

    file_.seekp(off, std::ios::beg);    // 相对于头部的offset
    //std::streampos pos = file_.tellp();
    //if (off != pos)
    //    return false;

    file_.write((char*)bin_data, size);
    //file_.close();

	//采用追加方式打开文件
	//FILE* file = nullptr;
	//std::string path = root_path_ + filename;
	//fopen_s(&file, path.c_str(), "wb+");
	//if (file == nullptr) {
	//	perror(nullptr);
	//	printf("open file failed!\n");
	//	return false;
	//}
	//else
	//{
	//	fwrite(bin_data, 1, size, file);
	//}

	return true;
}

bool FileManager::Mkdir()
{
    //if (path_.empty())
    //    return false;

    size_t pos = path_.find_last_of("/\\");
    std::string path = path_.substr(0, pos);
    if (PathIsExist(path))
        return false;

    return (mkdir(path.c_str()) == 0);   // 文件存在，返回0，不存在，返回-1
}

bool FileManager::DeleteFile()
{
    return (remove(path_.c_str()) == 0);
}

bool FileManager::DeleteCache(const std::string& path)
{
    //if (!PathIsExist(path))
    //    return false;

    //char current_address[100];
    //memset(current_address, 0, 100);
    //getcwd(current_address, 100);

    // 删除文件夹下文件, win32 方式
    _finddata_t file;
    long lf = _findfirst(path.c_str(), &file);
    //输入文件夹路径
    if (lf == -1) 
    {
        printf("cache path: %s not found\n", path.c_str());
        return false;
    }
    else 
    {
        while (_findnext(lf, &file) == 0) 
        {
            //输出文件名
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            std::string filename = path.substr(0, path.size() - 1) + file.name;
            if (remove(filename.c_str()) == 0)
            {
                printf("remove: %s\n", filename.c_str());
            }
        }
    }
    _findclose(lf);

    // 删除文件夹
    // return (rmdir(path.c_str()) == 0);

    return true;
}

bool FileManager::PathIsExist(const std::string & path)
{
    if (path.empty() || access(path.c_str(), ReadWritePermission) == -1)
        return false;
    return true;
}

