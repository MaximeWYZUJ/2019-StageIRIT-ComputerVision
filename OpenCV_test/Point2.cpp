#include "stdafx.h"
#include <utility>  // std::pair
#include <cmath>  // std::sqrt
#include <vector>
#include <initializer_list>

#include "Point2.hpp"


Point2::Point2(std::vector<float> values)
{
    this->values = std::move(values);
}


Point2::Point2(std::initializer_list<float> values)
{
    this->values.assign(values);
}


Point2::Point2(unsigned long dimensions)
{
    this->values = std::vector<float>(dimensions, 0);
}


bool Point2::operator==(const Point2 &p) const
{
    return this->values == p.values;
}


bool Point2::operator!=(const Point2 &p) const
{
    return this->values != p.values;
}


Point2 Point2::operator+(const Point2 &p) const
{
    Point2 Point2(this->values);
    return Point2 += p;
}


Point2 &Point2::operator+=(const Point2 &p)
{
    for (long i = 0; i < p.dimensions(); ++i)
        this->values[i] += p[i];
    return *this;
}


Point2 Point2::operator-(const Point2 &p) const
{
    Point2 Point2(this->values);
    return Point2 -= p;
}


Point2 &Point2::operator-=(const Point2 &p)
{
    for (long i = 0; i < p.dimensions(); ++i)
        this->values[i] -= p[i];
    return *this;
}


Point2 Point2::operator*(const float d) const
{
    Point2 Point2(this->values);
    return Point2 *= d;
}


Point2 &Point2::operator*=(const float d)
{
    for (long i = 0; i < dimensions(); ++i)
        this->values[i] *= d;
    return *this;
}


Point2 Point2::operator/(const float d) const
{
    Point2 Point2(this->values);
    return Point2 /= d;
}


Point2 &Point2::operator/=(const float d)
{
    for (long i = 0; i < dimensions(); ++i)
        this->values[i] /= d;
    return *this;
}


float &Point2::operator[](const long index)
{
    return values[index];
}


const float &Point2::operator[](const long index) const
{
    return values[index];
}


unsigned long Point2::dimensions() const
{
    return values.size();
}


std::vector<float>::const_iterator Point2::begin() const
{
    return values.begin();
}


std::vector<float>::const_iterator Point2::end() const
{
    return values.end();
}


float Point2::euclideanDistance(const Point2 &p) const
{
    float sum = 0.0;
    for (std::pair<std::vector<float>::const_iterator, std::vector<float>::const_iterator> i(this->begin(), p.begin());
    i.first != this->end();  ++i.first, ++i.second) {
        float diff = *i.first - *i.second;
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

