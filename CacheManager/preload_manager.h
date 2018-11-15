#pragma once
#include "file_manager.h"
#include "http_client.h"
#include "media_player.h"
#include "messager.h"



class PreloadManager : public Observer
{
public:
	PreloadManager(MediaPlayer* player, HttpClient* http_client);
	virtual ~PreloadManager();

	void Start();
	void Stop();
	bool GetStatus() { return active_; }
	void PreloadProcess();
    void DoPreload(Program* program);
	virtual void Process(const Message& msg);
    void InitProgramStatus(Program* program);
    //static void HttpFunc(const HttpClient* http_client, const std::string& filename);

private:
	MediaPlayer* player_;
	//FileManager* file_manager_;
	HttpClient* http_client_;
    volatile bool active_;
    std::mutex mutex_;
    std::mutex ul_mutex_;
    std::condition_variable cv_;
    std::list<Program*> play_program_list_;     // 待播放队列，预加载完成，可以播放的节目
    std::list<Program*> preload_program_list_;  // 预加载队列，待加载的节目，最前面优先级最高
    std::list<Program*> deleted_program_list_;  // 预加载队列，待加载的节目，最前面优先级最高
    Program* cur_program_;  // 当前播放节目
    Program* next_program_;  // 下一个播放节目
    volatile bool need_preload_;
};

