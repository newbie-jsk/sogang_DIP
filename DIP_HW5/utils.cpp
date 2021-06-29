#include "utils.h"
#include <math.h>

#define PI 3.141592

Mat cvtBGR2HSI(Mat src)
{
	//BGR to HSI
	//Src format : floating point. value range 0 - 1. 3 channel color.
	float theta, H, S, I;
	float R, G, B, min;
	Mat dst(src.size(), src.type());

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			B = src.at<Vec3f>(i, j)[0];
			G = src.at<Vec3f>(i, j)[1];
			R = src.at<Vec3f>(i, j)[2];

			theta = acos(((2.0 * R - G - B) / 2.0) / sqrt((R - G) * (R - G) + (R - B) * (G - B)));
			theta = (theta) * 180.0 / PI;
			if (B > G)
				H = 360.0 - theta;
			else
				H = theta;

			if (R > G)
			{
				if (G > B)
					min = B;
				else
					min = G;
			}
			else
			{
				if (R > B)
					min = B;
				else
					min = R;
			}
			S = 1.0 - 3.0 * min / (R + G + B);
			I = (R + G + B) / 3.0;

			dst.at<Vec3f>(i, j)[0] = H / 360.0; // 0 to 1 normalize
			dst.at<Vec3f>(i, j)[1] = S;
			dst.at<Vec3f>(i, j)[2] = I;
		}
	}


	return dst;
}

Mat cvtHSI2BGR(Mat src)
{
	//HSI to BGR
	float R, G, B;
	float H, S, I;
	Mat dst(src.size(), src.type());

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			H = src.at<Vec3f>(i, j)[0];
			S = src.at<Vec3f>(i, j)[1];
			I = src.at<Vec3f>(i, j)[2];

			H = 2.0 * H * PI; // convert to radian (H * 360 * PI / 180)

			if (H < 2.0 * PI / 3.0) // H < 120
			{
				B = I * (1 - S);
				R = I * (1 + (S * cos(H)) / cos(PI / 3.0 - H));
				G = 3 * I - (R + B);
			}
			else if (H < 4.0 * PI / 3.0) // H < 240
			{
				H = H - 2 * PI / 3;

				R = I * (1 - S);
				G = I * (1 + (S * cos(H)) / cos(PI / 3.0 - H));
				B = 3 * I - (R + G);
			}
			else
			{
				H = H - 4 * PI / 3;

				G = I * (1 - S);
				B = I * (1 + (S * cos(H)) / cos(PI / 3.0 - H));
				R = 3 * I - (G + B);
			}

			dst.at<Vec3f>(i, j)[0] = B;
			dst.at<Vec3f>(i, j)[1] = G;
			dst.at<Vec3f>(i, j)[2] = R;
		}
	}
	return dst;
}

Mat sobelFilter(Mat src, float direction)
{
	Mat sobel(3, 3, CV_32FC1, Scalar(0));
	if (direction == 0)
	{
		sobel.at<float>(0, 0) = -1;
		sobel.at<float>(1, 0) = -2;
		sobel.at<float>(2, 0) = -1;

		sobel.at<float>(0, 2) = 1;
		sobel.at<float>(1, 2) = 2;
		sobel.at<float>(2, 2) = 1;
	}
	else if (direction == 1)
	{
		sobel.at<float>(0, 0) = -1;
		sobel.at<float>(0, 1) = -2;
		sobel.at<float>(0, 2) = -1;

		sobel.at<float>(2, 0) = 1;
		sobel.at<float>(2, 1) = 2;
		sobel.at<float>(2, 2) = 1;
	}

	Mat dst(src.size(), src.type());
	filter2D(src, dst, src.depth(), sobel);
	return dst;
}