#include "stdafx.h"
#include "fenetreGlissante.h"



using namespace cv;
using namespace std;


// Variables globales
DIRECTION d;
Mat patternB, patternG, patternR;
double moyFixeB, moyFixeG, moyFixeR;
double moyPreviousGlissanteB, moyPreviousGlissanteG, moyPreviousGlissanteR;
double normeFixeB, normeFixeG, normeFixeR;
double normePreviousGlissanteB, normePreviousGlissanteG, normePreviousGlissanteR;



// Tri dans l'ordre croissant. Renvoie les index du vecteur trie
vector<int> sort_indexes(const vector<double> &v) {

	// initialize original index locations
	vector<int> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}


vector<Point> maskIndex(Mat pattern)
{
	vector<Mat> channels;
	split(pattern, channels);
	Mat alpha = channels[pattern.channels()-1];

	vector<Point> ids; // points du mask ou l'alpha est a zero
	for (int i = 0; i < alpha.rows; i++) {
		for (int j = 0; j < alpha.cols; j++) {
			Point p(j, i);
			if (alpha.at<uchar>(p) < 50) {
				ids.push_back(p);
			}
		}
	}

	return ids;
}


void fenetreGlissante::rescale_pattern() {
	int l = ceil(pattern.rows*scale);
	int c = ceil(pattern.cols*scale);

	resize(pattern, pattern, Size(c, l));
}


void applyMask(Mat image, Mat & masked_image, Mat pattern) {
	vector<Point> id_mask = maskIndex(pattern);
	vector<Mat> channels;
	split(image, channels);
	for (Point p : id_mask) {
		for (Mat c : channels) {
			c.at<uchar>(p) = 0;
		}
	}
	merge(channels, masked_image);
}


fenetreGlissante::fenetreGlissante(int lignes, int colonnes, Mat image_originale, string nom_fenetre, double scale):
	nb_lignes(lignes), nb_colonnes(colonnes), image_origine(image_originale), nom_fenetre(nom_fenetre), pattern(), mask(), scale(scale)
{
}


fenetreGlissante::fenetreGlissante(int lignes, int colonnes, Mat image_originale, Mat pattern, string nom_fenetre, double scale) :
	nb_lignes(lignes), nb_colonnes(colonnes), image_origine(image_originale), nom_fenetre(nom_fenetre), pattern(pattern), mask(), scale(scale)
{
	applyMask(image_originale, mask, pattern);
}


fenetreGlissante::~fenetreGlissante()
{
}


void fenetreGlissante::disp_position()
{
	Point p(position.x + nb_lignes, position.y + nb_colonnes);
	Mat image = image_origine.clone();
	rectangle(image, position, p, Scalar(0, 255, 0), 2);
	imshow(nom_fenetre, image);
	waitKey(0);
}


