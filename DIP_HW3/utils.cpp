#include "utils.h"

void swapPhase(Mat src1, Mat src2, Mat& dst1, Mat& dst2)
{
	Mat src1_fft = fftshift2d(fft2d(src1));
	Mat src2_fft = fftshift2d(fft2d(src2));

//	fft_printer(src1_fft, 1);
//	fft_printer(src2_fft, 1);

	Mat planes[2];
	split(src1_fft, planes);
	Mat src1_mag, src1_phase;
	magnitude(planes[0], planes[1], src1_mag);
	phase(planes[0], planes[1], src1_phase);

	split(src2_fft, planes);
	Mat src2_mag, src2_phase;
	magnitude(planes[0], planes[1], src2_mag);
	phase(planes[0], planes[1], src2_phase);

	Mat dst1_re(src1.size(), CV_32F);
	Mat dst1_im(src1.size(), CV_32F);
	Mat dst2_re(src2.size(), CV_32F);
	Mat dst2_im(src2.size(), CV_32F);

	for (int i = 0; i < src1.rows; i++)
	{
		for (int j = 0; j < src1.cols; j++)
		{
			dst1_re.at<float>(i, j) = src1_mag.at<float>(i, j) * cos(src2_phase.at<float>(i, j));
			dst1_im.at<float>(i, j) = src1_mag.at<float>(i, j) * sin(src2_phase.at<float>(i, j));

			dst2_re.at<float>(i, j) = src2_mag.at<float>(i, j) * cos(src1_phase.at<float>(i, j));
			dst2_im.at<float>(i, j) = src2_mag.at<float>(i, j) * sin(src1_phase.at<float>(i, j));
		}
	}

	Mat dst1_fft, dst2_fft;

	Mat channels1[2] = { dst1_re, dst1_im };
	merge(channels1, 2, dst1_fft);
	Mat channels2[2] = { dst2_re, dst2_im };
	merge(channels2, 2, dst2_fft);


	dst1 = ifft2d(fftshift2d(dst1_fft));
	dst2 = ifft2d(fftshift2d(dst2_fft));

//	fft_printer(dst1_fft, 1);
//	fft_printer(dst2_fft, 1);
}

Mat freqFilter(Mat src, Mat kernel)
{
	// Do Fast Fourier Transform
	Mat kern_fft = fftshift2d(fft2d(kernel));
	Mat src_fft = fftshift2d(fft2d(src));

//	fft_printer(kern_fft, 0);
//	fft_printer(src_fft, 0);

	// Source * Kernel
	Mat dst_fft = mat_mul(src_fft, kern_fft);

//	fft_printer(dst_fft, 0);

	// Inverse FFT
	Mat dst_ifft = ifft2d(fftshift2d(dst_fft));
	Mat dst;
	dst_ifft.convertTo(dst, CV_8U, 1, 0);

	return dst;
}

Mat freqBoxFilter(Mat src, Size kernel_size)
{
	//Make box kernel
	Mat kernel(kernel_size, src.type(), Scalar(1));

	//Padding two images
	Mat padded_src = img_padding(src, Size(1000,1000));
	Mat padded_kern = img_padding(kernel, Size(1000,1000));

	// box kernel * 1/kernel_size
	padded_kern.convertTo(padded_kern, CV_32F);
	float div = 1.0 / (kernel_size.width * kernel_size.height);
	padded_kern = padded_kern.mul(div);
	
	//filtering
	Mat dst_prem = freqFilter(padded_src, padded_kern);

	//Cut image to original size
	Mat dst(src.size(), src.type());
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			dst.at<uchar>(i, j) = dst_prem.at<uchar>(i, j);
		}
	}

	return dst;
}

Mat freqGaussFilter(Mat src, Size kernel_size)
{
	//Declare variables
	Mat kernel(kernel_size, CV_32F, Scalar(0));
	float kernel_pixel_value, kernel_cummulative_value = 0;
	int row_center, col_center;

	//Make Gauss filter in spartial domain
	row_center = ceil(kernel_size.height / 2);
	col_center = ceil(kernel_size.width / 2);
	int sigma;
	if (kernel_size.width == 31)
		sigma = 5;
	else
		sigma = 9;

	for (int i = 0; i < kernel_size.height; i++)
	{
		float x_sqr = (i - row_center) * (i - row_center);
		for (int j = 0; j < kernel_size.width; j++)
		{
			float y_sqr = (j - col_center) * (j - col_center);

			// exp( - (x^2 + y^2) / 2 sig^2)
			kernel_pixel_value = exp(-1.0 * (x_sqr + y_sqr) / (2 * sigma * sigma));
			kernel.at<float>(i, j) = kernel_pixel_value;
			kernel_cummulative_value += kernel_pixel_value;
		}
	}

	//divide kernel by cummulated value
	kernel = kernel.mul(1.0 / kernel_cummulative_value);
	Mat padded_kern = img_padding(kernel, Size(1000, 1000));
	Mat padded_src = img_padding(src, Size(1000, 1000));

	fft_printer(fftshift2d(fft2d(padded_kern)), 0);

	//filtering
	Mat dst_prem = freqFilter(padded_src, padded_kern);

	Mat dst(src.size(), src.type());
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			dst.at<uchar>(i, j) = dst_prem.at<uchar>(i, j);
		}
	}

	return dst;
}

