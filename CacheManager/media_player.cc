#include "media_player.h"
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>


extern std::mutex g_mutex;
extern std::condition_variable g_cv;
extern std::list<std::string> g_preload_program_list;
extern std::list<std::string> g_playing_program_list;
extern bool g_switch;

MediaPlayer::MediaPlayer() :
	cur_index_(0)
{
	program_list_.clear();
	GMessager::GetInstance()->Subcrible(this);
}


MediaPlayer::~MediaPlayer()
{
}

void MediaPlayer::Initialize()
{
	std::fstream file;
	std::string root_path = "E:\\project_code\\CacheManager\\CacheManager\\";  // TODO: 全局数据管理
	std::string filename = root_path + "program_list.txt";
	file.open(filename, std::ios::in);
	if (file)
	{
		std::string key;
		int line_num = 0;
		while (!file.eof() && line_num++ < 10) {
			file >> key;
			//std::cout << key << std::endl;
			program_list_.push_back(key);
		}
	}
	else 
	{
		std::cout << "file not exist: " << filename << std::endl;
	}
	file.close();
}

void MediaPlayer::Start()
{
	if (program_list_.size() > 0)
		Play(program_list_[cur_index_]);

    for (auto& it : program_list_)
    {
        Push(it);
    }
}

void MediaPlayer::End()
{
}

void MediaPlayer::Play(const std::string key)
{
	Message msg;
	msg.msg_type = PreloadMsg;
	msg.msg_id = PlayProgram;
	msg.params.push_back(key);
	GMessager::GetInstance()->PostMsg(msg);

	//bool found = false;
	//for (auto& it : g_playing_program_list)
	//{
	//	if (it == key)	
	//	{
	//		found = true;
	//		break;
	//	}
	//}

	//if (found)	// 节目可以播放
	//{
	//	std::cout << "paly found" << std::endl;
	//	OnPlaying(key);
	//	Push(program_list_[cur_index_ + 1]);	// 缓存下一个
	//}
	//else // 节目不可以播放
	//{
	//	std::unique_lock<std::mutex> lock(g_mutex);
	//	Push(key);	// 缓存当前
	//	std::cout << "paly not found, wait" << std::endl;
	//	g_cv.wait(lock);	// 线程阻塞在这里
	//	Play(key);
	//}
}

void MediaPlayer::Stop(const std::string key)
{
	Message msg;
	msg.msg_type = PreloadMsg;
	msg.msg_id = StopProgram;
	msg.params.push_back(key);
	GMessager::GetInstance()->PostMsg(msg);
}

void MediaPlayer::Push(const std::string key)
{
	//std::cout << "MediaPlayer::Push" << std::endl;
	//g_preload_program_list.push_back(key);
	//g_cv.notify_all();

	Message msg;
	msg.msg_type = PreloadMsg;
	msg.msg_id = PreloadProgram;
	msg.params.push_back(key);
	GMessager::GetInstance()->PostMsg(msg);
}

void MediaPlayer::SwitchLast()
{
    if (cur_index_ > 0 && cur_index_ < program_list_.size())
    {
        std::cout << "===================MediaPlayer::SwitchLast" << std::endl;
        Play(program_list_[--cur_index_]);
    }
    else
    {
        std::cout << "no last program to play: " << cur_index_ << std::endl;
    }
}

void MediaPlayer::SwitchNext()
{
	if (cur_index_ + 1 < program_list_.size())
	{
		std::cout << "===================MediaPlayer::SwitchNext" << std::endl;
		Play(program_list_[++cur_index_]);
	}
	else
	{
		std::cout << "no next program to play: " << cur_index_ << std::endl;
	}
}

void MediaPlayer::SwitchRight()
{
	//Message msg;
	//msg.msg_type = PreloadMsg;
	//msg.msg_id = SwitchRightProgram;
	//msg.params.push_back(program_list_[--cur_index_]);
}

void MediaPlayer::OnPushed(const std::string key)
{
	//std::cout << "OnPushed:" << key << std::endl;
}

void MediaPlayer::OnPreloading(const std::string key)
{
	std::cout << "OnPreloading:" << key << std::endl;
}

void MediaPlayer::OnPreloadComplete(const std::string key)
{
	std::cout << "OnPreloadComplete:" << key << std::endl;
}

void MediaPlayer::OnPreloadStoped(const std::string key)
{
	std::cout << "OnPreloadStoped:" << key << std::endl;
}

void MediaPlayer::OnPlaying(const std::string key)
{
	std::cout << "OnPlaying:" << key << std::endl;
}

void MediaPlayer::OnWaiting(const std::string key)
{
}

void MediaPlayer::OnStop(const std::string key)
{
}

void MediaPlayer::Process(const Message & msg)
{
	if (msg.msg_type != MediaMsg)
		return;

	switch (msg.msg_id)
	{
	default:
		break;
	}
}