void fenetreGlissante::trouver_pattern(vector<Point> & points_trouves, vector<double> & correlations, int id_norm) {
	points_trouves.clear();
	correlations.clear();

	nb_lignes = pattern.rows;
	nb_colonnes = pattern.cols;

	// Calcul de donnees recurrentes (pour ne pas avoir a le faire a chaque mesure)
	vector<Mat> channelsPattern;
	split(pattern, channelsPattern);

	Mat c1, c2, c3;
	channelsPattern.at(0).convertTo(c1, CV_64F);
	channelsPattern.at(1).convertTo(c2, CV_64F);
	channelsPattern.at(2).convertTo(c3, CV_64F);

	if (abs(id_norm) <= 3) {
		normeFixeB = normeImage(channelsPattern.at(0));
		normeFixeG = normeImage(channelsPattern.at(1));
		normeFixeR = normeImage(channelsPattern.at(2));
	}


	vector<Point> all_points;
	vector<double> all_corr;
	auto start = chrono::system_clock::now();
	if (id_norm > 0) {
		// Calcul du score case par case
		for (int i = offsetY; i < image_origine.rows - nb_lignes; i += nb_lignes) {
			for (int j = offsetX; j < image_origine.cols - nb_colonnes; j += nb_colonnes) {
				position.x = j;
				position.y = i;

				all_points.push_back(Point(j, i));
				double score = comparer(id_norm);
				all_corr.push_back(score);
			}
		}
	}
	else {

		// Calcul du score pixel par pixel
		moyFixeB = moyenneImage(channelsPattern.at(0));
		moyFixeG = moyenneImage(channelsPattern.at(1));
		moyFixeR = moyenneImage(channelsPattern.at(2));

		patternB = c1 - moyFixeB; // canaux centres
		patternG = c2 - moyFixeG;
		patternR = c3 - moyFixeR;

		double moyDebutLigneB = -1000;
		double moyDebutLigneG = -1000;
		double moyDebutLigneR = -1000;

		for (int i = zoneRechercheCoinHG.y; i <= zoneRechercheCoinBD.y - nb_lignes; i++) {
			for (int j = zoneRechercheCoinHG.x; j <= zoneRechercheCoinBD.x - nb_colonnes; j++) {

				position.x = j;
				position.y = i;

				// On definit la direction
				if (i == 0 && j == 0) {
					d = Fixe;
				}
				else if (j == 0) {
					// Dans le cas ou on est en debut de ligne,
					// on considere que la fenetre precedente etait
					// le debut de la precedente ligne
					d = Vertical;
					moyPreviousGlissanteB = moyDebutLigneB;
					moyPreviousGlissanteG = moyDebutLigneG;
					moyPreviousGlissanteR = moyDebutLigneR;
				}
				else {
					d = Horizontal;
				}

				
				
				// Calcul du score et maj de moyPreviousX
				all_points.push_back(Point(j, i));
				double score = comparer(id_norm);
				all_corr.push_back(score);



				if (j == 0) {
					// on maj les valeurs de debut de ligne pour la ligne suivante
					moyDebutLigneB = moyPreviousGlissanteB;
					moyDebutLigneG = moyPreviousGlissanteG;
					moyDebutLigneR = moyPreviousGlissanteR;
				}
			}
		}
	}

	auto end = chrono::system_clock::now();
	chrono::duration<double> duree = end - start;
	//std::cout << "Duree recherche :  " << duree.count() << endl;


	vector<int> idx_sorted_corr = sort_indexes(all_corr);

	double corr_min = all_corr.at(idx_sorted_corr.at(0));


	double seuil = 0;
	if (pourcentageSeuil == -1) {
		seuil = corr_min + abs(0.1*corr_min);
	}
	else {
		seuil = corr_min + abs(pourcentageSeuil*corr_min);
	}
	//std::cout << "Seuil :  " << seuil << endl;

	/*for (int i = 0; i < all_corr.size(); i++) {
		cout << all_corr.at(idx_sorted_corr.at(i)) << endl;
	}*/

	int id = 0;
	int id_sorted = idx_sorted_corr.at(id);
	while (all_corr.at(id_sorted) <= seuil && id < all_points.size()) {
		points_trouves.push_back(all_points.at(id_sorted));
		correlations.push_back(all_corr.at(id_sorted));
		std::cout << "Point trouve :  (" << all_points.at(id_sorted).x << " , " << all_points.at(id_sorted).y << ")  avec la correlation  " << all_corr.at(id_sorted) << endl;
		id++;
		if (id < all_points.size()) {
			id_sorted = idx_sorted_corr.at(id);
		}
	}
}


