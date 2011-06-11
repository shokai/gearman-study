// 画像のだいたいの色とサイズを返すgearman worker
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <libgearman/gearman.h>

using namespace boost;
using namespace std;
IplImage *img, *img1px, *imgR, *imgG, *imgB;

void *job_imgcolor(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr);

int main(int argc, char* argv[]) {
  img1px = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 3);
  imgR = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);
  imgG = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);
  imgB = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);

  gearman_worker_st worker;
  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, "127.0.0.1", 7003);
  gearman_worker_add_function(&worker, "img_color", 0, job_imgcolor, NULL);

  while(true) gearman_worker_work(&worker);
  return 0;
}

void *job_imgcolor(gearman_job_st *job, void *cb_arg, size_t *result_size, gearman_return_t *ret_ptr){
  string fileName = (char*)gearman_job_workload(job);
  string result_str = "";
  IplImage *img = cvLoadImage(fileName.c_str());
  if(!img){
    result_str += "{error: image load error}";
  }
  else{
    cvResize(img, img1px, CV_INTER_CUBIC);
    cvSplit(img1px, imgB, imgG, imgR, NULL);
    result_str += str(format("{width: %d, height: %d, r: %d, g: %d, b:%d}")	
		      % img->width % img->height %
		      (uint)(uchar)imgR->imageDataOrigin[0] %
		      (uint)(uchar)imgG->imageDataOrigin[0] %
		      (uint)(uchar)imgB->imageDataOrigin[0]);
    cvReleaseImage(&img);
  }
  cout << fileName << " => " << result_str << endl;
  
  char *result = (char*)strdup(result_str.c_str());
  *result_size = result_str.size();
  *ret_ptr = GEARMAN_SUCCESS;
  return result;
}
