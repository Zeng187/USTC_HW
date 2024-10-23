#include"WarpingRBF.h"
#include <iostream>

WarpingRBF::WarpingRBF(std::vector<QLine*> warp_pairs_)
{
	Init(warp_pairs_);
	std::cout << "Finish initing" << std::endl;
	calculate_parameters();
	std::cout << "Finish constucting warping function" << std::endl;
}

WarpingRBF::~WarpingRBF()
{
	if (quadtree != NULL)
	{
		delete quadtree;
	}
}

void WarpingRBF::calculate_parameters()
{
	//calcalate the affine part
	int count = p_.size();
	if (count == 0)
	{
		M << 1, 0, 0, 1;
		b << 0, 0;
	}
	else if (count == 1)
	{
		M << 1, 0, 0, 1;
		b = q_[0] - p_[0];
	}
	else if(count==2)
	{
		M(1, 0) = 0;
		M(0, 1) = 0;
		M(0, 0) = (p_[1] - p_[0]).x() == 0 ? 1 : double((q_[1] - q_[0]).x()) / (p_[1] - p_[0]).x();
		M(1, 1) = (p_[1] - p_[0]).y() == 0 ? 1 : double((q_[1] - q_[0]).y()) / (p_[1] - p_[0]).y();
		b[0] = q_[0].x() - M(0, 0) * p_[0].x();
		b[1] = q_[1].y() - M(1, 1) * p_[1].y();
	}
	else
	{
		Eigen::Matrix<double, Dynamic, 3> X;
		Eigen::Matrix<double, Dynamic, 2> Y;
		Eigen::Matrix<double, 3, 2> result;
		X.resize(count, 3);
		Y.resize(count, 2);
		for (int i = 0; i < count; i++)
		{
			X(i, 0) = 1; X(i, 1) = p_[i].x(); X(i, 2) = p_[i].y();
			Y(i, 0) = q_[i].x(); Y(i, 1) = q_[i].y();
		}
		result = (X.transpose() * X).inverse() * (X.transpose() * Y);

		b <<result(0,0),result(0,1);
		M.col(0) << result(1, 0), result(1, 1);
		M.col(1) << result(2, 0), result(2, 1);
	}
	//std::cout << "finish part1" << std::endl;
	//set the r
	if (count == 1)
		r.push_back(1);
	else if (count > 1)
	{
		for (int i = 0; i < count; i++)
		{
			if (i == 0)
			{
				r.push_back((p_[i], p_[1]).norm());
			}
			else
			{
				r.push_back((p_[i], p_[0]).norm());
			}
			for (int j = 1; j < count; j++)
			{
				if (j != i)
				{
					double tmp = (p_[i], p_[j]).norm();
					r[i]= tmp < r[i] ? tmp : r[i];
				}
			}
		}
	}

	//calculate coeffients a
	Eigen::Matrix<double,Dynamic, Dynamic> A;
	Eigen::Matrix<double, Dynamic, 1> b1,b2;
	A.resize(count, count);
	b1.resize(count, 1);
	b2.resize(count, 1);

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			double d = (p_[i] - p_[j]).norm();
			A(i, j) = rbf(j, d);
		}
		Vector2d temp = q_[i] - (M * p_[i] + b);
		b1[i] = temp[0];
		b2[i] = temp[1];
	}

	Eigen::Matrix<double, 2, Dynamic > result_a;
	result_a.resize(2, count);

	result_a.row(0) = A.colPivHouseholderQr().solve(b1);
	result_a.row(1) = A.colPivHouseholderQr().solve(b2);

	for (int i = 0; i < count; i++)
	{
		a.push_back(result_a.col(i));
	}

}

QPoint WarpingRBF::Warp_func(QPoint point)
{
	Vector2d value=Vector2d(0,0);
	Vector2d p = Vector2d(point.x(), point.y());
	value = M * p + b;
	for (int i = 0; i < p_.size(); i++)
	{
		double d = (p - p_[i]).norm();
		value += a[i] * rbf(i, d);
	}

	return QPoint(value[0],value[1]);
}

double WarpingRBF::rbf(int i,double d)
{
	return sqrt((d*d+r[i]*r[i]));
}