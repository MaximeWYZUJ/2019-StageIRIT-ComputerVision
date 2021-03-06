#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc.hpp>
#include "meanShift.hpp"
#include "Point2.hpp"
#include "Cluster.hpp"
#include "fenetreGlissante.h"
#include "fenetreGlissantePyramide.h"
#include "keypointsMatching.h"
#include "mesureCorrelation.h"
#include "fonctionsAnnexes.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <numeric>



/*
using namespace cv;
using namespace std;


static vector<vector<DMatch>> default_vect_dm;
static vector<KeyPoint> default_kp;
static Mat default_Mat;

// Variables globales
vector<vector<DMatch>> matching_Pattern2Image;
vector<KeyPoint> matchedKeyPoints_Pattern, matchedKeyPoints_Image;
Mat matchedDescriptor_Pattern, matchedDescriptor_Image;



int matchPattern(string chemin_pattern, string chemin_image, int nbOctaves,
		double contrastTresh, double edgeTresh, double sigma, bool onlyOne = true) {

	// Acquisition des images a comparer
	Mat pattern = imread(chemin_pattern, IMREAD_UNCHANGED);
	Mat imageCheck = imread(chemin_image, IMREAD_UNCHANGED);


	// Detection des keypoints dans chaque image
	auto startDetection = chrono::system_clock::now();
	std::vector<KeyPoint> kpSrc, kpDst;
	Mat descSrc, descDst;
	getKeypointsAndDescriptorsSift(pattern, kpSrc, descSrc, nbOctaves, contrastTresh, edgeTresh, sigma);
	getKeypointsAndDescriptorsSift(imageCheck, kpDst, descDst, nbOctaves, contrastTresh, edgeTresh, sigma);
	auto endDetection = chrono::system_clock::now();
	chrono::duration<double> dureeDetection = endDetection - startDetection;
	cout << "Detection des kp et extraction des descripteurs :  " << dureeDetection.count() << endl;
	

	// Filtrage des keypoints
	auto startFiltrage = chrono::system_clock::now();
	int nbKpStart = kpDst.size();
	filtrerKeypointsParMatching(kpDst, descSrc, descDst); // maj des kp de imageCheck
	auto endFiltrage = chrono::system_clock::now();
	int nbKpEnd = kpDst.size();
	chrono::duration<double> dureeFiltrage = endFiltrage - startFiltrage;
	cout << "Filtrage des kp :  " << dureeFiltrage.count() << endl;
	cout << "nb kp :  " << nbKpStart << "  ->  " << nbKpEnd << "  ;  filtrage de " << nbKpStart - nbKpEnd << " kp" << endl;


	// Matching des keypoints
	auto startMatching = chrono::system_clock::now();
	vector<vector<DMatch>> matches;
	match(matches, descSrc, descDst, 10);
	auto endMatching = chrono::system_clock::now();
	chrono::duration<double> dureeMatching = endMatching - startMatching;
	cout << "Matching :  " << dureeMatching.count() << endl;


	// Initialisation des variables globales
	matchedKeyPoints_Image = kpDst;
	matchedKeyPoints_Pattern = kpSrc;
	matchedDescriptor_Image = descDst;
	matchedDescriptor_Pattern = descSrc;
	matching_Pattern2Image = matches;
	
	return 0;
}


vector<KeyPoint> getNuageKp(vector<vector<DMatch>> matches, vector<KeyPoint> kp_dst) {
	vector<int> index_matched_kp;
	for (vector<DMatch> v : matches) {
		for (DMatch dm : v) {
			// On check si ce point n'est pas deja dans la liste des points a garder
			if (find(index_matched_kp.begin(), index_matched_kp.end(), dm.trainIdx) == index_matched_kp.end()) {
				index_matched_kp.push_back(dm.trainIdx);
			}
		}
	}
	
	// On cree le nuage de keypoints qui ont matche
	vector<KeyPoint> nuage;
	for (int idx : index_matched_kp) {
		nuage.push_back(kp_dst.at(idx));
	}

	return nuage;
}


void filtrerClustersParQuantite(vector<Cluster>& clusters, int nb_minimal_elements) {
	int i = 0;
	while (i < clusters.size()) {
		vector<Point2f> pointsCluster = getPointsSansDoublon(clusters.at(i));
		if (pointsCluster.size() < nb_minimal_elements) {
			clusters.erase(clusters.begin() + i);
		}
		else {
			++i;
		}
	}
}


void filtrerClustersParCorrelation(vector<Cluster> & clusters, Mat pattern, Mat image, int kernelSize = 10, double lambda = 1.3) {
	// Resize le pattern a la taille d'une tile
	Mat patternResized;
	int taille = 36;
	resize(pattern, patternResized, Size(taille, taille));
	Mat voisinagePattern = patternResized.colRange((int)(taille / 2 - kernelSize / 2), (int)(taille / 2 + kernelSize / 2));
	voisinagePattern = voisinagePattern.rowRange((int)(taille / 2 - kernelSize / 2), (int)(taille / 2 + kernelSize / 2));

	
	// Calcul des correlations des regions voisines de chaque cluster
	vector<double> correlations;
	for (Cluster c : clusters) {
		Point2 centre = c.getCentroid();
		int ligne = centre.values.at(0);
		int colonne = centre.values.at(1);
		Mat voisinage = image.colRange((int)(ligne - kernelSize / 2), (int)(ligne + kernelSize / 2));
		voisinage = voisinage.rowRange((int)(colonne - kernelSize / 2), (int)(colonne + kernelSize / 2));

		double correlation = correlationClassique(voisinage, voisinagePattern);
		cout << correlation << endl;
		correlations.push_back(correlation);
	}
	
	vector<int> indexes = sortIndexes(correlations);
	vector<Cluster> sortedLimitedClusters;
	for (int i = 0; i < min(1, (int)clusters.size()); i++) {
		sortedLimitedClusters.push_back(clusters.at(indexes.at(i)));
	}
	clusters = sortedLimitedClusters;
}


void filtrerClustersParGeometrie(vector<Cluster> & clusters, double seuil) {
	// Mesure de la correspondance sur chaque cluster
	vector<double> mesures;
	vector<Cluster> goodClusters;
	for (Cluster c : clusters) {
		double m = correlationGeometrique(matchedKeyPoints_Pattern, c, matching_Pattern2Image);
		//double m = correlationGeometriqueNoMatch(matchedKeyPoints_Pattern, c);
		cout << m << endl;
		if (m > seuil) {
			goodClusters.push_back(c);
		}
	}

	clusters = goodClusters;
}


void clusterisation(vector<Cluster> & clusters, Mat pattern, Mat image) {

	// Obtention du nuage de points matches
	vector<KeyPoint> nuage = getNuageKp(matching_Pattern2Image, matchedKeyPoints_Image);


	// Conversion des KP en Point pour appliquer le meanshift
	auto startConversion = chrono::system_clock::now();
	vector<Point2f> ptsImage; // points associes aux kp de l'image
	for (KeyPoint k : matchedKeyPoints_Image) {
		ptsImage.push_back(k.pt);
	}

	vector<Point2> points; // points utilisables par meanShift
	for (KeyPoint kp : nuage) {
		vector<float> coo;
		coo.push_back(kp.pt.x);
		coo.push_back(kp.pt.y);
		Point2 p(coo);

		// Ajout du kp correspondant
		p.kp = kp;

		// Ajout du descripteur correspondant
		vector<Point2f>::iterator iter = find(ptsImage.begin(), ptsImage.end(), kp.pt);
		int id = distance(ptsImage.begin(), iter);
		Mat desc = matchedDescriptor_Image.row(id).clone();
		p.descriptor = desc;

		// Ajout de l'indice du point
		p.indice = id;

		// Ajout du point dans le vecteur
		points.push_back(p);
	}
	auto endConversion = chrono::system_clock::now();
	chrono::duration<double> dureeConversion = endConversion - startConversion;
	cout << "Conversion des kp en points :  " << dureeConversion.count() << endl;


	// Meanshift sur ces points
	auto startMeanshift = chrono::system_clock::now();
	clusters = meanShift(points, 12);
	auto endMeanshift = chrono::system_clock::now();
	chrono::duration<double> dureeMeanshift = endMeanshift - startMeanshift;
	cout << "Meanshift :  " << dureeMeanshift.count() << endl;


	int nbClustersStart, nbClustersEnd;
	// Filtrage des clusters avec un minimum de points
	auto startFiltreCQTT = chrono::system_clock::now();
	nbClustersStart = clusters.size();
	filtrerClustersParQuantite(clusters, 3);
	auto endFiltreCQTT = chrono::system_clock::now();
	nbClustersEnd = clusters.size();
	chrono::duration<double> dureeFiltreCQTT = endFiltreCQTT - startFiltreCQTT;
	cout << "Filtrage des clusters par qtt :  " << dureeFiltreCQTT.count() << endl;
	cout << "nb clusters :  " << nbClustersStart << "  ->  " << nbClustersEnd << "  ;  filtrage de " << nbClustersStart - nbClustersEnd << " clusters" << endl;
	

	// Filtrage des clusters par geometrie
	auto startFiltreGeom = chrono::system_clock::now();
	nbClustersStart = clusters.size();
	filtrerClustersParGeometrie(clusters, 0.5);
	auto endFiltreGeom = chrono::system_clock::now();
	nbClustersEnd = clusters.size();
	chrono::duration<double> dureeFiltreGeom = endFiltreGeom - startFiltreGeom;
	cout << "Filtrage des clusters par geometrie :  " << dureeFiltreGeom.count() << endl;
	cout << "nb clusters :  " << nbClustersStart << "  ->  " << nbClustersEnd << "  ;  filtrage de " << nbClustersStart - nbClustersEnd << " clusters" << endl;
	
}


void clusterisation(vector<Cluster> & clusters, string cheminPattern, string cheminImage) {
	Mat pattern = imread(cheminPattern, IMREAD_UNCHANGED);
	Mat image = imread(cheminImage, IMREAD_UNCHANGED);
	clusterisation(clusters, pattern, image);
}


int afficherCluster(string cheminPattern, string cheminImage) {
	vector<KeyPoint> nuage = getNuageKp(matching_Pattern2Image, matchedKeyPoints_Image);
	Mat imSrc = imread(cheminImage, IMREAD_UNCHANGED);
	Mat imNuage(imSrc.rows, imSrc.cols, CV_8UC3, Scalar(255, 255, 255)); // creation de l'image aux memes dimensions que l'image de depart (bg)

	drawKeypoints(imNuage, nuage, imNuage, Scalar(0, 0, 255), DrawMatchesFlags::DEFAULT);
	cv::namedWindow("Nuage de keypoints", WINDOW_NORMAL);
	imshow("Nuage de keypoints", imNuage);

	cv::waitKey(0);


	vector<Cluster> clusters;
	clusterisation(clusters, cheminPattern, cheminImage);

	// Affichage des clusters sans doublons
	for (Cluster c : clusters) {
		vector<Point2f> pointsSansDoublon = getPointsSansDoublon(c);
		for (Point2f p : pointsSansDoublon) {
			circle(imNuage, p, 2, Scalar(255, 0, 0), 2);
		}
	}

	
	// Affichage des centres des clusters
	for (Cluster c : clusters) {
		Point2 cluster_centre = c.getCentroid();
		Point centre(cluster_centre.values.at(0), cluster_centre.values.at(1));
		circle(imNuage, centre, 3, Scalar(0, 255, 0), 3);
		circle(imSrc, centre, 3, Scalar(0, 255, 0), 3);
	}
	cv::namedWindow("Nuage de keypoints et clusters", WINDOW_NORMAL);
	imshow("Nuage de keypoints et clusters", imNuage);
	cv::namedWindow("Points ou cliquer", WINDOW_NORMAL);
	imshow("Points ou cliquer", imSrc);

	cv::waitKey(0);

	destroyAllWindows();

	return 0;
}
*/

#include <vector>


using namespace cv;
using namespace std;

struct Data {
	string nom;
	int numero;
	bool photo;
};


int main()
{

	vector<string> nomsFichiers;
	vector<Data> donnees;
	glob("../DATA/*", nomsFichiers);
	
	// On recup les donnees
	for (string nf : nomsFichiers) {
		Data d;
		d.nom = nf;
		d.photo = (nf.substr(nf.size() - 3, nf.size() - 1) == "jpg");
		d.numero = stoi(nf.substr(0, nf.find('.')));
	}


	// Tri des donnees
	vector<Data> donneesTriees(donnees.size());
	for (Data d : donnees) {
		donneesTriees.at(d.numero) = d;
	}


	// Lecture / Ecriture
	for (int i = 0; i < donneesTriees.size(); i++) {
		if (donneesTriees.at(i).photo) {
			Mat p = imread("../DATA/"+donneesTriees.at(i).nom);
			imwrite("../DATA_OUT/" + donneesTriees.at(i).nom, p);
		}

		auto t = chrono::system_clock::now();
		while ((chrono::system_clock::now()-t).count() < 1.5) {
			// do nothing
		}
	}
}

