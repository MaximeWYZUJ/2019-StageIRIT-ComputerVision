#include "stdafx.h"
#include "mesureCorrelation.h"

using namespace cv;
using namespace std;



double correlationClassique(Mat im1, Mat im2, bool normalize, double im2Norm) {
	// Conversion en double
	Mat im1d, im2d;
	im1.convertTo(im1d, CV_64F);
	im2.convertTo(im2d, CV_64F);

	// Normalisation
	if (normalize) {
		if (im2Norm < 0) {
			double norme = normeImage(im2);
			im2d = im2d / norme;
		}
		else {
			im2d = im2d / im2Norm;
		}
	}
	else {
		im1d = im1d / 255.0;
		im2d = im2d / 255.0;
	}

	// Calcul de la correlation
	double c = 0.0;
	for (int i = 0; i < im1.rows; i++) {
		for (int j = 0; j < im1.cols; j++) {
			c += abs(im1d.at<double>(i, j) - im2d.at<double>(i, j));
		}
	}

	c = c / (im1.rows * im1.cols * 3);

	return c;
}


double correlationClassiqueCarre(Mat im1, Mat im2, bool normalize, double im2Norm) {
	// Conversion en double
	Mat im1d, im2d;
	im1.convertTo(im1d, CV_64F);
	im2.convertTo(im2d, CV_64F);

	// Normalisation
	if (normalize) {
		if (im2Norm < 0) {
			double norme = normeImage(im2);
			im2d = im2d / norme;
		}
		else {
			im2d = im2d / im2Norm;
		}
	}
	else {
		im1d = im1d / 255.0;
		im2d = im2d / 255.0;
	}

	// Calcul de la correlation
	double c = 0.0;
	for (int i = 0; i < im1.rows; i++) {
		for (int j = 0; j < im1.cols; j++) {
			c += pow(static_cast<double>(im1d.at<double>(i, j) - im2d.at<double>(i, j)), 2);
		}
	}

	c = sqrt(c);

	c = c / (im1.rows * im1.cols * 3);

	return c;
}


double correlationCroisee(Mat im1, Mat im2) {
	// Conversion des images en double
	Mat imd1, imd2;
	im1.convertTo(imd1, CV_64F);
	im2.convertTo(imd2, CV_64F);

	// Calcul de l'intensite moyenne de chaque image
	double i_moy1 = 0, i_moy2 = 0;
	for (int i = 0; i < im1.rows; i++) {
		for (int j = 0; j < im1.cols; j++) {
			i_moy1 += imd1.at<double>(i, j);
			i_moy2 += imd2.at<double>(i, j);
		}
	}
	i_moy1 = i_moy1 / (imd1.rows * imd1.cols);
	i_moy2 = i_moy2 / (imd2.rows * imd2.cols);

	// Calcul de la matrice centree
	Mat im1_centre = imd1 - i_moy1;
	Mat im2_centre = imd2 - i_moy2;

	// Produit scalaire et calcul du ZNCC
	Mat mult = im1_centre.mul(im2_centre);
	Mat im1_carre = im1_centre.mul(im1_centre);
	Mat im2_carre = im2_centre.mul(im2_centre);
	double norm1 = 0, norm2 = 0, sum = 0;
	for (int i = 0; i < im1_carre.rows; i++) {
		for (int j = 0; j < im2_carre.cols; j++) {
			sum += mult.at<double>(i, j);
			norm1 += im1_carre.at<double>(i, j);
			norm2 += im2_carre.at<double>(i, j);
		}
	}
	norm1 = sqrt(norm1);
	norm2 = sqrt(norm2);

	if (norm1 == 0 || norm2 == 0) {
		return 0;
	}

	double ZNCC = sum / (norm1 * norm2);
	return -ZNCC / 3;
}


