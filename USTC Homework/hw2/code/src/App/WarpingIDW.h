#pragma once

#include"Warping.h"

class WarpingIDW:public Warping
{
public:
	WarpingIDW() {};
	WarpingIDW(std::vector<QLine*> warp_pairs_);
	~WarpingIDW();
	QPoint Warp_func(QPoint point);		//the warping function constructed

private:
	inline double radial_power(double r);
	double calculate_coeff(Vector2d x, int i);
	void calculate_Di();
	std::vector<Matrix2d> D;

};