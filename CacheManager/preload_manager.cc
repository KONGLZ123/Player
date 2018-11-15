#include "preload_manager.h"
#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include "cache_manager.h"
#include "commons.h"

extern std::list<std::string> g_preload_program_list;
extern std::list<std::string> g_playing_program_list;
extern std::mutex g_mutex;
extern std::condition_variable g_cv;
extern bool g_switch;
extern CacheManager g_cache_manager;
extern std::string g_string;

PreloadManager::PreloadManager(MediaPlayer* player, HttpClient* http_client) :
	active_(false),
	player_(player),
    http_client_(http_client),
    need_preload_(false)
{
	//file_manager_ = new FileManager();
	//http_client_ = new HttpClient();
	GMessager::GetInstance()->Subcrible(this);
}

PreloadManager::~PreloadManager()
{
	//delete file_manager_;
	//file_manager_ = nullptr;
	//delete http_client_;
	//http_client_ = nullptr;
}

void PreloadManager::Start()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	active_ = true;
	g_cv.notify_all();
}

void PreloadManager::Stop()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	active_ = false;
	g_cv.notify_all();
}

void PreloadManager::PreloadProcess()
{
	// �Ӷ�����push����
	while (active_)
	{
        // ʲôʱ��ֹͣԤ���أ�  ��ǰԤ���ض���Ϊ�� | ��ǰ���Ž�Ŀ�ļ��ļ�������� | ��һ�������Ž�Ŀ�ļ��������
        // ʲôʱ����Խ�����    �����ݾͿ��Խ�������ͼ�����ļ�ͷ...
        if (!cur_program_ || cur_program_->GetStatus() == Preloaded)
		{
			if (!next_program_ || (next_program_ && next_program_ && next_program_->GetStatus() == Preloaded))
			{
				//std::cout << "PreloadManager::PreloadProcess - wait" << std::endl;
				std::unique_lock<std::mutex> lock(ul_mutex_);
                printf("PreloadProcess - wait\n");
				cv_.wait(lock);
                printf("PreloadProcess - wait release\n");
			}
            else
            {
                DoPreload(next_program_);
            }
		}
		else
		{
            DoPreload(cur_program_);
		}
	}
}


void PreloadManager::DoPreload(Program * program)
{
    if (program->GetStatus() == NewAdded)
    {
        printf("DoPreload: %s - NewAdded\n", program->GetKey().c_str());
        if (program->GetFileManager() && program->GetFileManager()->FileIsExist())
        {
            FileManager* file_manager = program->GetFileManager();
            if (file_manager)
            {
                //file_manager->ReadFromFile();
                //int file_size = program->ParserHeader();    // ��ͼ�����ļ�ͷ
                //if (file_manager->GetFileSize() == file_size)
                //{
                //    // �����ɹ�
                //}
                //else
                //{
                //    // ����ͷʧ�ܻ����ļ�����ȫ��request
                    player_->OnPreloading(program->GetKey());
					
                    if (http_client_->Load(program))
                    {
                    }
					program->SetStatus(Preloading);
                //}
            }
        }
        else
        {
            player_->OnPreloading(program->GetKey());
            if (!http_client_->Load(program))
            {
            }
			program->SetStatus(Preloading);
        }
    }
    else if (program->GetStatus() == Preloading)
    {
        std::unique_lock<std::mutex> lock(ul_mutex_);
        printf("DoPreload: %s - Preloading wait\n", program->GetKey().c_str());
        cv_.wait(lock);
        printf("DoPreload: %s - Preloading wait release\n", program->GetKey().c_str());
    }
    else if (program->GetStatus() == Preloaded)
    {
        printf("DoPreload: %s - Preloaded\n", program->GetKey().c_str());
        if (next_program_ && next_program_ != program)
            DoPreload(next_program_);
    }
}

