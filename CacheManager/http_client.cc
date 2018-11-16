#include "http_client.h"
#include <inttypes.h>
#include <iostream>
#include "cache_manager.h"
#include "file_manager.h"

//#define CURL_EASY_MODE
#define CURL_MULTI_MODE

extern CacheManager g_cache_manager;
extern FileManager g_file_manager;
extern std::string g_string;

//std::mutex mutex_h;
//std::condition_variable cv_h;
//int running_h = 0;

HttpClient::HttpClient() :
	curl_(nullptr),
	headers_(nullptr),
	header_size_(0),
    curlm_(nullptr),
    running_(0),
    active_(true)
{
	curl_global_init(CURL_GLOBAL_ALL);
#ifdef CURL_EASY_MODE
    curl_ = curl_easy_init();
#else
    curlm_ = curl_multi_init();
#endif
    //GMessager::GetInstance()->Subcrible(this);
}

HttpClient::~HttpClient()
{
    active_ = false;
	curl_global_cleanup();

    if (headers_)
	    curl_slist_free_all(headers_);
#ifdef CURL_EASY_MODE
	if (curl_)
		curl_easy_cleanup(curl_);
#else
    if (curlm_)
        curl_multi_cleanup(curlm_);
#endif
}


bool HttpClient::Load(Program* program)
{
#ifdef CURL_EASY_MODE
    std::cout << "start load:" << program->GetKey() << std::endl;

    if (!curl_)
        curl_ = curl_easy_init();

    program->SetCurl(curl_);
    SetOpt(program);
    CURLcode res_code = curl_easy_perform(curl_);
    if (res_code != CURLE_OK)
    {
        OnError(res_code);
        return false;
    }
    
    std::cout << "end load" << std::endl;
#else
    if (program)
    {
        //request_list_.push_back(program);
        CURL* curl = curl_easy_init();
        program->SetCurl(curl);
        key_curl_map_[program->GetKey()] = curl;

        SetOpt(program);
        curl_multi_add_handle(curlm_, curl);
        // TODO: 加锁
        running_++;
        cv_.notify_all();
    }

#endif

	return true;
}

bool HttpClient::Stop(Program* program)
{
#ifdef CURL_EASY_MODE    
    if (!curl_)
        return false;
    curl_easy_pause(curl_, CURLPAUSE_ALL);  // 停止上传和下载
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
#else
    if (program)
    {
        CURL* curl = program->GetCurl();
        curl_multi_remove_handle(curlm_, curl);
        curl_easy_cleanup(curl);
        //curl = nullptr;
        printf("%s - Key:\n", __FUNCTION__, program->GetKey().c_str());
    }
#endif
    
    return true;
}

size_t HttpClient::OnProcess(char* data, size_t size, size_t count, void* stream)
{
	//printf("%s - size : %d, count : %d\n", __FUNCTION__, size, count);
	printf(".");
	//g_cache_manager.CacheToMemory(data, size, count, stream);
    Program* program = static_cast<Program*>(stream);
    if (program)
    {
        if (program->GetCacheManager())
            program->GetCacheManager()->CacheToMemory(data, size*count);
        if (program->GetFileManager())
            program->GetFileManager()->AppendToFile(data, size*count);
        //if (program->GetCacheManager() && program->GetFileManager())  // FIXME
        //    program->GetFileManager()->WriteToFile(data, program->GetCacheManager()->GetBufferLength(), size*count);

        program->NotifyParser();
    }

    // 抛出正在处理的消息
    //Message msg;
    //msg.msg_type = PreloadMsg;
    //msg.msg_id = HttpOnProcess;
    //msg.params.push_back(program->GetKey());
    //GMessager::GetInstance()->PostMsg(msg);

	//(static_cast<std::string*>(stream))->append((char*)data, size*count);
	//return fwrite(data, size, count, static_cast<FILE*>(stream));
    return size * count;
}

size_t HttpClient::OnHeader(void * ptr, size_t size, size_t count, void * stream)
{
	//printf("%s - size : %d, count : %d\n", __FUNCTION__, size, count);
	//printf(".");
	return size * count;
}

void HttpClient::OnError(const CURLcode code)
{
	printf("%s - error_code: %d\n", __FUNCTION__, code);
}

void HttpClient::OnError(const CURL * curl)
{
    printf("%s\n", __FUNCTION__);
    for (auto& it : key_curl_map_)
    {
        if (it.second == curl)
        {
            Message msg;
            msg.msg_type = PreloadMsg;
            msg.msg_id = HttpError;
            msg.params.push_back(it.first);
            GMessager::GetInstance()->PostMsg(msg);
            break;
        }
    }
}

void HttpClient::OnComplete(const CURL * curl)
{
    printf("%s\n", __FUNCTION__);
    for (auto& it : key_curl_map_)
    {
        if (it.second == curl)
        {
            Message msg;
            msg.msg_type = PreloadMsg;
            msg.msg_id = HttpComplete;
            msg.params.push_back(it.first);
            GMessager::GetInstance()->PostMsg(msg);
            break;
        }
    }
}