double fenetreGlissante::comparer(int id_norm)
{
	nb_lignes = pattern.rows;
	nb_colonnes = pattern.cols;

	// Get sous matrice de la fenetre
	Mat fenetre = image_origine.clone().rowRange(position.y, position.y + nb_lignes).colRange(position.x, position.x + nb_colonnes);
	
	// Apply mask
	applyMask(fenetre, fenetre, pattern);

	// Calcul de la correlation sur chaque canal
	vector<Mat> channels_fenetre, channels_pattern;
	split(fenetre, channels_fenetre);
	split(pattern, channels_pattern);
	double correlationR, correlationV, correlationB;
	switch (id_norm) {
		case -5: {
			correlationB = correlationSMPD(channels_fenetre.at(0), channels_pattern.at(0));
			correlationV = correlationSMPD(channels_fenetre.at(1), channels_pattern.at(1));
			correlationR = correlationSMPD(channels_fenetre.at(2), channels_pattern.at(2));
			break;
		}
		case -4: {
			correlationB = correlationCENSUS(channels_fenetre.at(0), channels_pattern.at(0));
			correlationV = correlationCENSUS(channels_fenetre.at(1), channels_pattern.at(1));
			correlationR = correlationCENSUS(channels_fenetre.at(2), channels_pattern.at(2));
			break;
		}
		case -3: {
			correlationB = correlationClassiqueCarre(channels_fenetre.at(0), channels_pattern.at(0), true, normeFixeB);
			correlationV = correlationClassiqueCarre(channels_fenetre.at(1), channels_pattern.at(1), true, normeFixeG);
			correlationR = correlationClassiqueCarre(channels_fenetre.at(2), channels_pattern.at(2), true, normeFixeR);
			break;
		}
		case -2: {
			correlationB = correlationClassique(channels_fenetre.at(0), channels_pattern.at(0), true, normeFixeB);
			correlationV = correlationClassique(channels_fenetre.at(1), channels_pattern.at(1), true, normeFixeG);
			correlationR = correlationClassique(channels_fenetre.at(2), channels_pattern.at(2), true, normeFixeR);
			break;
		}
		case -1: {
			correlationB = fastCorrelationCroisee(patternB, moyFixeB, normeFixeB, channels_fenetre.at(0), moyPreviousGlissanteB, d);
			correlationV = fastCorrelationCroisee(patternG, moyFixeG, normeFixeG, channels_fenetre.at(1), moyPreviousGlissanteG, d);
			correlationR = fastCorrelationCroisee(patternR, moyFixeR, normeFixeR, channels_fenetre.at(2), moyPreviousGlissanteR, d);
			break;
		}
		case 1 : {
			correlationB = correlationCroisee(channels_fenetre.at(0), channels_pattern.at(0));
			correlationV = correlationCroisee(channels_fenetre.at(1), channels_pattern.at(1));
			correlationR = correlationCroisee(channels_fenetre.at(2), channels_pattern.at(2));
			break;
		}
		case 2: {
			correlationB = correlationClassique(channels_fenetre.at(0), channels_pattern.at(0), true, normeFixeB);
			correlationV = correlationClassique(channels_fenetre.at(1), channels_pattern.at(1), true, normeFixeG);
			correlationR = correlationClassique(channels_fenetre.at(2), channels_pattern.at(2), true, normeFixeR);
			break;
		}
		case 3: {
			correlationB = correlationClassiqueCarre(channels_fenetre.at(0), channels_pattern.at(0), true, normeFixeB);
			correlationV = correlationClassiqueCarre(channels_fenetre.at(1), channels_pattern.at(1), true, normeFixeG);
			correlationR = correlationClassiqueCarre(channels_fenetre.at(2), channels_pattern.at(2), true, normeFixeR);
			break;
		}
		case 4: {
			correlationB = correlationCENSUS(channels_fenetre.at(0), channels_pattern.at(0));
			correlationV = correlationCENSUS(channels_fenetre.at(1), channels_pattern.at(1));
			correlationR = correlationCENSUS(channels_fenetre.at(2), channels_pattern.at(2));
			break;
		}
		case 5: {
			correlationB = correlationSMPD(channels_fenetre.at(0), channels_pattern.at(0));
			correlationV = correlationSMPD(channels_fenetre.at(1), channels_pattern.at(1));
			correlationR = correlationSMPD(channels_fenetre.at(2), channels_pattern.at(2));
			break;
		}
	}
	
	//std::cout << correlationR << " ; " << correlationV << " ; " << correlationB << "    ->    " << correlationR + correlationV + correlationB << endl;

	return correlationR + correlationV + correlationB;
}


void fenetreGlissante::filtrer_points_trouves(vector<Point> & points) {
	int width = pattern.cols;
	int height = pattern.rows;

	// On recupere les fenetres associees a chaque point
	vector<Mat> fenetres;
	for (Point p : points) {
		Mat f = image_origine.colRange(p.x, p.x + width).rowRange(p.y, p.y + height);
		fenetres.push_back(f);
	}

	// On cree le filtre par histogramme
	vector<bool> filtreHist = filtrerParHistogramme(pattern, fenetres);

	// On supprime les points en trop
	vector<Point> pointsFiltres;
	for (int i = 0; i < points.size(); i++) {
		if (filtreHist.at(i)) {
			pointsFiltres.push_back(points.at(i));
		}
	}

	points = pointsFiltres;
}


Mat fenetreGlissante::disp_points_trouves(vector<Point> points, string nom) {
	Mat image = image_origine.clone();
	for (Point p : points) {
		rectangle(image, p, Point(p.x+nb_lignes, p.y+nb_colonnes), Scalar(0, 255, 0), 2);
	}

	namedWindow(nom, WINDOW_NORMAL);
	imshow(nom, image);

	return image;
}


void fenetreGlissante::disp_points_trouves(vector<Point> points, Mat & image, Scalar couleur) {
	for (Point p : points) {
		rectangle(image, p, Point(p.x + nb_lignes, p.y + nb_colonnes), couleur, 2);
	}
}