void PreloadManager::Process(const Message & msg)
{
	if (msg.msg_type != PreloadMsg)
		return;

	switch (msg.msg_id)
	{
    case NotifyParser:
    {
        // �Ƿ������ͷ
            // ��ֱ��OnWaiting��Ȼ��break
            // �ǣ�������Ƶ�ļ�
    }
    break;
    case HttpError:
    {
        std::cout << "HttpError" << std::endl;
		std::string key = msg.params.front();
		if (cur_program_ && cur_program_->GetKey() == key)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cur_program_->SetStatus(Stoped);
			cv_.notify_all();
			//player_->OnPreloadComplete(cur_program_->GetKey());
		}
		else if (next_program_ && next_program_->GetKey() == key)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			next_program_->SetStatus(Stoped);
			cv_.notify_all();
			//player_->OnPreloadComplete(next_program_->GetKey());
		}
    }
    break;
    case HttpComplete:
    {
        //std::cout << "HttpComplete" << std::endl;
        std::string key = msg.params.front();
		if (cur_program_ && cur_program_->GetKey() == key)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			cur_program_->SetStatus(Preloaded);
			//http_client_->Stop(cur_program_);
			cv_.notify_all();
            printf("HttpComplete - cur_program_\n");
			player_->OnPreloadComplete(cur_program_->GetKey());
		}
		else if (next_program_ && next_program_->GetKey() == key)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			next_program_->SetStatus(Preloaded);
			//http_client_->Stop(cur_program_);
			cv_.notify_all();
            printf("HttpComplete - next_program_\n");
			player_->OnPreloadComplete(next_program_->GetKey());
		}
    }
    break;
	case PreloadProgram:
	{
		std::string key = msg.params.front();	
        // ����Ԥ���ض��У����������10����Ŀ������ж��Ȼ�����һ��
        bool found = false;
        for (auto& it : preload_program_list_)
        {
            if (it && it->GetKey() == key)
            {
                found = true;
                // ����Ԥ����ͷ��
                break;
            }
        }

        if (found == false)
        {
            Program* program = new Program(key);
            if (program)
            {
                player_->OnPushed(key);
                std::lock_guard<std::mutex> lock(mutex_);
				if (!next_program_)
				{
					next_program_ = program;
					next_program_->SetStatus(NewAdded);
				}
				else
				{
					preload_program_list_.push_back(program);
				}
                cv_.notify_all();
            }
        }
	}
	break;
	case PlayProgram:
    {
        std::string key = msg.params.front();

		// �л���Ŀ
        if (cur_program_ && cur_program_->GetKey() != key)
        {
            printf("PlayProgram - switch program\n");
            std::lock_guard<std::mutex> lock(mutex_);
			if (cur_program_->GetStatus() == Preloading)
			{
				//http_client_->Stop(cur_program_);
			}
            deleted_program_list_.push_back(cur_program_);
            //delete cur_program_;
            cur_program_ = nullptr;

            if (next_program_ && next_program_->GetKey() == key)  // Ҫ���ŵĽ�Ŀ����Ϊ��һ�� 
            {
                cur_program_ = next_program_;
                InitProgramStatus(next_program_);               
                cv_.notify_all();
                break;  // ֱ�ӷ���
            }
        }

        // �鿴�Ƿ���ӹ�Ԥ���ض��У���û�У��½�һ��Program
        bool found = false;
        if (found == false)
        {
            for (auto& it : preload_program_list_)
            {
                if (it && it->GetKey() == key)
                {
                    // ����Ԥ����ͷ��
                    std::lock_guard<std::mutex> lock(mutex_);
                    found = true;
					cur_program_ = it;
                    cur_program_->SetStatus(NewAdded);
                    preload_program_list_.remove(it);  // remove��it�ͱ��ͷ���
					if (!next_program_)
					{
						next_program_ = preload_program_list_.front();
                        next_program_->SetStatus(NewAdded);
						preload_program_list_.remove(next_program_);
					}
                    printf("PlayProgram - preload_program_list_ found, notify_all\n");
                    cv_.notify_all();
                    break;
                }
            }
        }
        
        if (found == false)
        {
            Program* program = new Program(key);
            if (program)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                cur_program_ = program;
                cur_program_->SetStatus(NewAdded);
                
                InitProgramStatus(next_program_);
                printf("PlayProgram - new Program, notify_all\n");
                cv_.notify_all();
            }
        }
	}
	break;
	case StopProgram:
	{
		std::string filename = msg.params.front();
	}
	break;
	default:
		break;
	}
}

void PreloadManager::InitProgramStatus(Program * program)
{
    if (preload_program_list_.size() > 0)
    {
        program = preload_program_list_.front();
        program->SetStatus(NewAdded);
        preload_program_list_.remove(program);
    }
    else
    {
        program = nullptr;
    }
}
