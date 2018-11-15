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
    std::list<Program*> play_program_list_;     // �����Ŷ��У�Ԥ������ɣ����Բ��ŵĽ�Ŀ
    std::list<Program*> preload_program_list_;  // Ԥ���ض��У������صĽ�Ŀ����ǰ�����ȼ����
    std::list<Program*> deleted_program_list_;  // Ԥ���ض��У������صĽ�Ŀ����ǰ�����ȼ����
    Program* cur_program_;  // ��ǰ���Ž�Ŀ
    Program* next_program_;  // ��һ�����Ž�Ŀ
    volatile bool need_preload_;
};