Mat NotchFilter(Mat src)
{
	Mat src_fft = fftshift2d(fft2d(src));
//	fft_printer(src_fft, 1);

	Mat notch_filter(src.size(), CV_32FC2, Scalar(1, 1));
	int row_cent = ceil((src.rows) / 2);
	int col_cent = ceil((src.cols) / 2);
	int diff = 32, depth = 3;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			//center notch
			if (j > col_cent - 1)
			{
				if (j < col_cent + 1)
				{
					if (i < row_cent - 50)
					{
						notch_filter.at<Vec2f>(i, j)[0] = 0;
						notch_filter.at<Vec2f>(i, j)[1] = 0;
					}
					else if (i > row_cent + 50)
					{
						notch_filter.at<Vec2f>(i, j)[0] = 0;
						notch_filter.at<Vec2f>(i, j)[1] = 0;
					}
				}
			}

			//left notch
			if (j > (col_cent - diff - depth))
			{
				if (j < (col_cent - diff + depth))
				{
					notch_filter.at<Vec2f>(i, j)[0] = 0;
					notch_filter.at<Vec2f>(i, j)[1] = 0;
				}
			}

			//right notch
			if (j < (col_cent + diff + depth))
			{
				if (j > (col_cent + diff - depth))
				{
					notch_filter.at<Vec2f>(i, j)[0] = 0;
					notch_filter.at<Vec2f>(i, j)[1] = 0;
				}
			}
		}
	}

//	fft_printer(notch_filter, 0);

	Mat dst_fft = mat_mul(src_fft, notch_filter);

//	fft_printer(dst_fft, 1);

	Mat dst_ifft = ifft2d(fftshift2d(dst_fft));
	Mat dst;
	dst_ifft.convertTo(dst, CV_8U, 1, 0);

	return dst;
}

Mat img_padding(Mat src, Size size)
{
	Mat dst(size, src.type(), Scalar(0));

	if (src.type() == CV_8U)
	{
		for (int j = 0; j < src.rows; j++)
		{
			for (int i = 0; i < src.cols; i++)
			{
				dst.at<uchar>(j, i) = src.at<uchar>(j, i);
			}
		}
	}
	else if (src.type() == CV_32F)
	{
		for (int j = 0; j < src.rows; j++)
		{
			for (int i = 0; i < src.cols; i++)
			{
				dst.at<float>(j, i) = src.at<float>(j, i);
			}
		}
	}
	

	return dst;
}

Mat mat_mul(Mat src1_fft, Mat src2_fft)
{
	Mat dst(src1_fft.size(), CV_32FC2);

	// Real part calc
	for (int i = 0; i < src1_fft.rows; i++)
	{
		for (int j = 0; j < src1_fft.cols; j++)
		{
			dst.at<Vec2f>(i, j)[0] = src1_fft.at<Vec2f>(i, j)[0] * src2_fft.at<Vec2f>(i, j)[0];
			dst.at<Vec2f>(i, j)[0] += src1_fft.at<Vec2f>(i, j)[1] * src2_fft.at<Vec2f>(i, j)[1];
		}
	}

	//Img part calc
	for (int i = 0; i < src1_fft.rows; i++)
	{
		for (int j = 0; j < src1_fft.cols; j++)
		{
			dst.at<Vec2f>(i, j)[1] = src1_fft.at<Vec2f>(i, j)[0] * src2_fft.at<Vec2f>(i, j)[1];
			dst.at<Vec2f>(i, j)[1] += src1_fft.at<Vec2f>(i, j)[1] * src2_fft.at<Vec2f>(i, j)[0];
		}
	}

	return dst;
}

//FFT spectrum / phase image show & save. 
void fft_printer(Mat src_fft, int norm)
{
	//split src_fft's channels into two planes
	Mat planes[2];
	split(src_fft, planes);
	Mat src_fft_mag, src_fft_phase;
	magnitude(planes[0], planes[1], src_fft_mag);
	phase(planes[0], planes[1], src_fft_phase);

	// Find min, max & normalize output
	if (norm == 1)
	{
		float minmax[2];
		min_max_finder(src_fft_mag, minmax);
		int max = minmax[1];
		int min = minmax[0];
		src_fft_mag.convertTo(src_fft_mag, CV_32F, 255.0 / (max - min), 255.0 / (max - min) * min);
	}

	//Log operation
	src_fft_mag += Scalar::all(1);
	log(src_fft_mag, src_fft_mag);

	src_fft_phase += Scalar::all(1);
	log(src_fft_phase, src_fft_phase);

	//Image showing
	imshow("fft_mag", src_fft_mag);
	imwrite("fft_mag.tif", src_fft_mag);
	imshow("fft_phase", src_fft_phase);
	imwrite("fft_phase.tif", src_fft_phase);
	waitKey(0);
}

void min_max_finder(Mat src, float minmax[])
{
	Size size = src.size();
	float min = 0, max = 0, pix;

	for (int i = 0; i < size.height; i++)
	{
		for (int j = 0; j < size.width; j++)
		{
			pix = src.at<float>(i, j);

			if (pix > max)
				max = pix;
			if (pix < min)
				min = pix;
		}
	}

	minmax[0] = min;
	minmax[1] = max;
}