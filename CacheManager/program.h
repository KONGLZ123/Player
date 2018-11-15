#pragma once
#include "cache_manager.h"
#include "file_manager.h"
#include "curl.h"
#include "messager.h"
#include "commons.h"

class Program /*: public Observer*/
{
public:
    Program(const std::string& key);
    virtual ~Program();

    std::string GetKey() { return key_; }
    CURL* GetCurl() { return curl_; }
    void SetCurl(CURL* curl) { curl_ = curl; }
	FileManager* GetFileManager();
	CacheManager* GetCacheManager();
    void NotifyParser();
    PreloadStatus GetStatus() { return status_; }
    void SetStatus(const PreloadStatus& status) { status_ = status; }

    virtual void Process(const Message& msg);

private:
    FileManager* file_manager_;
    CacheManager* cache_manager_;
    std::string key_;
    CURL* curl_;
    PreloadStatus status_;
};

