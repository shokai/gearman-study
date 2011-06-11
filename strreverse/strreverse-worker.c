// 文字列をreverseして返すworker
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgearman/gearman.h>

void *job_str_reverse(gearman_job_st *job, void *cb_arg, size_t *result_size,
                      gearman_return_t *ret_ptr){
  char *str = (char *)gearman_job_workload(job);
  int len = strlen(str);
  printf("str:%s  length:%d\n", str, len);

  char reverse[len];
  for(int i = 0; i < len; i++){
    reverse[i] = str[len-i-1]; // 文字列を逆にする
  }

  char *result = strdup(reverse); // 結果の文字列はコピーしてポインタで返す
  *result_size= gearman_job_workload_size(job);
  *ret_ptr= GEARMAN_SUCCESS;
  return result;
}


int main(int argc, char *argv[]){
  gearman_return_t ret;
  gearman_worker_st worker;

  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, "127.0.0.1", 7003);
  gearman_worker_add_function(&worker, "str_reverse", 0, job_str_reverse, NULL);
  
  while(1) gearman_worker_work(&worker); // ジョブ登録したらループで待つ

  gearman_worker_free(&worker);
  return 0;
}

