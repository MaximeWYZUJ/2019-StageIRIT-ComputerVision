#pragma once
#include "stdafx.h"
#include "Point2.hpp"
#include "Cluster.hpp"
#include <iostream>
#include <numeric>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc.hpp>
#include "meanShift.hpp"
#include "mesureCorrelation.h"



using namespace cv;
using namespace std;


vector<int> sortIndexes(const vector<double> &v);
Point2f transformation(Point2f originePattern, Point2f origineCluster, double scale, Point2f pointPattern);
bool checkPointProche(Point2f point, vector<KeyPoint> allKp, int imprecision = 10, bool checkSelf = false);
bool checkPointProche(Point2f point, vector<Point2> allPt, int imprecision = 10, bool checkSelf = false);
vector<Point2f> getPointsSansDoublon(Cluster c);
vector<Point> getPointsSansDoublon(vector<Point> points, double rayon = 3.0);
double estimateScale(vector<pair<Point2f, Point2f>> matchingPoints);
double moyenneImage(Mat image);
double normeImage(Mat image);

// L'image doit etre de type CV_UC1 (type uchar)
vector<int> getHistogramme(Mat image);
void afficherHistogramme(const vector<int> & hist, string nomFenetre = "Histogramme");
vector<bool> filtrerParHistogramme(Mat pattern, vector<Mat> images, double seuilCoeff = 0.5);