#ifndef MEANSHIFT_CSVUTILS_HPP
#define MEANSHIFT_CSVUTILS_HPP

#include "stdafx.h"
#include <vector>
#include <string>

#include "Point2.hpp"
#include "Cluster.hpp"

std::vector<Point2> getPoint2sFromCsv(std::string fileName);

void writeClustersToCsv(std::vector<Cluster> &clusters, std::string fileName);


#endif //MEANSHIFT_CSVUTILS_HPP
