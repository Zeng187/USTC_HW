#include "Warping.h"
#include <iostream>



Warping::Warping(std::vector<QLine*> warp_pairs_)
{
	Init(warp_pairs_);
}

Warping::~Warping()
{
	if (quadtree != NULL)
	{
		delete quadtree;
	}
}

void Warping::Init(std::vector<QLine*> warp_pairs_)
{
	for (auto const& pair : warp_pairs_)
	{
		bool is_repeat_ = false;
		for (int i = 0; i < p_.size(); i++)
		{
			if (p_[i].x() == pair->x1() && p_[i].y() == pair->y1())
			{
				is_repeat_ = true;
				break;
			}
		}
		if (!is_repeat_)
		{
			p_.push_back(Vector2d(pair->x1(), pair->y1()));
			q_.push_back(Vector2d(pair->x2(), pair->y2()));
		}
	}
}

QPoint Warping::Warp_func(QPoint point)
{
	return QPoint(0, 0);
}

void Warping::DoWarping(QImage* image)
{
	clock_t start = clock();

	QImage image_temp(*image);
	image_temp.fill(QColor(255, 255, 255));
	
	has_assigned.resize(image->width(), image->height());
	for (int i = 0; i < image->width(); i++)
	{
		for (int j = 0; j < image->height(); j++)
		{
			has_assigned(i, j) = false;
		}
	}

	for (int i = 0; i < image->width(); i++)
	{
		for (int j = 0; j < image->height(); j++)
		{
			QPoint new_point = Warp_func(QPoint(i, j));
			if (new_point.x() >= 0 && new_point.x() < image_temp.width() &&
				new_point.y() >= 0 && new_point.y() < image_temp.height())
			{
				new_points.push_back(new_point);
				has_assigned(new_point.x(), new_point.y())=true;
				image_temp.setPixelColor(new_point, image->pixelColor(QPoint(i, j)));
			}
		}
	}
	//std::cout << new_points.size() << std::endl;
	//std::cout << image->height() << std::endl;
	//std::cout << image->width() << std::endl;

	double cost = (double)(clock() - start) / 1000;
	std::printf("Finished in %.3lf seconds.\n", cost);

	clock_t start2 = clock();

	Fill_hole(&image_temp);

	double cost2 = (double)(clock() - start2) / 1000;
	std::printf("Fill holes finished in %.3lf seconds.\n", cost2);

	*image = image_temp;
}

void Warping::Fill_hole(QImage* image)
{
	if (new_points.size() == 0)
		return;
	//std::cout << new_points.size() << std::endl;
	int k = 5; //4+1
	quadtree = new QuadTree(Vector2d(0, 0), Vector2d(image->height(), image->width()),8);
	quadtree->AddPoints(new_points, new_points.size());

	
	for (int i = 0; i < image->width(); i++)
	{
		for (int j = 0; j < image->height(); j++)
		{
			if (has_assigned(i, j))
				continue;

			std::vector<int> pointIds = std::vector<int>();
			double boundry = 2.0;
			int findk = quadtree->FindKNearestNeighbors(Vector2d(i, j), k, pointIds,boundry);

			//std::cout << pointIds.size() << '\t' << findk << std::endl;
			if (findk > 0)
			{
				//std::cout << pointIds.size() << std::endl;
				if (new_points[pointIds[0]].x() == i && new_points[pointIds[0]].y() == j)
				{
					continue;
				}
				else
				{
					int r, g, b;
					r = g = b=0;
					for (int l = 0; l < findk; l++)
					{
						int index = pointIds[l];
						r += (image->pixelColor(new_points[index])).red();
						g += (image->pixelColor(new_points[index])).green();
						b += (image->pixelColor(new_points[index])).blue();
					}
					r = r / findk;
					g = g / findk;
					b = b / findk;
					r = std::clamp(r, 0, 255);
					g = std::clamp(g, 0, 255);
					b = std::clamp(b, 0, 255);
					image->setPixelColor(QPoint(i, j), QColor(r, g, b));
				}
				
			}
		}
	}
	
}

