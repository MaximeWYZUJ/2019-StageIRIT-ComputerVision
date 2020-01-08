#ifndef MEANSHIFT_MEANSHIFT_HPP
#define MEANSHIFT_MEANSHIFT_HPP

#include "stdafx.h"
#include <vector>

#include "Point2.hpp"
#include "Cluster.hpp"


#define MAX_ITERATIONS 100


std::vector<Cluster> meanShift(const std::vector<Point2> &Point2s, float bandwidth);


#endif //MEANSHIFT_MEANSHIFT_HPP
