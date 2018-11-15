#include "program.h"


Program::Program(const std::string& key) :
    file_manager_(nullptr),
    cache_manager_(nullptr),
    key_(key)
{
    file_manager_ = new FileManager(key_);
    cache_manager_ = new CacheManager();
    //GMessager::GetInstance()->Subcrible(this);
}


Program::~Program()
{
    delete file_manager_;
    file_manager_ = nullptr;
    delete cache_manager_;
    cache_manager_ = nullptr;
}

FileManager * Program::GetFileManager()
{
	return file_manager_; 
}

CacheManager * Program::GetCacheManager()
{
	return cache_manager_;
}

void Program::NotifyParser()
{
    Message msg;
    msg.msg_type = PreloadMsg;
    msg.msg_id = MsgId::NotifyParser;
    msg.params.push_back(key_);
    GMessager::GetInstance()->PostMsg(msg);
}

void Program::Process(const Message & msg)
{
}


