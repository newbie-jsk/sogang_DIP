#include "utils.h"

Mat gammaTransform(Mat src, float gamma)
{
	///Variables Declaration & Initialization///
	Mat dst(src.rows, src.cols, src.type());
	uchar* dst_data = dst.data;
	float* hist;
	
	hist = getHist(src);

	/// Mapping dst <- gamma(src)
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			dst.data[i * src.cols + j] = gamma_function(src, i, j, gamma);
		}
	}
	return dst;
}

float* getHist(Mat src)
{
	///Variables Declaration///
	float histogram[256] = { 0 };
	uchar* data;
	uchar intensity_level;
	float one_div_MN;

	data = src.data;							//Img data type
	one_div_MN = 1.0 / (src.cols * src.rows);	// 1/Img size

	///Cummulate intensity data by inspecting all pixels 
	/// -> histogram
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			intensity_level = data[i * src.cols + j];
			histogram[intensity_level] += one_div_MN;
		}
	}

	return histogram;
}

Mat histEqualization(Mat src)
{
	///Variables Declaration & Initialization///
	Mat dst(src.rows, src.cols, src.type());
	float* hist;
	float accum_hist[256] = { 0 };
	uchar* dst_data = dst.data;
	uchar* src_data = src.data;
	
	hist = getHist(src);

	accum_hist[0] = hist[0];
	for (int i = 1; i < 256; i++)
	{
		accum_hist[i] = accum_hist[i - 1] + hist[i];
	}
	
	// transform
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			int location = i * src.cols + j;
			// dst <- src mapping using histogram.
			// dst = accum_hist( src )
			dst_data[location] = (uchar)round((255.0 * accum_hist[src_data[location]]));
		}
	}

	return dst;
}

Mat boxFilter(Size size)
{
	Mat dst(size, CV_8U, Scalar(1));
	return dst;
}

Mat imFilter(Mat src, Mat kernel, string pad_type)
{
	Mat dst(src.rows, src.cols, src.type());
	uchar* src_data, * dst_data;
	int location;

	src_data = src.data;
	dst_data = dst.data;

	if (pad_type == "zero")
	{
		for (int i = 0; i < src.rows; i++)
		{
			for (int j = 0; j < src.cols; j++)
			{
				location = i * src.cols + j;

				dst_data[location] = filtering_zero(src, kernel, i, j);
			}
		}
	} 
	else if (pad_type == "mirror")
	{
		for (int i = 0; i < src.rows; i++)
		{
			for (int j = 0; j < src.cols; j++)
			{
				location = i * src.cols + j;

				dst_data[location] = filtering_mirr(src, kernel, i, j);
			}
		}
	}
	else
	{
		printf("Wrong padding type. Please input \"mirror\" or \"zero\"");
		return src;
	}
	return dst;
}