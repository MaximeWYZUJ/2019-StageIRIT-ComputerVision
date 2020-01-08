#include "stdafx.h"
#include <vector>
#include <algorithm>

#include "Point2.hpp"
#include "Cluster.hpp"
#include "ClustersBuilder.hpp"

;

ClustersBuilder::ClustersBuilder(const std::vector<Point2> &originalPoint2s, float clusterEps)
{
    this->originalPoint2s = originalPoint2s;
    shiftedPoint2s = originalPoint2s;
    // vector of booleans such that the element in position i is false if the i-th Point2
    // has stopped to shift
    shifting = std::vector<bool>(originalPoint2s.size(), true);
    this->clusterEps = clusterEps;
    this->shiftingEps = clusterEps / 10;
}


Point2 &ClustersBuilder::getShiftedPoint2(long index)
{
    return shiftedPoint2s[index];
}


void ClustersBuilder::shiftPoint2(const long index, const Point2 &newPosition)
{
    if (newPosition.euclideanDistance(shiftedPoint2s[index]) <= shiftingEps)
        shifting[index] = false;
    else
        shiftedPoint2s[index] = newPosition;
}


bool ClustersBuilder::hasStoppedShifting(long index)
{
    return !shifting[index];
}


bool ClustersBuilder::allPoint2sHaveStoppedShifting()
{
    return std::none_of(shifting.begin(), shifting.end(), [](bool v) { return v; });
}


std::vector<Point2>::iterator ClustersBuilder::begin()
{
    return shiftedPoint2s.begin();
}


std::vector<Point2>::iterator ClustersBuilder::end()
{
    return shiftedPoint2s.end();
}


std::vector<Cluster> ClustersBuilder::buildClusters()
{
    std::vector<Cluster> clusters;

    // shifted Point2s with distance minor or equal than clusterEps will go in the same cluster
    for (int i = 0; i < shiftedPoint2s.size(); ++i) {
        Point2 shiftedPoint2 = shiftedPoint2s[i];
        auto it = clusters.begin();
        auto itEnd = clusters.end();
        while (it != itEnd) {
            if (it->getCentroid().euclideanDistance(shiftedPoint2) <= clusterEps) {
                // the Point2 belongs to a cluster already created
                it->addPoint2(originalPoint2s[i]);
                break;
            }
            ++it;
        }
        if (it == itEnd) {
            // create a new cluster
            Cluster cluster(shiftedPoint2);
            cluster.addPoint2(originalPoint2s[i]);
            clusters.emplace_back(cluster);
        }
    }
    return clusters;
}