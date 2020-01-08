#pragma once
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <numeric>
#include "fenetreGlissante.h"
#include "fonctionsAnnexes.h"

using namespace cv;


void fenetreGlissantePyramide(Mat img, Mat pattern, int nbStep, int idNorme, double seuilZoneRecherche = 0.3, double seuilFiltrageFinal = 0.1);