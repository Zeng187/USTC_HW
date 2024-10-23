#include "PoissonEdit.h"

PoissonEdit::PoissonEdit(QPoint lefttop_,QPoint rightbottom_)
{
	n = 0;
	this->lefttop_ = lefttop_;
	this->rightbottom_ = rightbottom_;

	img_s = NULL;
	img_t = NULL;
}

PoissonEdit::~PoissonEdit()
{

}

int PoissonEdit::width_s()
{
	return img_s->width();
}

int PoissonEdit::height_s()
{
	return img_s->height();
}

void PoissonEdit::Init(QImage* img_s_,Matrix<PointType, Dynamic, Dynamic>& points_type_)
{
	img_s = img_s_;
	points_type = points_type_;

	indices.resize(width_s(), height_s());
	TranslateIndices();
	ComputeA();


}

void PoissonEdit::ImageChange(QImage* img_t_, int xPos_, int yPos_)
{
	img_t = img_t_;
	xPos = xPos_;
	yPos = yPos_;
	Computeb();
	X = solver.solve(b);
	//Finish computing the new image
}

QColor PoissonEdit::GetNewRGB(QPoint q)
{
	QPoint currentPos = QPoint(xPos, yPos);
	QPoint RelativePos = lefttop_ + (q - currentPos);

	if (points_type(RelativePos.x(),RelativePos.y())==kinner)
	{
		//return QColor(255, 255, 255);
		int i = indices(RelativePos.x(), RelativePos.y());
		int r = X(i, 0), g = X(i, 1), b = X(i, 2);
		r = r >= 0 ? r : 0;
		r = r <= 255 ? r : 255;
		g = g >= 0 ? g : 0;
		g = g <= 255 ? g : 255;
		b = b >= 0 ? b : 0;
		b = b <= 255 ? b : 255;
		return QColor(r, g, b);
	}
	else
	{
		//return QColor(0, 0, 0);
		return img_t->pixelColor(q);
	}
}

void PoissonEdit::TranslateIndices()
{
	n = 0;
	for (int x = 0; x < width_s(); x++)
	{
		for (int y = 0; y < height_s(); y++)
		{
			if (points_type(x, y) == kinner)
			{
				indices(x, y) = n;
				coordinates.push_back(QPoint(x, y));
				n++;

			}
		}
	}
}


void PoissonEdit::ComputeA()
{
	A.resize(n, n);

	std::vector<Eigen::Triplet<double> > coeff;

	for (int i = 0; i < n; i++)
	{
		QPoint coor = coordinates[i];
		int x = coor.x();
		int y = coor.y();
		int c= 4;
		if (0 == x || width_s() - 1 == x)
			c--;
		if (0 == y || height_s() - 1 == y)
			c--;
		coeff.push_back(Eigen::Triplet<double>(i, i, c));

		if (x - 1 >= 0 && points_type(x - 1, y) == kinner)
			coeff.push_back(Eigen::Triplet<double>(i, indices(x - 1, y), -1));
		if (x + 1 < width_s() && points_type(x + 1, y) == kinner)
			coeff.push_back(Eigen::Triplet<double>(i, indices(x + 1, y), -1));
		if (y - 1 >= 0 && points_type(x, y - 1) == kinner)
			coeff.push_back(Eigen::Triplet<double>(i, indices(x, y - 1), -1));
		if (y + 1 < height_s() && points_type(x, y + 1) == kinner)
			coeff.push_back(Eigen::Triplet<double>(i, indices(x, y + 1), -1));
	}

	A.setFromTriplets(coeff.begin(), coeff.end());
	solver.compute(A);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
}



void PoissonEdit::Computeb()
{
	b.resize(n, 3);
	b.fill(0);

	QPoint delta[4] = { QPoint(-1,0),QPoint(1,0),QPoint(0,-1),QPoint(0,1) };
	QPoint currentPos = QPoint(xPos, yPos);

	for (int i = 0; i < n; i++)
	{
		QPoint p = coordinates[i];

		for (int j = 0; j < 4; j++)
		{
			QPoint q = p+delta[j];

			if (q.x() >= 0 && q.x()< width_s()&&q.y()>=0 && q.y()<height_s())
			{
				b(i, 0) += img_s->pixelColor(p).red() - img_s->pixelColor(q).red();
				b(i, 1) += img_s->pixelColor(p).green() - img_s->pixelColor(q).green();
				b(i, 2) += img_s->pixelColor(p).blue() - img_s->pixelColor(q).blue();

				QPoint RelativePos = currentPos + (q - lefttop_);
				if (points_type(q.x(), q.y()) == kbound)
				{
					b(i, 0) += img_t->pixelColor(RelativePos).red();
					b(i, 1) += img_t->pixelColor(RelativePos).green();
					b(i, 2) += img_t->pixelColor(RelativePos).blue();
				}
			}
		}



	}
}