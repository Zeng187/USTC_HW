#pragma once
#include <assert.h>
#include"Warping.h"

class WarpingRBF :public Warping
{
public:
	WarpingRBF() {};
	~WarpingRBF();
	WarpingRBF(std::vector<QLine*> warp_pairs_);
	QPoint Warp_func(QPoint point);		//the warping function constructed

private:
	inline double rbf(int i,double r);
	void calculate_parameters();
	std::vector<Vector2d> a;
	std::vector<double> r;
	Eigen::Matrix2d M;
	Eigen::Vector2d b;
};

