#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <libgearman/gearman.h>

using namespace boost;
using namespace std;

void *job_transimg(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr);

int main(int argc, char* argv[]) {
  cout << argv[1] << endl;
  
  gearman_worker_st worker;
  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, "127.0.0.1", 7003);
  gearman_worker_add_function(&worker, "transimg", 0, job_transimg, NULL);

  while(true) gearman_worker_work(&worker);
  return 0;
}

void *job_transimg(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr){
  string request = (char*)gearman_job_workload(job);
  cout << request << endl;
  string result_str = "";
  regex delimiter("([^,]+)");
  vector<string> params;

  regex_split(back_inserter(params), request, delimiter);
  string fileName = params[0];
  int h = 0;
  int s = 0;
  int v = 0;
  if(params.size() > 1) h = atoi(params[1].c_str());
  if(params.size() > 2) s = atoi(params[2].c_str());
  if(params.size() > 3) v = atoi(params[3].c_str());
  cout << fileName << endl;
  cout << "h: " << h << endl;
  cout << "s: " << s << endl;
  cout << "v: " << v << endl;

  IplImage *img = cvLoadImage(fileName.c_str());
  if(!img){
    result_str += "error: image not found";
  }

  IplImage *imgHsv = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
  IplImage *imgResult = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
  IplImage *imgHue = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  IplImage *imgSat = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  IplImage *imgVal = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  cvCvtColor(img, imgHsv, CV_BGR2HSV);
  cvSplit(imgHsv, imgHue, imgSat, imgVal, NULL);
  cvAddS(imgHue, cvScalarAll(h), imgHue, NULL);
  cvAddS(imgSat, cvScalarAll(s), imgSat, NULL);
  cvAddS(imgVal, cvScalarAll(v), imgVal, NULL);
  cvMerge(imgHue, imgSat, imgVal, NULL, imgHsv);
  cvCvtColor(imgHsv, imgResult, CV_HSV2BGR);

  string out_filename = "/Users/sho/out.jpg";
  cout << "save! " << out_filename << endl;
  cvSaveImage(out_filename.c_str(), imgResult);

  char *result = (char*)strdup(result_str.c_str());
  *result_size = gearman_job_workload_size(job);
  *ret_ptr = GEARMAN_SUCCESS;
  return result;
}
