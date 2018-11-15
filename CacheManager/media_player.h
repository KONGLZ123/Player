#pragma once
#include <vector>
#include <string>
#include "messager.h"

class MediaPlayer : public Observer
{
public:
	MediaPlayer();
	virtual ~MediaPlayer();

	void Initialize();
	void Play(const std::string key);
	void Stop(const std::string key);
	void Push(const std::string key);

	void Start();
	void End();
	void SwitchLast();
	void SwitchNext();
	void SwitchRight();

	void OnPushed(const std::string key);			// push成功回调
	void OnPreloading(const std::string key);		// 预加载中
	void OnPreloadComplete(const std::string key);	// preload完成回调
	void OnPreloadStoped(const std::string key);	// 停止preload

	void OnPlaying(const std::string key);	// playing
	void OnWaiting(const std::string key);	// 等待播放
	void OnStop(const std::string key);		// 停止播放

	virtual void Process(const Message& msg);

private:
	std::vector<std::string> program_list_;
	size_t cur_index_;
};

