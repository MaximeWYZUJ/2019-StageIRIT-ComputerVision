#include "stdafx.h"
#include "fonctionsAnnexes.h"


using namespace cv;
using namespace std;


// Tri dans l'ordre croissant. Renvoie les index du vecteur trie
vector<int> sortIndexes(const vector<double> &v) {

	// initialize original index locations
	vector<int> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}


// Renvoie le point ou devrait se trouver la correspondance de pointPattern dans l'image
Point2f transformation(Point2f originePattern, Point2f origineCluster, double scale, Point2f pointPattern) {
	double dx = pointPattern.x - originePattern.x;
	double dy = pointPattern.y - originePattern.y;
	Point p(origineCluster.x + dx * scale, origineCluster.y + dy * scale);
	return p;
}


bool checkPointProche(Point2f point, vector<KeyPoint> allKp, int imprecision, bool checkSelf) {
	Rect2f rectangle = Rect2f(point.x - imprecision / 2, point.y - imprecision / 2, imprecision, imprecision);
	for (KeyPoint kp : allKp) {
		if (kp.pt.inside(rectangle)) {
			if (!checkSelf || (abs(point.x - kp.pt.x) > 1) && abs(point.y - kp.pt.y) > 1) {
				return true;
			}
		}
	}
	return false;
}


bool checkPointProche(Point2f point, vector<Point2> allPt, int imprecision, bool checkSelf) {
	Rect2f rectangle = Rect2f(point.x - imprecision / 2.0, point.y - imprecision / 2.0, imprecision, imprecision);
	for (Point2 p : allPt) {
		if (p.kp.pt.inside(rectangle)) {
			if (!checkSelf || (abs(point.x - p.kp.pt.x) > 1) && abs(point.y - p.kp.pt.y) > 1) {
				return true;
			}
		}
	}
	return false;
}


vector<Point2f> getPointsSansDoublon(Cluster c) {
	vector<Cluster> groupes = meanShift(c.Point2s, 3);

	vector<Point2f> points;
	for (Cluster g : groupes) {
		Point2f centre(g.getCentroid().values.at(0), g.getCentroid().values.at(1));
		points.push_back(centre);
	}
	return points;
}


vector<Point> getPointsSansDoublon(vector<Point> points, double rayon) {
	// Construction des Point2
	vector<Point2> points2;
	for (Point p : points) {
		vector<float> v = { (float)p.x, (float)p.y };
		Point2 p2(v);
		points2.push_back(p2);
	}
	
	// Rassemblement des doublons
	vector<Cluster> clusters = meanShift(points2, rayon);

	// Reconstruction des Point
	vector<Point> pointsSansDoublon;
	for (Cluster c : clusters) {
		float x = c.getCentroid().values.at(0);
		float y = c.getCentroid().values.at(1);
		Point p((int)x, (int)y);
		pointsSansDoublon.push_back(p);
	}

	return pointsSansDoublon;
}


vector<int> getHistogramme(Mat image) {
	// Initialisation de l'histogramme
	vector<int> hist(256);
	for (int i = 0; i < 256; i++) {
		hist.at(i) = 0;
	}

	// Calcul de l'histogramme
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			int id = image.at<uchar>(i, j);
			hist.at(id) += 1;
		}
	}

	return hist;
}


void afficherHistogramme(const vector<int>& data, string nomFenetre)
{
	// Initialisation
	int binSize = 3;
	int height = 0;
	

	int max_value = *max_element(data.begin(), data.end());
	int rows = 0;
	int cols = 0;
	if (height == 0) {
		rows = max_value + 10;
	}
	else {
		rows = max(max_value + 10, height);
	}

	cols = data.size() * binSize;

	Mat3b dst = Mat3b(rows, cols, Vec3b(0, 0, 0));

	for (int i = 0; i < data.size(); ++i)
	{
		int h = rows - data[i];
		rectangle(dst, Point(i*binSize, h), Point((i + 1)*binSize - 1, rows), (i % 2) ? Scalar(0, 100, 255) : Scalar(0, 0, 255), FILLED);
	}


	// Affichage
	namedWindow(nomFenetre, WINDOW_NORMAL);
	imshow(nomFenetre, dst);
}


double estimateScale(vector<pair<Point2f, Point2f>> matchingPoints) {

	vector<double> ratios;
	for (int i = 0; i < matchingPoints.size(); i++) {
		pair<Point2f, Point2f> matchI = matchingPoints.at(i);

		for (int j = i + 1; j < matchingPoints.size(); j++) {
			pair<Point2f, Point2f> matchJ = matchingPoints.at(j);
			
			double distanceC = sqrt(pow(matchI.first.x - matchJ.first.x, 2) + pow(matchI.first.y - matchJ.first.y, 2));
			if (distanceC > 0.1) {
				double distanceP = sqrt(pow(matchI.second.x - matchJ.second.x, 2) + pow(matchI.second.y - matchJ.second.y, 2));
				
				ratios.push_back(distanceC / distanceP);
			}
		}
	}

	vector<int> idxSortedRatios = sortIndexes(ratios);
	double medianRatio = ratios.at(idxSortedRatios.at(ratios.size() / 2));

	return medianRatio;
}


double moyenneImage(Mat image) {
	// Conversion en double
	Mat imd;
	image.convertTo(imd, CV_64F);

	double i_moy = 0;
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			i_moy += imd.at<double>(i, j);
		}
	}
	i_moy = i_moy / (imd.rows * imd.cols);

	return i_moy;
}


double normeImage(Mat image) {
	// Conversion en double
	Mat imd;
	image.convertTo(imd, CV_64F);

	// Calcul de la norme
	double norm = 0;
	Mat imageCarre = imd.mul(imd);
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			norm += imageCarre.at<double>(i, j);
		}
	}
	norm = sqrt(norm);

	return norm;
}


vector<bool> filtrerParHistogramme(Mat pattern, vector<Mat> images, double seuilCoeff) {
	vector<int> histPattern = getHistogramme(pattern);

	// Calcul des correlations entre le pattern et chaque image
	vector<double> correlations;
	for (Mat img : images) {
		vector<Mat> chan;
		split(img, chan);

		double score = 0;
		for (int c = 0; c < min(3, (int)chan.size()); c++) {
			score += correlationHistogramme(histPattern, chan.at(c));
		}
		cout << score << endl;
		correlations.push_back(score);
	}
	
	// Tri des correlations
	vector<int> sortedIndexes = sortIndexes(correlations);
	double corrMin = correlations.at(sortedIndexes.at(0));
	double seuil = corrMin + abs(corrMin * seuilCoeff);

	// Creation du filtre : true -> on garde l'image, false -> on la supprime
	vector<bool> filtre;
	for (double corr : correlations) {
		if (corr < seuil) {
			filtre.push_back(true);
		}
		else {
			filtre.push_back(false);
		}
	}

	return filtre;
}