void fenetreGlissante::initOffsetsAndScale() {
	
	// Input utilisateur pour definir la taille d'une tuile
	Rect rectangle = selectROI(image_origine, true, false);
	Point2i br = rectangle.br();
	Point2i tl = rectangle.tl();

	// Rescale du pattern
	int width = br.x - tl.x;
	int height = br.y - tl.y;

	double horizontalScale = (double)width / (double)pattern.cols;
	double verticalScale = (double)height / (double)pattern.rows;

	double scale = (horizontalScale + verticalScale) / (double)2;

	resize(pattern, pattern, Size((int)pattern.rows * scale, (int)pattern.cols * scale));

	// Definition de l'offset
	int size = (int)(width + height) / 2;

	int n = br.x / size;
	int m = br.y / size;

	offsetX = br.x - n * size;
	offsetY = br.y - m * size;
}


void fenetreGlissante::initOffsets(string name) {

	// Input utilisateur pour definir la taille d'une tuile
	Rect rectangle = selectROI(name, image_origine, true, false);
	destroyWindow(name);

	Point2i br = rectangle.br();
	Point2i tl = rectangle.tl();

	// On cherche le score minimum dans une region centree sur le rectangle
	fenetreGlissante f(10, 10, image_origine, pattern, "fenetre glissante initialisation", 1.0);
	Point bd(min(image_origine.cols, br.x + pattern.cols / 3), min(image_origine.rows, br.y + pattern.rows / 3));
	Point hg(max(0, tl.x - pattern.cols / 3), max(0, tl.y - pattern.rows / 3));

	if (bd.x - hg.x < pattern.cols || bd.y - hg.y < pattern.rows) {
		bd.x = min(bd.x + pattern.cols / 2, image_origine.cols);
		bd.y = min(bd.y + pattern.rows / 2, image_origine.rows);

		hg.x = max(0, hg.x - pattern.cols / 2);
		hg.y = max(0, hg.y - pattern.rows / 2);
	}

	f.zoneRechercheCoinBD = bd;
	f.zoneRechercheCoinHG = hg;
	
	vector<Point> pointsTrouves;
	vector<double> correlations;
	f.trouver_pattern(pointsTrouves, correlations, -1);

	Point pointOptimal = pointsTrouves.at(0);

	// Definition de l'offset
	int n = pointOptimal.x / pattern.cols;
	int m = pointOptimal.y / pattern.rows;

	offsetX = pointOptimal.x - n * pattern.cols;
	offsetY = pointOptimal.y - m * pattern.rows;


	cout << "Fin de l'initialisation" << endl;
}



int mouseClickedX = 0;
int mouseClickedY = 0;
int sizeGrid = 10;
bool jobDone = false;

void displayGrid(Mat img, Mat& imgGrid) {
	
	imgGrid = img.clone();

	int offX = mouseClickedX - sizeGrid * (mouseClickedX / sizeGrid);
	int offY = mouseClickedY - sizeGrid * (mouseClickedY / sizeGrid);

	// On dessine la grille sur imgGrid
	for (int i = offX; i < img.cols - sizeGrid; i+=sizeGrid) {
		for (int j = offY; j < img.rows - sizeGrid; j+=sizeGrid) {
			rectangle(imgGrid, Rect(i, j, sizeGrid, sizeGrid), Scalar(0, 0, 255), 2);
		}
	}
}


void mouseCallBackFunction(int event, int x, int y, int flags, void* userdata) {
	switch (event) {
	case EVENT_LBUTTONDOWN: {
		// Clic gauche
		mouseClickedX = x;
		mouseClickedY = y;
		break;
	}
	case EVENT_RBUTTONDOWN: {
		// Clic droit
		jobDone = true;
		break;
	}
	case EVENT_MOUSEWHEEL: {
		// Molette qui tourne
		if (getMouseWheelDelta(flags) > 0) {
			sizeGrid += 1;
		}
		else {
			sizeGrid -= 1;
		}
		break;
	}
	}
}


void fenetreGlissante::initOffestsAndScalePerso() {

	Mat display = image_origine.clone();

	namedWindow("Initialisation", WINDOW_AUTOSIZE);
	setMouseCallback("Initialisation", mouseCallBackFunction, NULL);

	while (!jobDone) {
		displayGrid(image_origine, display);
		imshow("Initialisation", display);
		waitKey(0);
	}

	offsetX = mouseClickedX - sizeGrid * (mouseClickedX / sizeGrid);
	offsetY = mouseClickedY - sizeGrid * (mouseClickedY / sizeGrid);
	resize(pattern, pattern, Size(sizeGrid, sizeGrid));
}