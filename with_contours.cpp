#pragma comment(lib, "opencv_world331.lib")

#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
#include <map>
#include <set>
using namespace std;


// For later use with multiset
class double_point2d_pair
{
public:
	double area;
	Point2d centre;
};

// For later use with multiset
bool operator<(const double_point2d_pair &lhs, const double_point2d_pair &rhs)
{
	return lhs.area < rhs.area;
}


int main(void)
{
	Mat frame = imread("cards.png", CV_LOAD_IMAGE_GRAYSCALE);

	threshold(frame, frame, 127, 255, THRESH_BINARY);

	imshow("f", frame);

	Mat flt_frame(frame.rows, frame.cols, CV_32F);

	for (int j = 0; j < frame.rows; j++)
		for (int i = 0; i < frame.cols; i++)
			flt_frame.at<float>(j, i) = frame.at<unsigned char>(j, i) / 255.0f;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	if (contours.size() < 2)
	{
		cout << "Error: must have 2 or more contours." << endl;

		return 0;
	}

	/// Draw contours
	Mat output = Mat::zeros(flt_frame.size(), CV_8UC3);

	for (int i = 0; i < contours.size(); i++)
		drawContours(output, contours, i, Scalar(0, 127, 255), 1, 8, hierarchy, 0, Point());

	vector<double> areas(contours.size());

	for (int i = 0; i < contours.size(); i++)
		areas[i] = contourArea(contours[i]);

	vector<Point2d> mass_centres(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		const Moments mu = moments(contours[i], false);
		mass_centres[i] = Point2d(mu.m10 / mu.m00, mu.m01 / mu.m00);
	}

	multiset<double_point2d_pair> dpp_set;

	for (int i = 0; i < contours.size(); i++)
	{
		double_point2d_pair dpp;
		dpp.area = areas[i];
		dpp.centre = mass_centres[i];
		dpp_set.insert(dpp);
	}

	int count = 0;
	Point2d first_centre, second_centre;

	// Get the two largest contour areas
	for (multiset<double_point2d_pair>::const_reverse_iterator cri = dpp_set.rbegin(); cri != dpp_set.rend(); cri++)
	{
		if (count == 0)
		{
			first_centre = cri->centre;
			count++;
		}
		else
		{
			second_centre = cri->centre;
			break;
		}
	}

	Point2d difference = first_centre - second_centre;
	double distance = sqrt(difference.ddot(difference));

	cout << "Distance (in pixels): " << distance << endl;

	line(output, first_centre, second_centre, Scalar(255, 127, 0), 2);

	imshow("f2", output);

	waitKey();

	destroyAllWindows();

	return 0;
}
