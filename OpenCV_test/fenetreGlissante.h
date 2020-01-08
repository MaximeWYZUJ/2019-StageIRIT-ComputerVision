#pragma once
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "mesureCorrelation.h"
#include <iostream>
#include <numeric>
#include <fstream>


using namespace cv;
using namespace std;

class fenetreGlissante
{
public:
	fenetreGlissante(int lignes, int colonnes, Mat image_originale, string nom_fenetre = "nouv", double scale = 1);
	fenetreGlissante(int lignes, int colonnes, Mat image_originale, Mat pattern, string nom_fenetre = "nouv", double scale = 1);
	~fenetreGlissante();

	// normes positives => parcours sur la grille
	// normes negatives => parcours pixel par pixel
	// 1 : ZNCC,  2 : L1,  3 : L2,  4 : CENSUS
	void trouver_pattern(vector<Point> & points_trouves, vector<double> & correlations, int id_norm = 1);
	double comparer(int id_norm = 1);
	void disp_position();
	Mat disp_points_trouves(vector<Point> points, string nom = "fenetre glissante");
	void disp_points_trouves(vector<Point> points, Mat & image, Scalar couleur = Scalar(0, 255, 0));
	void rescale_pattern();
	void filtrer_points_trouves(vector<Point> & points);
	void initOffsetsAndScale();
	void initOffestsAndScalePerso();
	void initOffsets(string name = "Initialisation");


	double scale;
	double pourcentageSeuil = -1;
	int nb_lignes, nb_colonnes;
	Mat mask;
	Point position;
	int decalage_x, decalage_y;
	string nom_fenetre;
	Mat image_origine;
	Mat pattern;
	int offsetX = 0, offsetY = 0;
	Point zoneRechercheCoinHG, zoneRechercheCoinBD;
};
