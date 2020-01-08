#pragma once
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc.hpp>
#include "meanShift.hpp"
#include "Point2.hpp"
#include "Cluster.hpp"
#include <iostream>


using namespace cv;
using namespace std;


void getKeypointsSift(string chemin, vector<KeyPoint> & kp, int nbOctaves = 6, double contrastTresh = 0.2, double edgeTresh = 2, double sigma = 1);
void getKeypointsSift(Mat image, vector<KeyPoint> & kp, int nbOctaves = 6, double contrastTresh = 0.2, double edgeTresh = 2, double sigma = 1);

void afficherKeypoints(string cheminImage, vector<KeyPoint> kp, string nomFenetre = "Affichage des keypoints");
void afficherKeypoints(Mat image, vector<KeyPoint> kp, string nomFenetre = "Affichage des keypoints");

void getKeypointsAndDescriptorsSift(string chemin, vector<KeyPoint> & kp, Mat & desc, int nbOctaves = 6, double contrastTresh = 0.2, double edgeTresh = 2, double sigma = 1);
void getKeypointsAndDescriptorsSift(Mat image, vector<KeyPoint> & kp, Mat & desc, int nbOctaves = 6, double contrastTresh = 0.2, double edgeTresh = 2, double sigma = 1);

void filtrerKeypointsParMatching(vector<KeyPoint> & kpImage, Mat descPattern, Mat & descImage);
void match(vector<vector<DMatch>> & matches, Mat descSrc, Mat descDst, int kppv = 1);

void afficherMatch(Mat pattern, Mat image, vector<KeyPoint> kpPattern, vector<KeyPoint> kpImage, vector<vector<DMatch>> matches, string nomFenetre = "Matching");