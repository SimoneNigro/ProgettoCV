#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <string>

#define TOLLERANCE 7

using namespace cv;
using namespace std;

vector<Point> contoursConvexHull( vector<vector<Point> > contours )
{
    vector<Point> result;
    vector<Point> pts;

    for ( size_t i = 0; i< contours.size(); i++)
        for ( size_t j = 0; j< contours[i].size(); j++)
            pts.push_back(contours[i][j]);

    convexHull( pts, result );

    return result;
}


int main(int argc, char** argv)
{
	Mat img;

	if( argc != 2)
    {
        printf("Usage: ./progetto nome_file\n");
	    return -1;
    }

	// Opening the image
	img=imread(argv[1], CV_LOAD_IMAGE_COLOR);

	//////////////////////////////////////////// Detecting the outer circle ////////////////////////////////////////////

	// We need to convert it from bgr to hsv 
	Mat hsv;
	cvtColor(img,hsv,CV_BGR2HSV);

	// Threshold the HSV image, keep only the red pixels
	Mat lower_red_hue_range;
	Mat upper_red_hue_range;
	inRange(hsv, Scalar(1, 120,120), Scalar(10, 255, 255), lower_red_hue_range);
	inRange(hsv, Scalar(150, 100,100), Scalar(180, 255, 255), upper_red_hue_range);

	// Combine the above two images
	Mat red_hue_image;
	addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);

	// We want to fill the area contained by the larger circle
 	vector<vector<Point> > contours;
	findContours( red_hue_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
	Mat drawing = Mat::zeros(red_hue_image.size(), CV_8UC1);
    vector<Point> ConvexHullPoints =  contoursConvexHull(contours);
	vector<vector<Point> > vecPoints(1);
	vecPoints[0]=ConvexHullPoints;
	fillPoly(drawing, vecPoints, Scalar(255)); 

    // We blur it
    GaussianBlur(drawing, drawing, Size(9, 9), 2, 2);

	// Detecting the circle using Hough Transform
	vector<Vec3f> outer_circles;
	HoughCircles(drawing, outer_circles, CV_HOUGH_GRADIENT, 2, drawing.rows/8, 200, 15, 220, 230); 

	//////////////////////////////////////////// Detecting the inner circle ////////////////////////////////////////////

	// We need to feed HoughCircles a greyscale image
	Mat grey;
    cvtColor(img, grey, COLOR_RGB2GRAY);

	// We blur it
	GaussianBlur(grey, grey, Size(9, 9), 2, 2);

	// Detecting the circle using Hough Transform
	vector<Vec3f> inner_circles;
	HoughCircles(grey, inner_circles, CV_HOUGH_GRADIENT, 1, grey.rows/8, 100, 20, 185, 190);

	//////////////////////////////////// Drawing and analyzing the detected circles ////////////////////////////////////

	// Finding outer circle's center and radius
	if(outer_circles.size() == 0){
		printf("!!!NO CIRCLES DETECTED!!!\n");
		exit(-1);
	}

	Point outer_center(outer_circles[0][0], outer_circles[0][1]);
	int outer_radius = outer_circles[0][2];

	circle(img, outer_center, 3, Scalar(0, 255, 0), -1);
	circle(img, outer_center, outer_radius, Scalar(0, 255, 0), 1);

	float distance = 0;

	// Finding inner circle's center and radius
	if(inner_circles.size() == 0){
		printf("!!!NO CIRCLES DETECTED!!!\n");
		exit(-1);
	}

	Point inner_center(inner_circles[0][0], inner_circles[0][1]);
	int inner_radius = inner_circles[0][2];

	circle(img, inner_center, 3, Scalar(255, 0, 0), -1);
	circle(img, inner_center, inner_radius, Scalar(255, 0, 0), 1);
	
	// Calculating the distance between the centers
    distance = norm(outer_center - inner_center);

    if(distance < TOLLERANCE)
        printf("distance = %f \nIT'S CENTERED!!!!\n", distance);
    else
        printf("distance = %f \nIT'S NOT CENTERED :((\n", distance);

	//////////////////////////////////////////////// Saving the results ////////////////////////////////////////////////

	string full_path(argv[1]);

	size_t indexp = full_path.find_last_of(".");
	size_t indexs = full_path.find_last_of("/");

	string path = full_path.substr(0, indexs);
	string rawname = full_path.substr(indexs, 5);

    stringstream ss;    

	ss << path;

	if(distance < TOLLERANCE)
		ss  << rawname << "_OK.bmp";
	else
		ss  << rawname << "_NO.bmp";

	string s = ss.str();

	imwrite(s,img);

/*
    //We draw the white filled circle onto the original image for debug purposes 
    stringstream ss2;

	ss2 << path;

	if(distance < TOLLERANCE)
		ss2  << rawname << "_OK_bianco.bmp";
	else
		ss2  << rawname << "_NO_bianco.bmp";

	string s2 = ss2.str();

    Mat dst;
	cvtColor(drawing, drawing, COLOR_GRAY2BGR);
	addWeighted( img, 0.5, drawing, 0.5, 0.0, dst);

	imwrite(s2,dst);
*/

	return 0;
}
