#include "utils.h"
#include <fstream>

float* getHist(Mat src);
void SNR_calc(Mat origin, Mat restored);

int main(int argc, char* argv[])
{
	/*
	// 구현 1 //
	Mat src1 = imread("../images/Pattern.tif", IMREAD_GRAYSCALE);
	if (src1.empty()) {
		cout << "Input image does not exist." << endl;
		exit(-1);
	}

	imshow("original", src1);
	getHist(src1);
	
	Mat noise = addSaltPepper(src1, 0.1, 0.0);
	imshow("Salt_noise_01", noise);
	imwrite("Salt_noise_01.tif", noise);

	getHist(noise);

	noise = addSaltPepper(src1, 0.5, 0.0);
	imshow("Salt_noise_05", noise);
	imwrite("Salt_noise_05.tif", noise);

	getHist(noise);

	noise = addSaltPepper(src1, 0.0, 0.1);
	imshow("Pepper_noise_01", noise);
	imwrite("Pepper_noise_01.tif", noise);

	getHist(noise);

	noise = addSaltPepper(src1, 0.0, 0.5);
	imshow("Pepper_noise_05", noise);
	imwrite("Pepper_noise_05.tif", noise);

	getHist(noise);

	noise = addSaltPepper(src1, 0.05, 0.05);
	imshow("Salt & Pepper_noise_005", noise);
	imwrite("Salt & Pepper_noise_005.tif", noise);

	getHist(noise);

	noise = addSaltPepper(src1, 0.25, 0.25);
	imshow("Salt & Pepper_noise_025", noise);
	imwrite("Salt & Pepper_noise_025.tif", noise);


	getHist(noise);
	
	// Ckt_board 폴더내에 있는 6개 파일을 바꿔가면서 해주세요. //
	*/
	Mat src2 = imread("../images/Ckt_board/Ckt_board_salt&pepper_0.3.tif", IMREAD_GRAYSCALE);
	if (src2.empty()) {
		cout << "Input image does not exist." << endl;
		exit(-1);
	}

	Mat dst_contra_1_5 = contraharmonic(src2, Size(3, 3), 1.5);
	Mat dst_contra_0 = contraharmonic(src2, Size(3, 3), 0.0);
	Mat dst_contra_minus_1_5 = contraharmonic(src2, Size(3, 3), -1.5);
	Mat dst_meidan_3 = median(src2, Size(3, 3));
	Mat dst_meidan_5 = median(src2, Size(5, 5));
	dst_meidan_5 = median(dst_meidan_5, Size(5, 5));
	dst_meidan_5 = median(dst_meidan_5, Size(5, 5));
	Mat dst_adamed_7 = adaptiveMedian(src2, Size(7, 7));
	Mat dst_adamed_11 = adaptiveMedian(src2, Size(11, 11));

	imshow("Contraharmonic mean 1.5", dst_contra_1_5);
	imshow("Contraharmonic mean 0", dst_contra_0);
	imshow("Contraharmonic mean -1.5", dst_contra_minus_1_5);
	imshow("Median 3", dst_meidan_3);
	imshow("Median 5", dst_meidan_5);
	imshow("Adaptive median 7", dst_adamed_7);
	imshow("Adaptive median 11", dst_adamed_11);

	imwrite("Contraharmonic mean 1.5.tif", dst_contra_1_5);
	imwrite("Contraharmonic mean 0.0.tif", dst_contra_0);
	imwrite("Contraharmonic mean -1.5.tif", dst_contra_minus_1_5);
	imwrite("Median 3.tif", dst_meidan_3);
	imwrite("Median 5.tif", dst_meidan_5);
	imwrite("Adaptive median 7.tif", dst_adamed_7);
	imwrite("Adaptive median 11.tif", dst_adamed_11);
	waitKey(0);
	/*
	
	Mat src3 = imread("../images/sogang.tif", IMREAD_GRAYSCALE);
	if (src3.empty()) {
		cout << "Input image does not exist." << endl;
		exit(-1);
	}

	imshow("org", src3);

	Mat add_turbulence_0025 = add_turbulence(src3, 0.0025);
	Mat add_turbulence_001 = add_turbulence(src3, 0.001);
	Mat add_turbulence_00025 = add_turbulence(src3, 0.00025);
//	imshow("Sogang_0025",add_turbulence_0025);
//	imwrite("Sogang_0025.tif", add_turbulence_0025);
//	imshow("Sogang_001", add_turbulence_001);
//	imwrite("Sogang_001.tif", add_turbulence_001);
//	imshow("Sogang_00025", add_turbulence_00025);
//	imwrite("Sogang_00025.tif", add_turbulence_00025);
	
	// K값을 계속 바꾸면서 하세요. //
	
	Mat Wiener_filter_image = wienerFilter(add_turbulence_0025, 0.00004, 0.0025);
	imshow("Wiener_filter_image_0025", Wiener_filter_image);
	imwrite("Wiener_Sogang_0025.tif", Wiener_filter_image);
	SNR_calc(src3, Wiener_filter_image);
	Wiener_filter_image = wienerFilter(add_turbulence_001, 0.00012, 0.001);
	imshow("Wiener_filter_image_001", Wiener_filter_image);
	imwrite("Wiener_Sogang_001.tif", Wiener_filter_image);
	SNR_calc(src3, Wiener_filter_image);
	Wiener_filter_image = wienerFilter(add_turbulence_00025, 0.0015, 0.00025);
	imshow("Wiener_filter_image_00025", Wiener_filter_image);
	imwrite("Wiener_Sogang_00025.tif", Wiener_filter_image);
	SNR_calc(src3, Wiener_filter_image);




	waitKey(0);
	*/
	
}

float* getHist(Mat src)
{
	///Variables Declaration///
	float histogram[256] = { 0 };
	uchar intensity_level;
	float one_div_MN;

	one_div_MN = 1.0 / (src.cols * src.rows);	// 1/Img size

	///Cummulate intensity data by inspecting all pixels 
	/// -> histogram
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			intensity_level = src.at<uchar>(i, j);
			histogram[intensity_level] += one_div_MN;
		}
	}
	ofstream fout1("o.txt");
	for (int i = 0; i < 256; i++)
	{
		fout1 << i;
		fout1 << " ";
		fout1 << histogram[i];
		fout1 << "\n";
	}
	fout1.close();
	return histogram;
}

void SNR_calc(Mat origin, Mat restored)
{
	float nominator = 0.0;
	float denominator = 0.0;
	for (int i = 0; i < origin.rows; i++)
	{
		for (int j = 0; j < origin.cols; j++)
		{
			nominator += (float)(restored.at<uchar>(i, j) * restored.at<uchar>(i, j));
			denominator += (float)(origin.at<uchar>(i, j) - restored.at<uchar>(i, j)) * (origin.at<uchar>(i, j) - restored.at<uchar>(i, j));
		}
	}

	printf("\n SNR : %f", (nominator / denominator));
}