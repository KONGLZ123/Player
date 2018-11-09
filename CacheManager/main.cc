#include <iostream>
#include <string>

#pragma warning(disable:4996)

#define CURL_STATICLIB
#include "curl.h"
#include "easy.h"

FILE *fp;  //定义FILE类型指针
           //这个函数是为了符合CURLOPT_WRITEFUNCTION而构造的
           //完成数据保存功能
size_t wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, static_cast<FILE*>(stream));
}

/* 从http头部获取文件size*/
size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream) {
    int r;
    long len = 0;

    /* _snscanf() is Win32 specific */
    // r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
    r = sscanf(static_cast<const char*>(ptr), "Content-Length: %ld\n", &len);
    if (r) /* Microsoft: we don't read the specs */
        *((long *)stream) = len;

    return size * nmemb;
}

int download(CURL *curl, const char * remotepath, const char * localpath,
    long timeout, long tries)
{
    FILE *f;
    curl_off_t local_file_len = -1;
    long filesize = 0;
    struct curl_slist* headers = NULL;

    CURLcode r = CURLE_GOT_NOTHING;
    int c;
    struct stat file_info;
    int use_resume = 0;
    curl_slist_append(headers, "qyid:123466789");

    /* 得到本地文件大小 */
    //if(access(localpath,F_OK) ==0)

    if (stat(localpath, &file_info) == 0)
    {
        local_file_len = file_info.st_size;
        use_resume = 1;
    }
    //采用追加方式打开文件，便于实现文件断点续传工作
    fopen_s(&f, localpath, "ab+");
    if (f == NULL) {
        perror(NULL);
        return 0;
    }

    //curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    //curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 131072);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "QYPlayer/Windows/4.2.2;NetType/ethernet");

    std::string range_field;
    range_field = "0-1024000";
    //if (request.range_end > 0)
    //    range_field.Format("%" PRIu64"-%" PRIu64, request.range_from, request.range_end);
    //else
    //    range_field.Format("%" PRIu64"-", request.range_from);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_field.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, remotepath);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);  // 设置连接超时，单位秒

    // 设置文件续传的位置给libcurl
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);


    //设置http 头部处理函数
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &filesize);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wirtefunc);

    //curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfunc);
    //curl_easy_setopt(curl, CURLOPT_READDATA, f);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


    r = curl_easy_perform(curl);


    fclose(f);
    curl_slist_free_all(headers);

    if (r == CURLE_OK)
        return 1;
    else {
        fprintf(stderr, "%s\n", curl_easy_strerror(r));
        return 0;
    }
}


int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    std::string url = "http://videostream.iqiyi.com/playback/v/0839560d9e904601a7d0324a91056673";
    std::string filename = url.substr(url.rfind("/") + 1, url.size());
    //fopen_s(&fp, filename.c_str(), "w");
    //if (fp == NULL)
    //{
    //    //curl_easy_cleanup(curl);
    //    exit(1);
    //}

    CURL* curl = curl_easy_init();
    filename = "E:\\project_code\\CacheManager\\CacheManager\\" + filename;
    download(curl, url.c_str(), filename.c_str(), 3, 1);

    //CURL* curl = curl_easy_init();
    //if (curl)
    //{
    //    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    //    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wirtefunc);
    //    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    //    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &filesize);
    //    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);
    //    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    //    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    //    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    //    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    //    CURLcode res_code = curl_easy_perform(curl);
    //    if (res_code != CURLE_OK)
    //    {
    //        std::cout << "request failed!" << std::endl;
    //    }
    //    curl_easy_cleanup(curl);
    //}    
    
    curl_global_cleanup();
    //Sleep(3000);

    return 0;
}

