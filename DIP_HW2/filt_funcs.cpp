#include "utils.h"

enum PTR_LOCATION {
	INSIDE = 0,
	OUTSIDE_LEFT = 1,
	OUTSIDE_RIGHT = 2,
	OUTSIDE_TOP = 3,
	OUTSIDE_DOWN = 4,
	OUTSIDE_TOP_LEFT = 5,
	OUTSIDE_TOP_RIGHT = 6,
	OUTSIDE_DOWN_LEFT = 7,
	OUTSIDE_DOWN_RIGHT = 8,
};

PTR_LOCATION find_location(int src_rows, int src_cols, int x, int y);


uchar filtering_zero(Mat src, Mat kern, int src_center_x, int src_center_y)
{
	int src_loc, kern_center_x, kern_center_y;
	int dx, dy;
	uchar* src_data, * kern_data;
	float output = 0;
	PTR_LOCATION location;

	src_data = src.data;
	kern_data = kern.data;
	kern_center_x = ceil(kern.rows / 2);
	kern_center_y = ceil(kern.cols / 2);

	for (int i = 0; i < kern.rows; i++)
	{
		dx = kern_center_x - i;

		for (int j = 0; j < kern.cols; j++)
		{
			dy = kern_center_y - j;
			src_loc = (src_center_x + dx) * src.rows + src_center_y + dy;

			location = find_location(src.rows, src.cols, src_center_x + dx, src_center_y + dy);

			switch (location)
			{
			case(INSIDE):
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			}

		}
	}

	return (uchar)round(output / (kern.cols * kern.rows));
}

uchar filtering_mirr(Mat src, Mat kern, int src_center_x, int src_center_y)
{
	int src_loc, kern_center_x, kern_center_y;
	int dx, dy;
	uchar* src_data, * kern_data;
	float output = 0;
	PTR_LOCATION location;

	src_data = src.data;
	kern_data = kern.data;
	kern_center_x = ceil(kern.rows / 2);
	kern_center_y = ceil(kern.cols / 2);

	for (int i = 0; i < kern.rows; i++)
	{
		dx = kern_center_x - i;

		for (int j = 0; j < kern.cols; j++)
		{
			dy = kern_center_y - j;

			location = find_location(src.rows, src.cols, src_center_x + dx, src_center_y + dy);

			switch (location)
			{
			case(INSIDE):
				src_loc = (src_center_x + dx) * src.rows + (src_center_y + dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_TOP):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y + dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_DOWN):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y + dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_RIGHT):
				src_loc = (src_center_x + dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_LEFT):
				src_loc = (src_center_x + dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_TOP_RIGHT):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_TOP_LEFT):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_DOWN_RIGHT):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			case(OUTSIDE_DOWN_LEFT):
				src_loc = (src_center_x - dx) * src.rows + (src_center_y - dy);
				output += kern_data[i * kern.cols + j] * src_data[src_loc];
				break;
			}
		}
	}

	return (uchar)round(output / (kern.cols * kern.rows));
}

PTR_LOCATION find_location(int src_rows, int src_cols, int x, int y)
{
	PTR_LOCATION location;

	if (x < 0)
	{
		if (y < 0)
			location = OUTSIDE_TOP_LEFT;
		else if (y > src_cols)
			location = OUTSIDE_TOP_RIGHT;
		else
			location = OUTSIDE_TOP;
	}
	else if (y < 0)
	{
		if (x > src_rows)
			location = OUTSIDE_DOWN_LEFT;
		else
			location = OUTSIDE_LEFT;
	}
	else if (x >= src_rows)
	{
		if (y >= src_cols)
			location = OUTSIDE_DOWN;
		else
			location = OUTSIDE_DOWN_RIGHT;
	}
	else if (y >= src_cols)
		location = OUTSIDE_RIGHT;
	else
		location = INSIDE;

	return location;
}