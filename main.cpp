#pragma comment(lib, "opencv_world331.lib")

#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
#include <map>
#include <set>
using namespace std;


// For later use with multiset
class float_int_pair
{
public:
	float a;
	int b;
};

// For later use with multiset
bool operator<(const float_int_pair &lhs, const float_int_pair &rhs)
{
	return lhs.b < rhs.b;
}


int main(void)
{
	Mat frame = imread("cards.png", CV_LOAD_IMAGE_GRAYSCALE);

	threshold(frame, frame, 127, 255, THRESH_BINARY);

	imshow("f", frame);

	Mat flt_frame(frame.rows, frame.cols, CV_32F);

	for (int j = 0; j < frame.rows; j++)
		for (int i = 0; i < frame.cols; i++)
			flt_frame.at<float>(j, i) = frame.at<unsigned char>(j, i);

	// Find white sections in flt_frame, fill using floodFill.
	int section_count = 0;
	int fill_colour = 256;

	for (int j = 0; j < flt_frame.rows; j++)
	{
		for (int i = 0; i < flt_frame.cols; i++)
		{
			float colour = flt_frame.at<float>(j, i);

			if (colour == 255)
			{
				floodFill(flt_frame, Point(i, j), Scalar(fill_colour));

				section_count++;
				fill_colour++;
			}
		}
	}

	// Get number of pixels per section colour
	map<float, int> section_sizes;

	for (int j = 0; j < flt_frame.rows; j++)
	{
		for (int i = 0; i < flt_frame.cols; i++)
		{
			float colour = flt_frame.at<float>(j, i);

			if (colour != 0)
				section_sizes[colour]++;
		}
	}

	if (section_sizes.size() < 2)
	{
		cout << "Error: section count is not 2 or higher." << endl;
		return 0;
	}

	// Sort the section sizes in ascending order
	multiset<float_int_pair> sorted_section_sizes;

	for (map<float, int>::const_iterator ci = section_sizes.begin(); ci != section_sizes.end(); ci++)
	{
		float_int_pair ip;
		ip.a = ci->first;
		ip.b = ci->second;

		sorted_section_sizes.insert(ip);
	}

	Mat output(flt_frame.rows, flt_frame.cols, CV_8UC3);

	// Get the largest section size
	int largest_section_size = sorted_section_sizes.rbegin()->b;

	// Keep track of all the depth measurements per section.
	int centres_index = 0;
	vector< vector< Point > > centres;
	centres.resize(sorted_section_sizes.size());

	// For each section in flt_frame
	for (multiset<float_int_pair>::const_iterator ci = sorted_section_sizes.begin(); ci != sorted_section_sizes.end(); ci++)
	{
		// Iterate throughout the whole image
		for (int j = 0; j < flt_frame.rows; j++)
		{
			for (int i = 0; i < flt_frame.cols; i++)
			{
				float colour = flt_frame.at<float>(j, i);

				if (colour == ci->a)
				{
					// Mark each section in red
					output.at<Vec3b>(j, i)[0] = 0;
					output.at<Vec3b>(j, i)[1] = 0;
					output.at<Vec3b>(j, i)[2] = 255;

					Point p;
					p.x = i;
					p.y = j;

					centres[centres_index].push_back(p);
				}
			}
		}

		centres_index++;
	}

	// Get average centre per section.
	vector<Point> avg_centres;
	avg_centres.resize(centres.size());

	// Consider only the two largest sections
	for (int i = avg_centres.size() - 2; i < avg_centres.size(); i++)
	{
		// Get sum
		for (int j = 0; j < centres[i].size(); j++)
			avg_centres[i] += centres[i][j];

		// Get average
		avg_centres[i] /= static_cast<float>(centres[i].size());

		circle(output, avg_centres[i], 4, Scalar(255, 255, 255), FILLED, LINE_8);
	}

	imshow("output", output);

	waitKey();

	destroyAllWindows();

	return 0;
}
