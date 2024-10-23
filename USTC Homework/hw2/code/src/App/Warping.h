#pragma once

#include<vector>
#include <QWidget>
#include <Eigen\dense>
#include "Quadtree.h"

using namespace Eigen;

class Warping
{
public:
	Warping() {};
	Warping(std::vector<QLine*> warp_pairs_) ;
	virtual ~Warping();
	virtual QPoint Warp_func(QPoint point);		//the warping function constructed
	void Init(std::vector<QLine*> warp_pairs_);		//init the warping points, and avoid the same points
	void DoWarping(QImage* image);
	void Fill_hole(QImage* image);							//fill the hole

protected:
	std::vector<Vector2d> p_,q_;		//the initial points, and the target points
	std::vector<QPoint> new_points;	//the new points of the image
	QuadTree* quadtree;		//quadtree, to find knn
	Matrix<bool, Dynamic, Dynamic> has_assigned;
};