#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "http_client.h"
#include "media_player.h"
#include "preload_manager.h"
#include "cache_manager.h"
#include "messager.h"
#include "mp4_parser.h"

#pragma warning(disable:4996)

#define SLEEP(x) (std::this_thread::sleep_for(std::chrono::seconds(x)))

std::list<std::string> g_preload_program_list;
std::list<std::string> g_playing_program_list;
bool g_switch = false;
std::mutex g_mutex;
std::condition_variable g_cv;
CacheManager g_cache_manager;
//FileManager g_file_manager;
std::string g_string;


void PlayerThreadFunc(MediaPlayer* player)
{
	std::cout << "player_thread id:" << std::this_thread::get_id() << std::endl;

	player->Start();
    auto start = std::chrono::high_resolution_clock::now();
    SLEEP(10);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " ms\n";

	player->SwitchNext();
    SLEEP(5);
	player->SwitchLast();
	
    SLEEP(3);
	player->SwitchNext();
    SLEEP(5);
	player->SwitchNext();
    SLEEP(5);
	player->SwitchNext();
    SLEEP(10);
	player->SwitchNext();
	SLEEP(10);
	player->SwitchNext();
	SLEEP(10);
	player->SwitchNext();
	SLEEP(10);
	player->SwitchNext();
	player->End();

	//std::unique_lock<std::mutex> lock(g_mutex);
	//std::notify_all_at_thread_exit(g_cv, std::move(lock));
	std::cout << "player_thread quit" << std::endl;
}

void PreloadThreadFunc(PreloadManager* preload)
{
	preload->PreloadProcess();
	std::cout << "preload_thread quit" << std::endl;
}

void HttpThreadFunc(HttpClient* http_client)
{
    http_client->Process();
}


#define MAX_WAIT_MSECS 30*1000 /* Wait max. 30 seconds */

static const char *urls[] = {
    "http://www.microsoft.com",
    "http://www.baidu.com",
    "http://www.cn.bing.com",
    "http://www.iqiyi.com"
};

#define CNT 4

static size_t cb(char *d, size_t n, size_t l, void *p)
{
    /* take care of the data here, ignored in this example */
    (void)d;
    (void)p;
    return n*l;
}

static void init(CURLM *cm, int i)
{
    CURL *eh = curl_easy_init();
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
    curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
    curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
    curl_multi_add_handle(cm, eh);
}

void func(std::weak_ptr<std::string> str)
{
    if (str.lock())
        std::cout << *str.lock() << std::endl;
}

int main(int argc, char* argv[])
{
	std::cout << "main thread id:" << std::this_thread::get_id() << std::endl;

	//MediaPlayer* player = new MediaPlayer();
 //   HttpClient* http_client = new HttpClient();
	//PreloadManager* preload = new PreloadManager(player, http_client);
 //   FileManager::DeleteCache("E:\\project_code\\CacheManager\\CacheManager\\cache\\*");

	//player->Initialize();
	//preload->Start();

	//std::thread player_thread(PlayerThreadFunc, player);
 //   std::thread http_client_thread(HttpThreadFunc, http_client);
	//std::thread preload_thread(PreloadThreadFunc, preload);

	//player_thread.join();
	////preload->Stop();
	////std::unique_lock<std::mutex> lock(g_mutex);
	////std::notify_all_at_thread_exit(g_cv, std::move(lock));
 //   http_client_thread.join();
	//preload_thread.join();

	//delete player;
	//delete preload;
 //   delete http_client;

    Mp4Parser mp4_parser;
    mp4_parser.Parser();

    //CURLM *cm = NULL;
    //CURL *eh = NULL;
    //CURLMsg *msg = NULL;
    //CURLcode return_code = (CURLcode)0;
    //int still_running = 0, i = 0, msgs_left = 0;
    //int http_status_code;
    //const char *szUrl;

    //curl_global_init(CURL_GLOBAL_ALL);

    //cm = curl_multi_init();

    //for (i = 0; i < CNT; ++i) {
    //    init(cm, i);
    //}

    ////curl_multi_perform(cm, &still_running);

    //do {
    //    int numfds = 0;
    //    int res = curl_multi_wait(cm, NULL, 0, MAX_WAIT_MSECS, &numfds);
    //    if (res != CURLM_OK) {
    //        fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
    //        return EXIT_FAILURE;
    //    }
    //    /*
    //    if(!numfds) {
    //    fprintf(stderr, "error: curl_multi_wait() numfds=%d\n", numfds);
    //    return EXIT_FAILURE;
    //    }
    //    */
    //    curl_multi_perform(cm, &still_running);

    //} while (still_running);

    //while ((msg = curl_multi_info_read(cm, &msgs_left))) {
    //    if (msg->msg == CURLMSG_DONE) {
    //        eh = msg->easy_handle;

    //        return_code = msg->data.result;
    //        if (return_code != CURLE_OK) {
    //            fprintf(stderr, "CURL error code: %d\n", msg->data.result);
    //            continue;
    //        }

    //        // Get HTTP status code
    //        http_status_code = 0;
    //        szUrl = NULL;

    //        curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
    //        curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);

    //        if (http_status_code == 200) {
    //            printf("200 OK for %s\n", szUrl);
    //        }
    //        else {
    //            fprintf(stderr, "GET of %s returned http status code %d\n", szUrl, http_status_code);
    //        }

    //        curl_multi_remove_handle(cm, eh);
    //        curl_easy_cleanup(eh);
    //    }
    //    else {
    //        fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
    //    }
    //}

    //curl_multi_cleanup(cm);

    //return EXIT_SUCCESS;

    return 0;
}