void HttpClient::Process()
{
#ifndef CURL_EASY_MODE
    while (active_)
    {
		if (running_ == 0)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait(lock);
		}
		else
		{
			do
			{
				int numfds = 0;
				int res = curl_multi_wait(curlm_, NULL, 0, 10000, &numfds);  // 等到可以下一次调用
				if (res != CURLM_OK) {
					fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
					//return EXIT_FAILURE;
					continue;
				}
				curl_multi_perform(curlm_, (int*)&running_);	// 执行请求
			} while (running_);

			CURLMsg *msg = NULL;
			CURL *curl = NULL;
			CURLcode return_code = (CURLcode)0;
			int msgs_left = 0;
			int http_status_code;
			const char* url;
			while ((msg = curl_multi_info_read(curlm_, &msgs_left))) {
				if (msg->msg == CURLMSG_DONE) {
					curl = msg->easy_handle;
					return_code = msg->data.result;
					if (return_code != CURLE_OK) {
						fprintf(stderr, "CURL error code: %d\n", msg->data.result);
						this->OnError(curl);
						continue;
					}

					// Get HTTP status code
					http_status_code = 0;
					url = NULL;
					int time = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);
					curl_easy_getinfo(curl, CURLINFO_PRIVATE, &url);
					//curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
					//printf("total time: %d\n", time);

					if (http_status_code == CURLM_OK || http_status_code == 206) {
						printf("OK for %d : %s\n", http_status_code, url);
						this->OnComplete(curl);
					}
					else {
						fprintf(stderr, "GET of %s returned http status code %d\n", url, http_status_code);
						this->OnError(curl);
					}

					curl_multi_remove_handle(curlm_, curl);
					curl_easy_cleanup(curl);
				}
				else {
					fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
					this->OnError(curl);
				}
			}
		}
	}
#endif
}

int HttpClient::GetResponseInfo(const std::string & key)
{
    CURLMsg *msg = NULL;
    CURL *curl = NULL;
    CURLcode return_code = (CURLcode)0;
    int msgs_left = 0;
    int http_status_code;
    const char* url;
    while ((msg = curl_multi_info_read(curlm_, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            curl = msg->easy_handle;

            return_code = msg->data.result;
            if (return_code != CURLE_OK) {
                fprintf(stderr, "CURL error code: %d\n", msg->data.result);
                continue;
            }

            // Get HTTP status code
            http_status_code = 0;
            url = NULL;

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);
            curl_easy_getinfo(curl, CURLINFO_PRIVATE, &url);
            if (FileManager::ParseFilename(std::string(url)) == key)
            {
                curl_multi_remove_handle(curlm_, curl);
                curl_easy_cleanup(curl);
                return http_status_code;
            }

            if (http_status_code == CURLM_OK) {
                printf("200 OK for %s\n", url);
            }
            else {
                fprintf(stderr, "GET of %s returned http status code %d\n", url, http_status_code);
            }

            curl_multi_remove_handle(curlm_, curl);
            curl_easy_cleanup(curl);
        }
        else {
            fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
        }
    }

    return return_code;
}

void HttpClient::SetOpt(Program* program)
{
    Request req;
    req.range_from = program->GetFileManager()->GetFileSize();
    req.range_end = 1024 * 10;  // 10k
	//req.range_end = -1;
    req.timeout = 5000;
    req.url = "http://videostream.iqiyi.com/playback/v/" + program->GetKey();

    headers_ = 0;
	std::string range_field;
	char buf[100];
	if (req.range_end > 0)
		snprintf(buf, sizeof(buf), "%" PRIu64"-%" PRIu64, req.range_from, req.range_end);
	else 
		snprintf(buf, sizeof(buf), "%" PRIu64"-", req.range_from);
	range_field.append(buf);

	for (size_t i = 0; i < req.headers.size(); ++i)
	{
		headers_ = curl_slist_append(headers_, req.headers[i].c_str());
	}

    CURL* curl = program->GetCurl();
	curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_PRIVATE, req.url.c_str());
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
	//curl_easy_setopt(curl_, CURLOPT_BUFFERSIZE, 1024 * 1024);		// buffer size
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 1L);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_);
	//curl_easy_setopt(curl, CURLOPT_USERAGENT, "QYPlayer/Windows/4.2.2;NetType/ethernet");
	curl_easy_setopt(curl, CURLOPT_RANGE, range_field.c_str());
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, req.timeout);  // 设置连接超时，单位秒
	//curl_easy_setopt(curl_, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);	// 设置文件续传位置

	// 设置http头处理函数
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, OnHeader);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_size_);

	// 设置http处理函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnProcess);
	//curl_easy_setopt(curl_, CURLOPT_WRITEDATA, file_);
	//curl_easy_setopt(curl_, CURLOPT_WRITEDATA, g_cache_manager.GetAddress());
	//curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &g_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, program);

	//curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfunc);
	//curl_easy_setopt(curl, CURLOPT_READDATA, f);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);	// 禁止访问超时时抛出超时信号
	//curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);	// 打印信息

    //curl_easy_setopt(curl_, CURLOPT_HEADER, 1);    // 只要求header头，需要服务器支持HEAD方式获取
    //curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, “GET”);    //使用CURLOPT_CUSTOMREQUEST
    //curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);   // 使用HTTPGET
    //curl_easy_setopt(curl_, CURLOPT_NOBODY, 1);    // 不需求body
}