double fastCorrelationCroisee(Mat imFixeCentree, double moyFixe, double normeFixe, Mat imGlissante, double & moyPreviousGlissante, DIRECTION d) {
	// Conversion en double
	Mat imdFixe, imdGlissante;
	imFixeCentree.convertTo(imdFixe, CV_64F);
	imGlissante.convertTo(imdGlissante, CV_64F);

	// Calcul de la moyenne de la fenetre glissante
	double moyGlissante = 0;
	switch (d) {
	case Fixe: {
		moyGlissante = moyenneImage(imGlissante);
		break;
	}
	case Horizontal: {
		double moyColonne = moyenneImage(imGlissante.col(imGlissante.cols - 1));
		double moyGlissante = (moyColonne + moyPreviousGlissante * (imGlissante.cols - 1)) / imGlissante.cols;
		break;
	}
	case Vertical: {
		double moyLigne = moyenneImage(imGlissante.row(imGlissante.rows - 1));
		double moyGlissante = (moyLigne + moyPreviousGlissante * (imGlissante.rows - 1)) / imGlissante.rows;
		break;
	}
	}
	moyPreviousGlissante = moyGlissante;

	
	// Calcul de la fenetre centree
	Mat imdGlissanteCentree = imdGlissante - moyGlissante;


	// Calcul de la norme de la fenetre
	double normeGlissante = normeImage(imdGlissanteCentree);


	// Calcul du produit scalaire
	double sum = 0;
	Mat ps = imdGlissanteCentree.mul(imFixeCentree);
	for (int i = 0; i < imdFixe.rows; i++) {
		for (int j = 0; j < imdGlissante.cols; j++) {
			sum += ps.at<double>(i, j);
		}
	}

	
	// Calcul du ZNCC
	double ZNCC = sum / (normeGlissante * normeFixe);

	if (normeGlissante == 0 || normeFixe == 0) {
		return 0;
	}
	else {
		return -ZNCC / 3;
	}
}


double correlationCENSUS(Mat im1, Mat im2) {
	Mat imd1, imd2;

	// Creation des chaines de recensement
	vector<int> recensement1, recensement2;
	int pixCentral1 = im1.at<uchar>(im1.rows / 2, im2.cols / 2);
	int pixCentral2 = im2.at<uchar>(im2.rows / 2, im2.cols / 2);
	for (int i = 0; i < im1.rows; i++) {
		for (int j = 0; j < im1.cols; j++) {
			int pix1 = im1.at<uchar>(i, j);
			int pix2 = im2.at<uchar>(i, j);
			pix1 > pixCentral1 ? recensement1.push_back(1) : recensement1.push_back(0);
			pix2 > pixCentral2 ? recensement2.push_back(1) : recensement2.push_back(0);
		}
	}

	// Calcul de la mesure CENSUS
	double CENSUS = 0;
	for (int k = 0; k < recensement1.size(); k++) {
		CENSUS += abs(recensement1.at(k) - recensement2.at(k));
	}

	// Normalisation de la mesure
	CENSUS = (double)CENSUS / (double)(recensement1.size() * 3);

	return CENSUS;
}


double correlationSMPD(Mat im1, Mat im2, int p, int h) {
	// Calcul de la difference des deux images
	Mat diff = im1 - im2;

	// Calcul de la difference mediane
	vector<double> intensites;
	for (int i = 0; i < im1.rows; i++) {
		for (int j = 0; j < im1.cols; j++) {
			intensites.push_back((double)diff.at<uchar>(i, j));
		}
	}
	vector<int> idx = sortIndexes(intensites);
	int medianne = (int)intensites.at(idx.at(idx.size() / 2));

	// Calcul du SMPD
	int SMPD = 0;
	for (int i = 0; i < min((int)intensites.size(), h); i++) {
		SMPD += pow(abs(intensites.at(idx.at(i)) - medianne), p);
	}

	// Normalisation
	SMPD = SMPD / ((double)3 * min(h, (int)intensites.size()));

	return SMPD;
}


