#ifndef MEANSHIFT_Point2_HPP
#define MEANSHIFT_Point2_HPP

#include "stdafx.h"
#include <vector>
#include <initializer_list>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc.hpp>


	class Point2 {

	public:
		explicit Point2(std::vector<float> values);

		Point2(std::initializer_list<float> values);

		explicit Point2(unsigned long dimensions);

		Point2() = default;

		bool operator==(const Point2 &p) const;

		bool operator!=(const Point2 &p) const;

		Point2 operator+(const Point2 &p) const;

		Point2 &operator+=(const Point2 &p);

		Point2 operator-(const Point2 &p) const;

		Point2 &operator-=(const Point2 &p);

		Point2 operator*(float d) const;

		Point2 &operator*=(float d);

		Point2 operator/(float d) const;

		Point2 &operator/=(float d);

		float &operator[](long index);

		const float &operator[](long index) const;

		unsigned long dimensions() const;

		std::vector<float>::const_iterator begin() const;

		std::vector<float>::const_iterator end() const;

		float euclideanDistance(const Point2 &p) const;

		std::vector<float> values;

		cv::KeyPoint kp;

		cv::Mat descriptor;

		int indice;
	};


#endif //MEANSHIFT_Point2_HPP
