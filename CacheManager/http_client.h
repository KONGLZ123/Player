#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "curl.h"
#include "easy.h"
#include "program.h"

struct Request
{
	std::string url;
	int64_t range_from;
	int64_t range_end;
	std::vector<std::string> headers;
	int timeout;
	std::string filename;

	Request()
	{
		url = "";
		range_from = 0;
		range_end = -1;
		headers.clear();
		timeout = 3;	// д╛хо3s
		filename = "";
	}
};

class HttpClient /*: public Observer*/
{
public:
	HttpClient();
	virtual ~HttpClient();
	bool Load(Program* program);
    bool Stop(Program* program);

	static size_t OnProcess(char* ptr, size_t size, size_t count, void* stream);
	static size_t OnHeader(void* ptr, size_t size, size_t count, void* stream);
	void OnError(const CURLcode code);
	void OnError(const CURL* curl);
	void OnComplete(const CURL* curl);
    void Process();
    int GetResponseInfo(const std::string& key);
    //virtual void Process(const Message& msg) {}

private:
	void SetOpt(Program* program);
	
private:
	CURL* curl_;
    CURLM* curlm_;
	std::list<Program*> request_list_;
    std::map<std::string, CURL*> key_curl_map_;
	struct curl_slist* headers_;
	int64_t header_size_;
	volatile int running_;
    bool active_;
    std::mutex mutex_;
    std::condition_variable cv_;
	//char* buffer_;
	//FILE* file_;
};