double correlationGeometrique(vector<KeyPoint> kpPattern, Cluster c, vector<vector<DMatch>> matches) {
	// Pour chaque point du cluster, on va identifier le point du pattern qui match le mieux.
	// Ensuite, on essaiera de faire correspondre les points qui match en determinant un transformation
	// geometrique possible.
	
	vector<pair<Point2f, Point2f>> bestMatches;
	for (Point2 p : c.Point2s) {
		int id = p.indice; // c'est l'indice du kp dans l'image
		int correspondant = -1;
		double distanceCorrespondant = 1000000000;
		for (int i = 0; i < matches.size(); i++) {
			// on regarde si p est parmi les k ppv de matches[i]
			for (int k = 0; k < matches.at(i).size(); k++) {
				DMatch dm = matches.at(i).at(k);
				if (dm.trainIdx == id && dm.distance < distanceCorrespondant) {
					correspondant = i;
					distanceCorrespondant = dm.distance;
				}
			}
		}

		if (correspondant != -1) {
			Point2f pointC(p.values.at(0), p.values.at(1));
			pair<Point2f, Point2f> paire = make_pair(pointC, kpPattern.at(correspondant).pt);
			bestMatches.push_back(paire);
		}
	}

	if (bestMatches.size() >= 3) { // il nous en faut au moins 2 pour determiner la scale, et un troisieme pour tester

		double scale = estimateScale(bestMatches);
		cout << "Scale :  " << scale << endl;


		// On estime la correlation entre les points du cluster et les points du pattern
		// avec lesquels on a match
		double nbElementsCommuns = 0, nbElementsUnion = 0;
		Point2f origineCluster, originePattern;
		origineCluster = bestMatches.at(0).first;
		originePattern = bestMatches.at(0).second;
		for (int i = 1; i < bestMatches.size(); i++) {
			Point2f pointPattern = bestMatches.at(i).second;
			Point2f transformationPointPattern = transformation(originePattern, origineCluster, scale, pointPattern);
			Point2f pointCluster = bestMatches.at(i).first;

			double distance = sqrt(pow(pointCluster.x - transformationPointPattern.x, 2) + pow(pointCluster.y - transformationPointPattern.y, 2));
			if (distance <= 4) {
				nbElementsCommuns += 1.0;
				nbElementsUnion += 1.0;
			}
			else {
				nbElementsUnion += 2.0;
			}
		}


		// Calcul de l'indice de Sorensen-Dice
		//double score = 2.0 * nbElementsCommuns / (2.0 * (double)bestMatches.size() - 2.0); // on ne prend pas en compte les points d'origine du cluster et du pattern

		// Calcul de l'indice de Jaccard
		double score = nbElementsCommuns / nbElementsUnion;

		return score;
	}
	else {
		return 0;
	}
}


double correlationGeometriqueNoMatch(vector<KeyPoint> kpPattern, Cluster c, double scale) {
	// On cree un vecteur correspondant aux points du cluster, sans doublon
	vector<Point2f> clusterPoints = getPointsSansDoublon(c);

	// Initialisation
	int nbCorrespondantsMax = 0;
	Point2f origineCluster = clusterPoints.at(0);

	// On parcourt tous les kp du pattern
	for (int idKp = 0; idKp < kpPattern.size(); idKp++) {
		Point2f originePattern = kpPattern.at(idKp).pt;
		int nbCorrespondants = 0;

		// On parcourt tous les points du cluster pour verifier
		// s'ils ont un correspondant dans le pattern
		for (int idC = 1; idC < clusterPoints.size(); idC++) {
			Point2f pointC = clusterPoints.at(idC);
			Point2f correspondant = transformation(origineCluster, originePattern, scale, pointC);
			if (checkPointProche(correspondant, kpPattern, 30)) {
				nbCorrespondants += 1;
			}
		}

		nbCorrespondantsMax = max(nbCorrespondants, nbCorrespondantsMax);
	}
	
	return ((double)nbCorrespondantsMax / (double)(clusterPoints.size()-1));
}


double correlationHistogramme(Mat im1, Mat im2) {
	vector<int> hist1 = getHistogramme(im1);

	return correlationHistogramme(hist1, im2);
}


double correlationHistogramme(vector<int> histPattern, Mat image) {
	vector<int> hist1 = histPattern;
	vector<int> hist2 = getHistogramme(image);


	double normeHist1 = 0, normeHist2 = 0;
	int ps = 0;
	for (int i = 1; i < hist1.size(); i++) {
		normeHist1 += pow(hist1.at(i), 2);
		normeHist2 += pow(hist2.at(i), 2);
		ps += hist1.at(i) * hist2.at(i);
	}

	normeHist1 = sqrt(normeHist1);
	normeHist2 = sqrt(normeHist2);

	if (normeHist1 != 0 && normeHist2 != 0) {
		double score = (double)ps / (normeHist1 * normeHist2);
		score = score / 3;

		return -score;
	}
	else {
		return 0;
	}
}