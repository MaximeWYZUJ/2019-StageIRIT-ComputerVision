#include "stdafx.h"
#include "fenetreGlissantePyramide.h"

using namespace cv;
using namespace std;



vector<Mat> creationEtagesLineaire(Mat image, int nbEtages) {
	vector<Mat> imagesStep;
	for (int i = nbEtages; i > 0; i--) {
		Mat imgResize;
		resize(image, imgResize, Size((image.cols * i) / nbEtages, (image.rows * i) / nbEtages), 0, 0, INTER_AREA);

		imagesStep.push_back(imgResize);
	}

	return imagesStep;
}


vector<Mat> creationEtagesExponentiel(Mat image, int nbEtages) {
	vector<Mat> imagesStep;
	for (int i = 0; i < nbEtages; i++) {
		Mat imgResize;
		resize(image, imgResize, Size(image.cols / pow(2, i), image.rows / pow(2, i)), 0, 0, INTER_AREA);

		imagesStep.push_back(imgResize);
	}

	return imagesStep;
}


void updateRegion(Point pointInSmallImage, Mat smallImage, Mat bigImage, Mat bigPattern, Point & regionHG, Point & regionBD) {
	double scaleFactor = (double)bigImage.cols / (double)smallImage.cols;

	int offsetX = floor(pointInSmallImage.x * scaleFactor);
	int offsetY = floor(pointInSmallImage.y * scaleFactor);

	double coeff = 0.5; // coefficient d'agrandissement de la zone de recherche par rapport au pattern

	offsetX = floor(offsetX - bigPattern.cols * (coeff / 2));
	offsetY = floor(offsetY - bigPattern.rows * (coeff / 2));

	int colMin = max(0, offsetX);
	int rowMin = max(0, offsetY);

	int regionTailleX = ceil(bigPattern.cols * (1 + coeff));
	int regionTailleY = ceil(bigPattern.rows * (1 + coeff));

	int colMax = min(bigImage.cols, colMin + regionTailleX);
	int rowMax = min(bigImage.rows, rowMin + regionTailleY);

	regionHG = Point(colMin, rowMin);
	regionBD = Point(colMax, rowMax);
}


