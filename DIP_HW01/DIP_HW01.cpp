#include "utils.h"
Mat size_transform(Mat src, Mat affine, float* center);

Mat warpAffine(Mat src, Mat affmat, string interp_type)
{
	Mat dst(src.rows, src.cols, src.type(), Scalar(0));

	int src_locX, src_locY;
	float src_float_locX, src_float_locY;
	Mat inv_affmat = affmat.inv();
	float* affmat_data = (float*)inv_affmat.data;

	if (interp_type == "nearest")
	{
		for (int i = 0; i < dst.rows; i++)
		{
			for (int j = 0; j < dst.cols; j++)
			{
				src_locX = round(affmat_data[0] * i + affmat_data[1] * j + affmat_data[2]);
				src_locY = round(affmat_data[3] * i + affmat_data[4] * j + affmat_data[5]);

				if (src_locX >= src.rows || src_locY >= src.cols)
					dst.at<uchar>(i, j) = 0;
				else if (src_locX < 0 || src_locY < 0)
					dst.at<uchar>(i, j) = 0;
				else
					dst.at<uchar>(i, j) = src.at<uchar>(src_locX, src_locY);
			}
		}
	}
	else if (interp_type == "bilinear")
	{
		int loc_up_x, loc_down_x, loc_up_y, loc_down_y;
		float intensity, dx, dy;

		for (int i = 0; i < dst.rows; i++)
		{
			for (int j = 0; j < dst.cols; j++)
			{
				src_float_locX = (affmat_data[0] * i + affmat_data[1] * j + affmat_data[2]);
				src_float_locY = (affmat_data[3] * i + affmat_data[4] * j + affmat_data[5]);

				src_locX = (int)ceil(src_float_locX);
				src_locY = (int)ceil(src_float_locY);

				if (src_locX >= src.rows || src_locY >= src.cols)
					dst.at<uchar>(i, j) = 0;
				else if (src_locX < 0 || src_locY < 0)
					dst.at<uchar>(i, j) = 0;
				else
				{
					if (src_locX == 0 || src_locY == 0)
					{
						dst.at<uchar>(i, j) = src.at<uchar>(src_locX, src_locY);
					}
					else if (src_locX >= src.rows || src_locY >= src.cols)
					{
						dst.at<uchar>(i, j) = src.at<uchar>(src_locX, src_locY);
					}
					else
					{
						loc_up_x = ceil(src_float_locX); loc_down_x = floor(src_float_locX), dst.rows;
						loc_up_y = ceil(src_float_locY); loc_down_y = floor(src_float_locY), dst.cols;

						dx = loc_up_x - src_float_locX;
						dy = loc_up_y - src_float_locY;

						intensity = src.at<uchar>(loc_down_x, loc_up_y) * dx * (1 - dy);
						intensity += src.at<uchar>(loc_down_x, loc_down_y) * dx * dy;
						intensity += src.at<uchar>(loc_up_x, loc_down_y) * (1 - dx) * dy;
						intensity += src.at<uchar>(loc_up_x, loc_up_y) * (1 - dx) * (1 - dy);

						dst.at<uchar>(i, j) = (uchar)round(intensity);
					}
				}
			}
		}
	}
	return dst;
}

Mat rotate(Mat src, float radian, string interp_type)
{
	Mat affmat = Mat::zeros(Size(3, 3), CV_32F);
	float* affmat_data = (float*)affmat.data;
	affmat_data[0] = cos(-1 * radian);	affmat_data[1] = -1 * sin(-1 * radian);	affmat_data[2] = 0.0;
	affmat_data[3] = sin(-1 * radian);	affmat_data[4] = cos(-1 * radian);	affmat_data[5] = 0.0;
	affmat_data[6] = 0.0;	affmat_data[7] = 0.0;	affmat_data[8] = 1.0;

	float center[2];
	Mat newMat = size_transform(src, affmat, center);

	int x_shift, y_shift;
	x_shift = (int)round(floor(newMat.rows / 2) - floor(center[0]));
	y_shift = (int)round(floor(newMat.cols / 2) - floor(center[1]));

	//Center shift
	affmat_data[2] = x_shift;
	affmat_data[5] = y_shift;

	Mat dst = warpAffine(newMat, affmat, interp_type);
	return dst;
}

Mat translation(Mat src, float x, float y, string interp_type)
{
	Mat affmat = Mat::zeros(Size(3, 3), CV_32F);
	float* affmat_data = (float*)affmat.data;
	affmat_data[0] = 1.0;	affmat_data[1] = 0.0;	affmat_data[2] = x;
	affmat_data[3] = 0.0;	affmat_data[4] = 1.0;	affmat_data[5] = y;
	affmat_data[6] = 0.0;	affmat_data[7] = 0.0;	affmat_data[8] = 1.0;

	float center[2];
	Mat newMat = size_transform(src, affmat, center);

	Mat dst = warpAffine(newMat, affmat, interp_type);
	return dst;
}

Mat size_transform(Mat src, Mat affine, float* center)
{
	int x_length = 0, y_length = 0;
	float point0[2], point1[2], point2[2], point3[2];
	float* affmat_data = (float*)affine.data;
	
	//4point vectors
	point0[0] = affmat_data[0] * 0 + affmat_data[1] * 0 + affmat_data[2];
	point0[1] = affmat_data[3] * 0 + affmat_data[4] * 0 + affmat_data[5];

	point1[0] = affmat_data[0] * src.rows + affmat_data[1] * 0 + affmat_data[2];
	point1[1] = affmat_data[3] * src.rows + affmat_data[4] * 0 + affmat_data[5];

	point2[0] = affmat_data[0] * 0 + affmat_data[1] * src.cols + affmat_data[2];
	point2[1] = affmat_data[3] * 0 + affmat_data[4] * src.cols + affmat_data[5];

	point3[0] = affmat_data[0] * src.rows + affmat_data[1] * src.cols + affmat_data[2];
	point3[1] = affmat_data[3] * src.rows + affmat_data[4] * src.cols + affmat_data[5];

	//center location calculation
	center[0] = affmat_data[0] * floor(src.rows / 2) + affmat_data[1] * floor(src.cols / 2) + affmat_data[2];
	center[1] = affmat_data[3] * floor(src.rows / 2) + affmat_data[4] * floor(src.cols / 2) + affmat_data[5];

	//Size calculate
	x_length = (int)round(max(abs(point0[0] - point3[0]), abs(point1[0] - point2[0])));
	y_length = (int)round(max(abs(point0[1] - point3[1]), abs(point1[1] - point2[1])));

	//creat new Mat dst
	Mat dst(x_length + affmat_data[2], y_length + affmat_data[5], src.type(), Scalar(0));

	//copy src to dst
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			dst.at<uchar>(i, j) = src.at<uchar>(i, j);
		}
	}
	return dst;
}