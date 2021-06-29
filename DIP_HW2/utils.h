﻿#ifndef __UTILS_H_
#define __UTILS_H_

constexpr auto M_PI = 3.14159265358979323846;

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


// Affine transform
Mat gammaTransform(Mat src, float gamma);
float* getHist(Mat src);
Mat histEqualization(Mat src);
Mat boxFilter(Size size);
Mat imFilter(Mat src, Mat kernel, string pad_type = "zero");

//Extra functions
uchar gamma_function(Mat src, int target_x, int target_y, float gamma);
uchar filtering_zero(Mat src, Mat kern, int center_x, int center_y);
uchar filtering_mirr(Mat src, Mat kern, int center_x, int center_y);
void print_histogram(float* hist, string str);
#endif