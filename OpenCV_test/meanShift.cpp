#include "stdafx.h"
#include <vector>
#include <iostream>
#include <cmath>

#include "Point2.hpp"
#include "Cluster.hpp"
#include "ClustersBuilder.hpp"
#include "meanShift.hpp"


std::vector<Cluster> meanShift(const std::vector<Point2> &Point2s, float bandwidth)
{
    ClustersBuilder builder = ClustersBuilder(Point2s, 0.4);
    long iterations = 0;
    unsigned long dimensions = Point2s[0].dimensions();
    float radius = bandwidth * 3;
    float doubledSquaredBandwidth = 2 * bandwidth * bandwidth;
    while (!builder.allPoint2sHaveStoppedShifting() && iterations < MAX_ITERATIONS) {

#pragma omp parallel for default(none) \
shared(Point2s, dimensions, builder, bandwidth, radius, doubledSquaredBandwidth) \
schedule(dynamic)

        for (long i = 0; i < Point2s.size(); ++i) {
            if (builder.hasStoppedShifting(i))
                continue;

            Point2 newPosition(dimensions);
            Point2 Point2ToShift = builder.getShiftedPoint2(i);
            float totalWeight = 0.0;
            for (auto &Point2 : Point2s) {
                float distance = Point2ToShift.euclideanDistance(Point2);
                if (distance <= radius) {
                    float gaussian = std::exp(-(distance * distance) / doubledSquaredBandwidth);
                    newPosition += Point2 * gaussian;
                    totalWeight += gaussian;
                }
            }

            // the new position of the Point2 is the weighted average of its neighbors
            newPosition /= totalWeight;
            builder.shiftPoint2(i, newPosition);
        }
        ++iterations;
    }
    if (iterations == MAX_ITERATIONS)
        std::cout << "WARNING: reached the maximum number of iterations" << std::endl;
    return builder.buildClusters();
}

