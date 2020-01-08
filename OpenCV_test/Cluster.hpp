#ifndef MEANSHIFT_CLUSTER_HPP
#define MEANSHIFT_CLUSTER_HPP

#include "stdafx.h"
#include <vector>

#include "Point2.hpp"


class Cluster {
public:
    explicit Cluster(Point2 centroid);

    Point2 getCentroid() const;

    void addPoint2(Point2 Point2);

    long getSize() const;

    std::vector<Point2>::iterator begin();

    std::vector<Point2>::iterator end();

    float getSse() const;


    std::vector<Point2> Point2s;
    Point2 centroid;
};


#endif //MEANSHIFT_CLUSTER_HPP
