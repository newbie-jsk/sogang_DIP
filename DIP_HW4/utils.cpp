#include "utils.h"
#include "random"
#include <cmath>
#include <algorithm>

void fft_printer(Mat src_fft, int norm);
void min_max_finder(Mat src, float minmax[]);
Mat mat_mul(Mat src1_fft, Mat src2_fft);


Mat addSaltPepper(Mat src, float ps, float pp, float salt_val, float pepp_val)
{
	Mat dst;
	src.copyTo(dst);

	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> distr(0.0, 1.0);
	// Random number between 0 - 1


	for (int i = 0; i < src.rows ; i++)
	{
		for (int j = 0; j < src.cols ; j++)
		{
			if ((ps >= distr(gen)) && (ps != 0.0))
				dst.at<uchar>(i, j) = salt_val;
			if ((pp >= distr(gen)) && (pp != 0.0))
				dst.at<uchar>(i, j) = pepp_val;
		}
	}
	

	return dst;
}

Mat contraharmonic(Mat src, Size kernel_size, float Q)
{
	Mat dst(src.size(), src.type(), Scalar(0));
	int kernel_width_center = (int)floor(kernel_size.width / 2.0);
	int kernel_height_center = (int)floor(kernel_size.height / 2.0);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			float nominator = 0.0;
			float denominator = 0.0;

			// sigma r,c ( contra harmonic function(image) )
			for (int r = -kernel_height_center; r <= kernel_height_center; r++)
			{
				for (int c = -kernel_width_center; c <= kernel_width_center; c++)
				{
					//Zero padding
					if ((i + r >= 0) && (i + r < src.rows))
					{
						if ((j + c >= 0) && (j + c < src.cols))
						{
							int pix_value = (int)src.at<uchar>(i + r, j + c);
							nominator += pow(pix_value, Q + 1);
							denominator += pow(pix_value, Q);
						}
					}
				}
			}
			dst.at<char>(i, j) = (uchar)round(nominator / denominator);
		}
	}

	return dst;
}  

Mat median(Mat src, Size kernel_size)
{
	Mat dst(src.size(), src.type(), Scalar(0));
	int kernel_width_center = (int)floor(kernel_size.width / 2.0);
	int kernel_height_center = (int)floor(kernel_size.height / 2.0);
	int size_kernel = kernel_size.height * kernel_size.width;
	int median = (int)ceil(size_kernel / 2.0);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			uchar* pix_list = new uchar[size_kernel] { 0 };
			int k = 0;

			//zero padding
			for (int r = -kernel_height_center; r <= kernel_height_center; r++)
			{
				for (int c = -kernel_width_center; c <= kernel_width_center; c++)
				{
					if ((i + r >= 0) && (i + r < src.rows))
					{
						if ((j + c >= 0) && (j + c < src.cols))
						{
							pix_list[k] = src.at<uchar>(i + r, j + c);
							k++;
						}
					}
				}
			}
			sort(pix_list, pix_list + size_kernel); // sorting pixel value list.
			dst.at<char>(i, j) = pix_list[median];  // select median value.
			delete[] pix_list;
		}
	}
	
	return dst;
}

Mat adaptiveMedian(Mat src, Size kernel_size)
{
	Mat dst(src.size(), src.type(), Scalar(0));

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			int height = 3, width = 3; //initial kernel size
			while (kernel_size.width >= width || kernel_size.height >= height) //Iterate kernel size
			{
				int size_kernel = height * width;
				int median = (int)ceil(size_kernel / 2.0);
				int kernel_width_center = (int)floor(width / 2.0);
				int kernel_height_center = (int)floor(height / 2.0);

				uchar* pix_list = new uchar[size_kernel]{ 0 };
				int k = 0;

				//zero padding
				for (int r = -kernel_height_center; r <= kernel_height_center; r++)
				{
					for (int c = -kernel_width_center; c <= kernel_width_center; c++)
					{
						if ((i + r >= 0) && (i + r < src.rows))
						{
							if ((j + c >= 0) && (j + c < src.cols))
							{
								pix_list[k] = src.at<uchar>(i + r, j + c);
								k++;
							}
						}
					}
				}
				sort(pix_list, pix_list + size_kernel); // sorting pixel value list.

				if ((pix_list[0] < pix_list[median]) && (pix_list[median] < pix_list[size_kernel])) // Level A
				{
					if ((pix_list[0] < src.at<uchar>(i, j)) && (src.at<uchar>(i, j) < pix_list[size_kernel])) // Level B
					{
						dst.at<char>(i, j) = src.at<uchar>(i, j);
						break;
					}
					else
					{
						dst.at<char>(i, j) = pix_list[median];
						break;
					}
				}
				else
				{
					width += 2;
					height += 2;
				}

				if (kernel_size.width < width || kernel_size.height < height)
				{
					dst.at<char>(i, j) = pix_list[median];
				}

				delete[] pix_list;
			}
		}
	}

	return dst;
}

Mat add_turbulence(Mat src, float k)
{
	Mat dst(src.size(), src.type());
	Mat turbulence(src.size(), CV_32F);
	Mat fft_src = fftshift2d(fft2d(src));

	int row_cent = (int)ceil(src.rows / 2.0);
	int col_cent = (int)ceil(src.cols / 2.0);
	for (int u = 0; u < turbulence.rows; u++)
	{
		for (int v = 0; v < turbulence.cols; v++)
		{
			int u_offset = (u - row_cent), v_offset = (v - col_cent);
			float turb = exp(-k * pow(u_offset * u_offset + v_offset * v_offset, 5.0 / 6.0));
			fft_src.at<Vec2f>(u, v)[0] *= turb;
			fft_src.at<Vec2f>(u, v)[1] *= turb;
		}
	}

	dst = ifft2d(fftshift2d(fft_src));
	dst.convertTo(dst, CV_8U, 1, 0);

	return dst;
}

Mat wienerFilter(Mat src, float K, float H)
{
	Mat dst(src.size(), src.type());
	Mat turbulence(src.size(), CV_32F);
	Mat fft_src = fftshift2d(fft2d(src));

	int row_cent = (int)ceil(src.rows / 2.0);
	int col_cent = (int)ceil(src.cols / 2.0);
	for (int u = 0; u < turbulence.rows; u++)
	{
		for (int v = 0; v < turbulence.cols; v++)
		{
			int u_offset = (u - row_cent), v_offset = (v - col_cent);
			float H_val = exp(-H * pow(u_offset * u_offset + v_offset * v_offset, 5.0 / 6.0));
			float deblur = H_val / (H_val * H_val + K);

			fft_src.at<Vec2f>(u, v)[0] *= deblur;
			fft_src.at<Vec2f>(u, v)[1] *= deblur;
		}
	}

	dst = ifft2d(fftshift2d(fft_src));

	dst.convertTo(dst, CV_8U, 1, 0);

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


Mat mat_mul(Mat src1_fft, Mat degradation)
{
	Mat dst(src1_fft.size(), CV_32FC2);

	// Real part calc
	for (int i = 0; i < src1_fft.rows; i++)
	{
		for (int j = 0; j < src1_fft.cols; j++)
		{
			dst.at<Vec2f>(i, j)[0] = src1_fft.at<Vec2f>(i, j)[0] * degradation.at<float>(i, j);
		}
	}

	//Img part calc
	for (int i = 0; i < src1_fft.rows; i++)
	{
		for (int j = 0; j < src1_fft.cols; j++)
		{
			dst.at<Vec2f>(i, j)[1] = src1_fft.at<Vec2f>(i, j)[1] * degradation.at<float>(i, j);
		}
	}

	return dst;
}

