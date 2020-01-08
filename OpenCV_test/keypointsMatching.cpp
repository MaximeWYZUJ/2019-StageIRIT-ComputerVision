#include "stdafx.h"
#include "keypointsMatching.h"

using namespace cv;
using namespace std;


void filtrageParMatchingSeuil(vector<vector<DMatch>> matches, vector<KeyPoint> kp, Mat desc,
		vector<KeyPoint> & filteredKp, Mat & filteredDesc, double seuil = 250) {
	filteredKp = kp;
	Mat filteredDescBis;

	int i = 0;
	for (int j = 0; j < matches.size(); j++) {
		vector<DMatch> v = matches.at(j);

		if (v.at(0).distance > seuil) {
			filteredKp.erase(filteredKp.begin() + i); // on supprime le kp
		}
		else {
			Mat ligne = desc.row(j).clone();
			filteredDescBis.push_back(ligne);
			i++;
		}
	}

	filteredDesc = filteredDescBis;
}


void filtrageDesMatchLoweRatioTest(vector<vector<DMatch>> & matches, double ratio = 0.7) {

	for (int m = 0; m < matches.size(); m++) {
		vector<DMatch> v = matches.at(m);

		int i = 1;
		while (i < v.size()) {
			if (v.at(i).distance*ratio > v.at(i - 1).distance) {
				v.erase(v.begin() + i);
			}
			else {
				i++;
			}
		}

		matches.at(m) = v;
	}
}


void getKeypointsSift(Mat image, vector<KeyPoint> & kp, int nbOctaves, double contrastTresh, double edgeTresh, double sigma) {
	Ptr<xfeatures2d::SIFT> f2d = xfeatures2d::SIFT::create(0, nbOctaves, contrastTresh, edgeTresh, sigma);
	f2d->detect(image, kp);
}

void getKeypointsSift(string chemin, vector<KeyPoint> & kp, int nbOctaves, double contrastTresh, double edgeTresh, double sigma) {
	Mat image = imread(chemin, IMREAD_UNCHANGED);
	getKeypointsSift(image, kp, nbOctaves, contrastTresh, edgeTresh, sigma);
}


void afficherKeypoints(string cheminImage, vector<KeyPoint> kp, string nomFenetre) {
	Mat image = imread(cheminImage, IMREAD_UNCHANGED);
	Mat image_result;
	drawKeypoints(image, kp, image_result, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	namedWindow(nomFenetre, WINDOW_NORMAL);
	imshow(nomFenetre, image_result);
}

void afficherKeypoints(Mat image, vector<KeyPoint> kp, string nomFenetre) {
	Mat image_result;
	drawKeypoints(image, kp, image_result, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	namedWindow(nomFenetre, WINDOW_NORMAL);
	imshow(nomFenetre, image_result);
}


void getKeypointsAndDescriptorsSift(string chemin, vector<KeyPoint> & kp, Mat & desc, int nbOctaves, double contrastTresh, double edgeTresh, double sigma) {
	Mat image = imread(chemin, IMREAD_UNCHANGED);
	Ptr<xfeatures2d::SIFT> f2d = xfeatures2d::SIFT::create(0, nbOctaves, contrastTresh, edgeTresh, sigma);
	f2d->detectAndCompute(image, Mat(), kp, desc);
}

void getKeypointsAndDescriptorsSift(Mat image, vector<KeyPoint> & kp, Mat & desc, int nbOctaves, double contrastTresh, double edgeTresh, double sigma) {
	Ptr<xfeatures2d::SIFT> f2d = xfeatures2d::SIFT::create(0, nbOctaves, contrastTresh, edgeTresh, sigma);
	f2d->detectAndCompute(image, Mat(), kp, desc);
}

void filtrerKeypointsParMatching(vector<KeyPoint> & kpImage, Mat descPattern, Mat & descImage) {
	vector<vector<DMatch>> matches;
	BFMatcher matcher(NORM_L2, false);
	// on match l'image avec le pattern
	matcher.knnMatch(descImage, descPattern, matches, 1);

	// Pour chaque matching trouve, on check si la correspondance est suffisante.
	// Si elle ne l'est pas, on supprime le kp correspondant.
	// Si elle l'est, on conserve la ligne du descripteur associee a ce kp et on garde le kp.
	filtrageParMatchingSeuil(matches, kpImage, descImage, kpImage, descImage);
	
}


void match(vector<vector<DMatch>> & matches, Mat descSrc, Mat descDst, int kppv) {
	BFMatcher matcher(NORM_L2, false);

	matcher.knnMatch(descSrc, descDst, matches, kppv);

	if (kppv != 1) {
		int nb = 0;
		for (vector<DMatch> v : matches) {
			nb += v.size();
		}
		cout << "nb match avant :  " << nb << endl;
		
		filtrageDesMatchLoweRatioTest(matches);
		
		nb = 0;
		for (vector<DMatch> v : matches) {
			nb += v.size();
		}
		cout << "nb match apres :  " << nb << endl;
	}
}


void afficherMatch(Mat pattern, Mat image, vector<KeyPoint> kpPattern, vector<KeyPoint> kpImage, vector<vector<DMatch>> matches, string nomFenetre) {
	Mat resultat;
	drawMatches(pattern, kpPattern, image, kpImage, matches, resultat);
	namedWindow(nomFenetre, WINDOW_NORMAL);
	imshow(nomFenetre, resultat);
}
