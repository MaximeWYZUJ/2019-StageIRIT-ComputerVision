#include "stdafx.h"
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>

#include "Point2.hpp"
#include "Cluster.hpp"
#include "csvUtils.hpp"

;

std::vector<Point2> getPoint2sFromCsv(std::string fileName)
{
    std::vector<Point2> Point2s;

    std::ifstream data(fileName);
    std::string line;
    while (std::getline(data, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<float> point;
        while (std::getline(lineStream, cell, ','))
            point.push_back(stod(cell));
        Point2s.emplace_back(Point2(point));
    }

    return Point2s;
}


void writeClustersToCsv(std::vector<Cluster> &clusters, std::string fileName)
{
    std::ofstream outputFile(fileName + ".csv");
    int clusterId = 0;
    for (auto &cluster : clusters) {
        for (auto &Point2 : cluster) {
            for (auto &value : Point2) {
                outputFile << value << ",";
            }
            outputFile << clusterId << "\n";
        }
        ++clusterId;
    }
}

