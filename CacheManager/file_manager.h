#pragma once
#include <string>
#include <fstream>

class FileManager
{
public:
	explicit FileManager(const std::string& filename);
	virtual ~FileManager();

	static std::string ParseFilename(const std::string url);
    static bool DeleteCache(const std::string& path);
    static bool PathIsExist(const std::string& path);

	bool FileIsExist();
	bool FileIsPermissionRW();
	int GetFileSize();
	bool AppendToFile(void* bin_data, const size_t size);
	bool ReadFromFile(void* bin_data, std::streamoff off, const size_t size);
	bool WriteToFile(void* bin_data, std::streamoff off, const size_t size);
    bool Mkdir();
    bool DeleteFile();
    

private:
	enum Mode
	{
		ExistenceOnly = 0,
		WritePermission = 2,
		ReadPermission = 4,
		ReadWritePermission = 6
	};
	//std::string root_path_;
    std::fstream file_;
    std::string filename_;
    std::string path_;
};

