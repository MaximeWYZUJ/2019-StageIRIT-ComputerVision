#pragma once
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "Point2.hpp"
#include "Cluster.hpp"
#include <iostream>
#include "fonctionsAnnexes.h"


using namespace cv;
using namespace std;

enum DIRECTION { Horizontal, Vertical, Fixe };

double correlationClassique(Mat im1, Mat im2, bool normalize = true, double im2Norm = -1);
double correlationClassiqueCarre(Mat im1, Mat im2, bool normalize = true, double im2Norm = -1);
double correlationCroisee(Mat im1, Mat im2);
double fastCorrelationCroisee(Mat imFixeCentree, double moyFixe, double normeFixe, Mat imGlissante, double & moyPreviousGlissante, DIRECTION d);
double correlationCENSUS(Mat im1, Mat im2);
double correlationSMPD(Mat im1, Mat im2, int p = 2, int h = 3);
double correlationGeometrique(vector<KeyPoint> kpPattern, Cluster c, vector<vector<DMatch>> matches);
double correlationGeometriqueNoMatch(vector<KeyPoint> kpPattern, Cluster c, double scale = 4.0 / 3.0);
double correlationHistogramme(Mat im1, Mat im2);
double correlationHistogramme(vector<int> histPattern, Mat image);