#include "WarpingIDW.h"
#include <assert.h>
#include <iostream>

#define EPS 1e-15

WarpingIDW::WarpingIDW(std::vector<QLine*> warp_pairs_)
{
	Init(warp_pairs_);
	std::cout << "Finish initing" << std::endl;
	calculate_Di();
	std::cout << "Finish constucting warping function" << std::endl;
}

WarpingIDW::~WarpingIDW()
{
	if (quadtree != NULL)
	{
		delete quadtree;
	}
}

inline double WarpingIDW::radial_power(double r)
{
	//sqr means the square of distance r
	if (r < EPS)
		return 0;	// to avoid wrong case
	else
		return 1.0 / (r*r);
}

double WarpingIDW::calculate_coeff(Vector2d x, int i)
{
	assert(i >= 0 && i < p_.size());

	double sumtemp = 0.0; double d;
	for (int j = 0; j < p_.size(); j++)
	{
		d = (x - p_[j]).norm();
		if (d < EPS)
		{
			return i == j ? 1 : 0;
		}
		else
		{
			sumtemp += radial_power(d);
		}
		
	}

	d = (x - p_[i]).norm();

	double temp = radial_power(d);
	return temp / sumtemp;
}

void WarpingIDW::calculate_Di()
{
	int count = p_.size();
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			//std::cout << i << std::endl;
			Matrix2d A = Matrix2d();
			Vector2d b1 = Vector2d();
			Vector2d b2 = Vector2d();
			A << 0, 0, 0, 0;
			b1 << 0, 0; b2 = b1;

			for (int j = 0; j < count; j++)
			{
				//std::cout << j << std::endl;
				if (j == i)
				{
					continue;
				}
				else
				{
					Vector2d v1 = p_[j] - p_[i];
					Vector2d v2 = q_[j] - q_[i];
					double coeffi = radial_power((p_[j] - p_[i]).norm());
					//std::cout << coeffi << std::endl;

					A(0, 0) += v1.x() * v1.x() * coeffi;
					A(0, 1) += v1.x() * v1.y() * coeffi;
					A(1, 1) += v1.y() * v1.y() * coeffi;

					b1[0] += v2.x() * v1.x() * coeffi;
					b1[1] += v2.x() * v1.y() * coeffi;
					b2[0] += v2.y() * v1.x() * coeffi;
					b2[1] += v2.y() * v1.y() * coeffi;

				}
			}
			A(1, 0) = A(0, 1);

			Matrix2d Di = Matrix2d();
			Di.row(0) = A.colPivHouseholderQr().solve(b1);
			Di.row(1) = A.colPivHouseholderQr().solve(b2);

			D.push_back(Di);
		}

	}

	//std::cout << (D[0])(0,0) << std::endl;
	//std::cout << (Warp_func(QPoint(p_[0].x(), p_[0].y()))).x()<<'\t'<< (Warp_func(QPoint(p_[0].x(), p_[0].y()))).y()<<std::endl;

}

QPoint WarpingIDW::Warp_func(QPoint point)
{
	Vector2d x = Vector2d(point.x(), point.y());
	Vector2d value=Vector2d(0,0);
	for (int i = 0; i < p_.size(); i++)
	{
		double coeffi=calculate_coeff(x,i);
		value += coeffi * (q_[i] + D[i] * (x - p_[i]));
	}
	return QPoint(value.x(), value.y());
}