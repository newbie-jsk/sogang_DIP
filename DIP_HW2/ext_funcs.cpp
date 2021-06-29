#include "utils.h"
#include "fstream"

uchar gamma_function(Mat src, int target_x, int target_y, float gamma)
{
	float normalized_data, out_data;
	uchar* data = src.data;

	normalized_data = (float)data[target_x * src.cols + target_y] / 255.0;
	out_data = 255.0 * pow(normalized_data, gamma);


	return (uchar)round(out_data);
}
