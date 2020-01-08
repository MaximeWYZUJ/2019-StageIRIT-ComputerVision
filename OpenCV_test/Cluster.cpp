#include "stdafx.h"
#include <cmath>
#include <vector>

#include "Cluster.hpp"
#include "Point2.hpp"

;


Cluster::Cluster(Point2 centroid)
{
    this->centroid = std::move(centroid);
}

Point2 Cluster::getCentroid() const
{
    return centroid;
}

void Cluster::addPoint2(Point2 Point2)
{
    Point2s.emplace_back(Point2);
}

long Cluster::getSize() const
{
    return Point2s.size();
}

std::vector<Point2>::iterator Cluster::begin()
{
    return Point2s.begin();
}

std::vector<Point2>::iterator Cluster::end()
{
    return Point2s.end();
}

float Cluster::getSse() const
{
    float sum = 0.0;
    for (const Point2 &p : Point2s)
        sum += std::pow(p.euclideanDistance(centroid), 2);
    return sum;
}
