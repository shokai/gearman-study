// 画像サイズを返すgearman worker
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/any.hpp>
#include <libgearman/gearman.h>
#include "json_builder.h"

using namespace boost;
using namespace std;

tuple<int, int> get_size(const string& fileName); // 画像のwidth,heightを返す
map<string,any> imgsize(const string& fileName); // gearman workerとしてclientに返すためのJSON Objectを作る
void *job_imgsize(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr);
void on_exit_signal(int sig);
vector<int> pids;

int main(int argc, char* argv[]) {
  program_options::options_description opts("options");
  opts.add_options()
    ("help,h", "helpを表示")
    ("server,s", program_options::value<string>(), "gearmanサーバーのアドレス")
    ("port,p", program_options::value<int>(), "gearmanサーバーのport番号")
    ("fork", program_options::value<int>(), "preforkする数")
    ("test,t", program_options::value<string>(), "gearman worker単体テスト用query");
  program_options::variables_map argmap;
  program_options::store(parse_command_line(argc, argv, opts), argmap);
  program_options::notify(argmap);

  if(!argmap.count("help")){
    if(argmap.count("test")){
      cout << "---test---" << endl;
      string gearman_param = argmap["test"].as<string>();
      cout << json_builder::toJson(imgsize(gearman_param)) << endl; // 単体でworkerとしてのテスト
      return 0;
    }else if(argmap.count("server") && argmap.count("port")){
      if(argmap.count("fork")){
	int i, pid;
	for(i = 1; i < argmap["fork"].as<int>(); i++){
	  pid = fork();
	  if(pid == 0){ // 子プロセス
	    pids.clear();
	    break;
	  }
	  else{ // 親プロセス
	    pids.push_back(pid);
	    cout << str(format("fork:%d - parent:%d child:%d") % 
			i %
			getpid() %
			pid) << endl;
	  }
	}
      }
      if(pids.size() > 0){ // 親プロセスの終了シグナルをフックする
	signal(SIGTERM, on_exit_signal);
	signal(SIGHUP, on_exit_signal);
      }
      gearman_worker_st worker;
      gearman_worker_create(&worker);
      string g_server = argmap["server"].as<string>();
      int g_port = argmap["port"].as<int>();

      struct hostent *g_host = gethostbyname((char*)g_server.c_str());
      string g_server_addr = str(format("%d.%d.%d.%d") %
				 (uint)(uchar)g_host->h_addr[0] %
				 (uint)(uchar)g_host->h_addr[1] %
				 (uint)(uchar)g_host->h_addr[2] %
				 (uint)(uchar)g_host->h_addr[3]);

      gearman_worker_add_server(&worker, g_server_addr.c_str(), g_port);
      gearman_worker_add_function(&worker, "img_size", 0, job_imgsize, NULL);
      cout << str(format("---start worker (%s:%d)---") %
		  g_server_addr % g_port) << endl;
      while(true) gearman_worker_work(&worker); // workerとして待機
      return 0;
    }
  }
  cerr << "server,portが必要です" << endl;
  cerr << opts << endl;
  return 1;
  
}

// opencvで画像サイズを取得
tuple<int, int> get_size(const string& fileName){
  IplImage *img = cvLoadImage(fileName.c_str());
  if(!img){
    return make_tuple(-1, -1);
  }
  else{
    int width = img->width;
    int height = img->height;
    cvReleaseImage(&img);
    return make_tuple(width, height);
  }
}

// 画像サイズを取得してgearman serverに返すJSON Objectを作る
map<string,any> imgsize(const string& fileName){
  map<string,any> result_m;
  int width, height;
  tie(width, height) = get_size(fileName);
  if(width > 0 && height > 0){
    result_m["width"] = width;
    result_m["height"] = height;
  }
  else{
    result_m["error"] = string("image load error");
  }
  return result_m;
}

// gearman worker job
void *job_imgsize(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr){
  string fileName = (char*)gearman_job_workload(job);
  cout << fileName << endl;
  string result_str = json_builder::toJson(imgsize(fileName));
  cout << " => " << result_str << endl;
  char *result = (char*)strdup(result_str.c_str());
  *result_size = result_str.size();
  *ret_ptr = GEARMAN_SUCCESS;
  return result;
}

void on_exit_signal(int sig){
  for(int i = 0; i < pids.size(); i++){
    cout << str(format("kill (pid:%d)") % pids[i]) << endl;
    if(kill(pids[i], SIGKILL) < 0){
      cerr << str(format("kill failed (pid:%d)") % pids[i]) << endl;
    }
  }
  exit(0);
}