void fenetreGlissantePyramide(Mat img, Mat pattern, int nbStep, int idNorme, double seuilZoneRecherche, double seuilFiltrageFinal) {

	// Creation des etages de la pyramide
	vector<Mat> imagesStep = creationEtagesExponentiel(img, nbStep);
	vector<Mat> patternsStep = creationEtagesExponentiel(pattern, nbStep);


	//  =============================================
	//  === Initialisation des zones de recherche ===
	//  =============================================
	int smallestImgIndex = imagesStep.size() - 1;
	Mat initialisationZone = imagesStep.at(smallestImgIndex).clone();
	Point zoneRechercheHG(0, 0), zoneRechercheBD(initialisationZone.cols, initialisationZone.rows);

	fenetreGlissante f(30, 50, imagesStep.at(smallestImgIndex), patternsStep.at(smallestImgIndex), "fenetre glissante", 1.0);
	f.zoneRechercheCoinBD = zoneRechercheBD;
	f.zoneRechercheCoinHG = zoneRechercheHG;
	f.pourcentageSeuil = seuilZoneRecherche;

	// On cherche les zones de recherche
	vector<Point> pointsTrouves;
	vector<double> correlations;
	f.trouver_pattern(pointsTrouves, correlations, idNorme);

	// On retire les doublons
	pointsTrouves = getPointsSansDoublon(pointsTrouves, 2);

	// Affichage
	f.disp_points_trouves(pointsTrouves, initialisationZone);
	cv::rectangle(initialisationZone, f.zoneRechercheCoinHG, f.zoneRechercheCoinBD, Scalar(0, 0, 255), 2);
	cv::namedWindow("initialisation zone", WINDOW_NORMAL);
	cv::imshow("initialisation zone", initialisationZone);

	// Initialisation des zones de recherche
	vector<Point> allZoneRechercheHG, allZoneRechercheBD;
	for (Point p : pointsTrouves) {

		Point zoneRechercheHG, zoneRechercheBD;
		updateRegion(p, imagesStep.at(smallestImgIndex), imagesStep.at(smallestImgIndex - 1), patternsStep.at(smallestImgIndex - 1), zoneRechercheHG, zoneRechercheBD);

		allZoneRechercheHG.push_back(zoneRechercheHG);
		allZoneRechercheBD.push_back(zoneRechercheBD);
	}


	//  ===========================================================================================
	//  === Recherche des patterns a chaque etage de la pyramide, dans chaque zone de recherche ===
	//  ===========================================================================================

	vector<Point> allPointsTrouves(allZoneRechercheBD.size());
	vector<double> allCorrelations(allZoneRechercheBD.size());

	for (int i = imagesStep.size() - 2; i >= 0; i--) {
		//cout << "Debut etape " << i << endl;
		Mat stepImage = imagesStep.at(i).clone();

		for (int z = 0; z < allZoneRechercheBD.size(); z++) {
			fenetreGlissante f(30, 50, imagesStep.at(i), patternsStep.at(i), "fenetre glissante", 1.0);
			f.zoneRechercheCoinBD = allZoneRechercheBD.at(z);
			f.zoneRechercheCoinHG = allZoneRechercheHG.at(z);


			// Recherche de la fenetre
			vector<double> correlations;
			vector<Point> pointsTrouves;
			f.trouver_pattern(pointsTrouves, correlations, idNorme);
			allPointsTrouves.at(z) = pointsTrouves.at(0);
			allCorrelations.at(z) = correlations.at(0);


			// Affichage de la fenetre
			vector<Point> pt;
			pt.push_back(pointsTrouves.at(0));
			f.disp_points_trouves(pt, stepImage);
			cv::rectangle(stepImage, f.zoneRechercheCoinHG, f.zoneRechercheCoinBD, Scalar(0, 0, 255), 2);


			// Setup pour la fenetre suivante, plus precise
			if (i >= 1) {
				updateRegion(allPointsTrouves.at(z), imagesStep.at(i), imagesStep.at(i - 1), patternsStep.at(i - 1), allZoneRechercheHG.at(z), allZoneRechercheBD.at(z));
			}
		}

		cout << "Etape " << i << endl;

		stringstream ss;
		ss << "etape " << i;
		cv::namedWindow(ss.str(), WINDOW_NORMAL);
		cv::imshow(ss.str(), stepImage);
	}


	// Filtrage des resultats obtenus
	vector<int> sortedIndexes = sortIndexes(allCorrelations);

	double corrMin = allCorrelations.at(sortedIndexes.at(0));
	double seuil = corrMin + abs(seuilFiltrageFinal*corrMin);

	vector<Point> allPointsTrouvesCorrects;
	vector<double> allCorrelationsCorrectes;
	
	for (int i = 0; i < allPointsTrouves.size(); i++) {
		if (allCorrelations.at(sortedIndexes.at(i)) < seuil) {
			allPointsTrouvesCorrects.push_back(allPointsTrouves.at(sortedIndexes.at(i)));
			allCorrelationsCorrectes.push_back(allCorrelations.at(sortedIndexes.at(i)));
		}
	}



	// Affichage du resultat
	Mat resultat = img.clone();
	for (int i = 0; i < allPointsTrouvesCorrects.size(); i++) {
		Point p1 = allPointsTrouvesCorrects.at(i);

		Point p2(p1.x + patternsStep.at(0).cols, p1.y + patternsStep.at(0).rows);
		
		std::cout << "Point trouve :  (" << (p1.x + p2.x) / 2 << " , " << (p1.y + p2.y) / 2 << ")  avec la correlation  " << allCorrelationsCorrectes.at(i) << endl;

		cv::rectangle(resultat, p1, p2, Scalar(0, 255, 0), 3);
	}
	cv::namedWindow("Resultat final", WINDOW_NORMAL);
	cv::imshow("Resultat final", resultat);

}