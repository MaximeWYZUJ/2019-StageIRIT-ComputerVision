#ifndef MEANSHIFT_CLUSTERBUILDER_HPP
#define MEANSHIFT_CLUSTERBUILDER_HPP

#include "stdafx.h"
#include <vector>

#include "Point2.hpp"
#include "Cluster.hpp"

class ClustersBuilder {
public:
    explicit ClustersBuilder(const std::vector<Point2> &originalPoint2s, float clusterEps);

    Point2 &getShiftedPoint2(long index);

    void shiftPoint2(long index, const Point2 &newPosition);

    bool hasStoppedShifting(long index);

    bool allPoint2sHaveStoppedShifting();

    std::vector<Point2>::iterator begin();

    std::vector<Point2>::iterator end();

    std::vector<Cluster> buildClusters();

private:
    std::vector<Point2> originalPoint2s;
    std::vector<Point2> shiftedPoint2s;
    // vector of booleans such that the element in position i is false if the i-th Point2
    // has stopped to shift
    std::vector<bool> shifting;
    float clusterEps;
    float shiftingEps;
};


#endif //MEANSHIFT_CLUSTERBUILDER_HPP
