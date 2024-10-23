#include "ScanLine.h"
#include<iostream>



ScanLine::ScanLine()
{
}


ScanLine::~ScanLine()
{
}


void ScanLine::init(QImage* img_, QPolygon polygon, Matrix<PointType, Dynamic, Dynamic>& points_type_)
{
	this->img = img_;
	this->points_type = points_type_;
	this->width_ = img->width();
	this->height_ = img->height();

	this->polygon_ = polygon;
	points_type.resize(width_, height_);
	points_type.fill(kout);

	//set the bounding box 


	QPolygon::iterator iter = polygon_.begin();
	left_ = right_= iter->x();
	top_ = bottom_ = iter->y();

	for (iter = polygon_.begin(); iter != polygon_.end(); iter++)
	{
		if (iter->x() < left_)
			left_ = iter->x();
		if (iter->x() > right_)
			right_ = iter->x();
		if (iter->y() < top_)
			top_ = iter->y();
		if (iter->y() > bottom_)
			bottom_ = iter->y();
	}

	//std::cout << "top:" << top_ << '\t' << "bottom:" << bottom_ << std::endl;
	//std::cout << "left:" << left_ << '\t' << "right:" << right_ << std::endl;
}

void ScanLine::FindPointType()
{
	ConstructNet();
	for (int y = top_; y < bottom_; y++)
	{
		RenewAET(y);
		FindIntersects(y);
		FindPointTypeAt(y);
	}
	FindBoundary();
}

PointType ScanLine::GetPointAt(int x, int y)
{
	return points_type(x, y);
}

Matrix<PointType, Dynamic, Dynamic> ScanLine::GetPointType()
{
	return points_type;
}

void ScanLine::ConstructNet()
{
	int count = polygon_.size();
	net_.resize(bottom_ - top_ + 1);
	for (int i = 0; i < count; i++)
	{
		Edge e;
		int index;
		QPoint p=polygon_[i];
		QPoint q=polygon_[(i + 1) % count];
		//std::cout << p.x() << '\t' << p.y() << std::endl;
		if (p.y() < q.y())
		{
			index = p.y() - top_;
			e = Edge(q.y(), p.x(), (q.x() - p.x()) / double(q.y() - p.y()));
			net_[index].push_back(e);

		}
		else if (p.y() > q.y())
		{
			index = q.y() - top_;
			e = Edge(p.y(), q.x(), (q.x() - p.x()) / double(q.y() - p.y()));
			net_[index].push_back(e);
		}
		else
		{
			index = p.y() - top_;
			e = Edge(p.y(), p.x(), 1e10);
			net_[index].push_back(e);
			index = q.y() - top_;
			e = Edge(q.y(), q.x(), 1e10);
			net_[index].push_back(e);
		}
	}
}

void ScanLine::RenewAET(int y)
{
	DeleteOldEdges(y);
	TransCurrentEdges();
	InsertNewEdges(y);
	aet_.sort(CompareX);
}

void ScanLine::DeleteOldEdges(int y)
{
	std::list<Edge>::iterator iter = aet_.begin();
	while (iter != aet_.end())
	{
		if (iter->ymax < y)
		{
			iter = aet_.erase(iter);
		}
		else
			iter++;
	}
}

void ScanLine::TransCurrentEdges()
{
	std::list<Edge>::iterator iter = aet_.begin();
	while (iter != aet_.end())
	{
		iter->x += iter->dx;
		iter++;
	}
}

void ScanLine::InsertNewEdges(int y)
{
	aet_.insert(aet_.end(), net_[y - top_].begin(), net_[y - top_].end());
}


bool ScanLine::CompareX(Edge e1, Edge e2)
{
	return (e1.x < e2.x);
}

void ScanLine::FindIntersects(int y)
{

	intersects_.clear();
	if (aet_.empty())
		return;
	std::list<Edge>::iterator iter = aet_.begin(), iter2;
	while (iter != aet_.end())
	{
		iter2 = iter;
		iter2++;
		if (aet_.end() == iter2)
		{
			intersects_.push_back(iter->x);
			iter++;
		}
		else if ((iter->ymax == y) && (iter2->ymax > y))
		{
			// a case that two edges share the same vertex, and ditribute on the different sides
			intersects_.push_back(iter2->x);
			iter++; iter++;
		}
		else if ((iter->ymax > y) && (iter2->ymax == y))
		{
			// a case that two edges share the same vertex, and ditribute on the different sides
			intersects_.push_back(iter->x);
			iter++; iter++;
		}
		else if ((iter->ymax != y) && (iter2->ymax != y))
		{
			//avoid all the cases for shared vertex
			intersects_.push_back(iter->x);
			intersects_.push_back(iter2->x);
			iter++; iter++;
		}
		else
		{
			iter++;
		}


	}
	std::sort(intersects_.begin(), intersects_.end());

}

void ScanLine::FindPointTypeAt(int y)
{
	std::vector<int> innerpointindices;

	for (int i = 0; i < intersects_.size(); i++)
	{
		if (innerpointindices.empty())
			innerpointindices.push_back((int)intersects_[i]);
		else if ((int)intersects_[i] == innerpointindices.back())
		{
			innerpointindices.pop_back();
		}
		else
			innerpointindices.push_back((int)intersects_[i]);
	}


	/*std::cout << y << ":";
	for (int i = 0; i < innerpointindices.size(); i++)
	{
		std::cout << innerpointindices[i] << '\t';
	}
	std::cout << std::endl;*/

	int index = 0;
	bool status = false;
	for (int x = left_ - 1; x <= right_; x++)
	{
		if (index >= innerpointindices.size())
			break;

		if (x == innerpointindices[index])
		{
			status = !status;
			index++;
		}
		if (status)
			if (x > 0 && x < width_ && y>0 && y < height_)
			{
				points_type(x, y) = kinner;
			}
		
	}
}

void ScanLine::FindBoundary()
{
	for (int y = 0; y < height_; y++)
	{
		//std::cout << y << ":";
		for (int x = 0; x < width_; x++)
		{
			if (points_type(x, y) != kinner)
			{
				if (x + 1 < width_ && points_type(x + 1, y) == kinner)
					points_type(x, y) = kbound;
				else if (x - 1 >= 0 && points_type(x - 1, y) == kinner)
					points_type(x, y) = kbound;
				else if (y + 1 < height_ && points_type(x, y + 1) == kinner)
					points_type(x, y) = kbound;
				else if (y - 1 >= 0 && points_type(x, y - 1) == kinner)
					points_type(x, y) = kbound;
			}
			/*if (points_type(x, y) == kbound)
			{
				std::cout << x << '\t';
			}*/
		}
		//std::cout << std::endl;

	}
}

int ScanLine::gettop()
{
	return top_;
}

int ScanLine::getbottom()
{
	return bottom_;
}

int ScanLine::getright()
{
	return right_;
}

int ScanLine::getleft()
{
	return left_;
}