#pragma once
#include <QtWidgets> 
#include <iostream>
#include <QImage>
#include <Eigen/Core>
#include <Eigen/SparseCholesky>
#include "enumstructure.h"
#include "scanline.h"

using namespace Eigen;



class PoissonEdit
{
public:
	PoissonEdit(QPoint,QPoint);
	~PoissonEdit();
	void Init(QImage* img_s_, Matrix<PointType, Dynamic, Dynamic> &point_type_);
	void ImageChange(QImage* img_t_, int xPos, int yPos);
	QColor GetNewRGB(QPoint p);


private:
	SparseMatrix<double> A;								//the left side of linear system
	MatrixX3d b;										//the right side of linear system
	MatrixX3d X;										//the solution of the linear system
	int n;												//the number of varibles of system
	SimplicialLLT<SparseMatrix<double> > solver;		//the solver of system

	QImage	*img_s;										//the pointer to the sourcce image
	QImage* img_t;										//the pointer to the target image
	int xPos, yPos;										//the x,y position at the target image now
	Matrix<PointType, Dynamic, Dynamic> points_type;	//a matrix to record the type of points in the source image
	Matrix<int, Dynamic, Dynamic> indices;				//a matrix to record the index at (x,y) if it is an inner point
	std::vector<QPoint> coordinates;					//a vector to record the coordinates of the point at index i
	QPoint lefttop_, rightbottom_;						//the bounding box of the choosing area

	int width_s();										//the width of the source image
	int height_s();										//the height of the source image
	void TranslateIndices();							//translate the coordinates into indices
	void ComputeA();									//compute the matrix A
	void Computeb();									//compute the matrix b

};