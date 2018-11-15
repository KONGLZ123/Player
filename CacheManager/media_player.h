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

	void OnPushed(const std::string key);			// push�ɹ��ص�
	void OnPreloading(const std::string key);		// Ԥ������
	void OnPreloadComplete(const std::string key);	// preload��ɻص�
	void OnPreloadStoped(const std::string key);	// ֹͣpreload

	void OnPlaying(const std::string key);	// playing
	void OnWaiting(const std::string key);	// �ȴ�����
	void OnStop(const std::string key);		// ֹͣ����

	virtual void Process(const Message& msg);

private:
	std::vector<std::string> program_list_;
	size_t cur_index_;
};

