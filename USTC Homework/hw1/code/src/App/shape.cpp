
#include "shape.h"
#include <assert.h>

void Figure::SetStartPoint(const QPoint& start_point_)
{
	start_point_x = start_point_.x();
	start_point_y = start_point_.y();
}

void Figure::SetEndPoint(const QPoint& end_point_)
{
	end_point_x = end_point_.x();
	end_point_y = end_point_.y();
}

void Figure::SetStyle(int linwidth_, QColor qcolor_)
{
	style = Style(linwidth_, qcolor_);
}

int Figure::GetLinwidth()
{
	return style.linwidth;
}

QColor Figure::GetQColor()
{
	return style.qcolor;
}

void Figure::Set_Is_Choose(bool is_choose_)
{
	this->is_choose_ = is_choose_;
}

bool Figure::IntervalOverlap(double x1, double x2, double x3, double x4)
{
	double t;
	if (x3 > x4)
	{
		t = x3;
		x3 = x4;
		x4 = t;
	}

	if (x3 > x2 || x4 < x1)
		return false;
	else
		return true;
}

CLine::CLine(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_)
{
	start_point_x = start_point_x_;
	start_point_y = start_point_y_;
	end_point_x = end_point_x_;
	end_point_y = end_point_y_;

	style = style_;
	is_choose_ = false;
}

bool CLine::Intersect(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_)
{
	int rx_min, rx_max, ry_min, ry_max;
	if (start_point_x_ < end_point_x_) { rx_min = start_point_x_; rx_max = end_point_x_; }
	else { rx_max = start_point_x_; rx_min = end_point_x_; }
	if (start_point_y_ < end_point_y_) { ry_min = start_point_y_; ry_max = end_point_y_; }
	else { ry_max = start_point_y; ry_min = end_point_y; }

	int a_x, a_y, b_x, b_y;
	if (start_point_y > end_point_y) { a_y = end_point_y; a_x = end_point_x; b_y = start_point_y; b_x = start_point_x; }
	else { b_y = end_point_y; b_x = end_point_x; a_y = start_point_y; a_x = start_point_x; }
	if (a_y == b_y)	// 线段平行于x轴
	{
		if (a_y <= ry_max && a_y >= ry_min)
		{
			return IntervalOverlap(rx_min, rx_max, a_x, b_x);
		}
		else
		{
			return false;
		}
	}
	// 在线段AB上确定点C和D
	// 两点确定一条直线: (x-x1)/(x2-x1)=(y-y1)/(y2-y1)
	double k = 1.0 * (b_x - a_x) / (b_y - a_y);
	int c_x, c_y, d_x, d_y;
	if (a_y < ry_min)
	{
		c_y = ry_min;
		c_x = k * (c_y - a_y) + a_x;
	}
	else {
		c_x = a_x;
		c_y = a_y;
	}

	if (b_y > ry_max)
	{
		d_y = ry_max;
		d_x = k * (d_y - a_y) + a_x;
	}
	else {
		d_x = b_x;
		d_y = b_y;
	}

	if (d_y >= c_y)	// y维上有交集
	{
		return IntervalOverlap(rx_min, rx_max, d_x, c_x);
	}
	else
	{
		return false;
	}
}

void CLine::Draw(QPainter& paint)
{
	QPen pen;

	if (is_choose_)
	{
		pen.setWidth(style.linwidth + 2);
		pen.setBrush(QColor(160, 200, 240, 100));
		QBrush brush(QColor(160, 200, 240, 50));
		paint.setBrush(brush);
	}
	else 
	{
		pen.setWidth(style.linwidth);
		pen.setBrush(style.qcolor);
		paint.setBrush(QBrush());
	}

	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	paint.setPen(pen);

	paint.drawLine(start_point_x, start_point_y, end_point_x, end_point_y);
}

void CLine::Move(int move_x, int move_y)
{
	start_point_x += move_x;
	start_point_y += move_y;
	end_point_x += move_x;
	end_point_y += move_y;
}

CEllipse::CEllipse(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_)
{
	start_point_x = start_point_x_;
	start_point_y = start_point_y_;
	end_point_x = end_point_x_;
	end_point_y = end_point_y_;

	style = style_;
	is_choose_ = false;
}

bool CEllipse::Intersect(int start_x, int start_y, int end_x, int end_y)
{
	int rx_min, rx_max, ry_min, ry_max;
	if (start_x < end_x) { rx_min = start_x; rx_max = end_x; }
	else { rx_max = start_x; rx_min = end_x; }
	if (start_y < end_y) { ry_min = start_y; ry_max = end_y; }
	else { ry_max = start_y; ry_min = end_y; }

	int x_min, x_max, y_min, y_max;
	if (start_point_x < end_point_x) { x_min = start_point_x; x_max = end_point_x; }
	else { x_max = start_point_x; x_min = end_point_x; }
	if (start_point_y < end_point_y) { y_min = start_point_y; y_max = end_point_y; }
	else { y_max = start_point_y; y_min = end_point_y; }

	double x_cen, y_cen;
	x_cen = 1.0 * (x_max + x_min) / 2;
	y_cen = 1.0 * (y_min + y_max) / 2;

	if ((x_cen > rx_min && x_cen<rx_max && y_min>ry_min && y_min < ry_max) || (x_cen > rx_min && x_cen<rx_max && y_max>ry_min && y_max < ry_max) ||
		(x_min > rx_min && x_min<rx_max && y_cen>ry_min && y_cen < ry_max) || (x_max > rx_min && x_max<rx_max && y_cen>ry_min && y_cen < ry_max))
		return true;

	if (IntervalOverlap(rx_min, rx_max, x_max, x_min) && IntervalOverlap(ry_min, ry_max, y_min, y_max)) {
		int c_x, c_y;
		if (IntervalOverlap(rx_min, rx_max, x_cen, x_cen)) {
			if (IntervalOverlap(ry_min, ry_max, y_cen, y_cen)) return true;
			if (abs(ry_max - y_cen) < abs(ry_min - y_cen)) c_y = ry_max;
			else c_y = ry_min;
			if (2 * abs(c_y - y_cen) < y_max - y_min) return true;
		}
		else if (IntervalOverlap(ry_min, ry_max, y_cen, y_cen)) {
			if (IntervalOverlap(rx_min, rx_max, x_cen, x_cen)) return true;
			if (abs(rx_max - x_cen) < abs(rx_min - x_cen)) c_x = rx_max;
			else c_x = rx_min;
			if (2 * abs(c_x - x_cen) < x_max - x_min) return true;
		}
		else {
			if (abs(ry_max - y_cen) < abs(ry_min - y_cen)) c_y = ry_max;
			else c_y = ry_min;
			if (abs(rx_max - x_cen) < abs(rx_min - x_cen)) c_x = rx_max;
			else c_x = rx_min;

			if (((c_x - x_cen) * (c_x - x_cen)) / ((x_max - x_cen) * (x_max - x_cen)) + ((c_y - y_cen) * (c_y - y_cen)) / ((y_max - y_cen) * (y_max - y_cen)) < 1) {
				return true;
			}
		}
	}
	return false;
}

void CEllipse:: Draw(QPainter& paint)
{
	QPen pen;
	if (is_choose_)
	{
		pen.setWidth(style.linwidth + 2);
		pen.setBrush(QColor(160, 200, 240, 100));
		QBrush brush(QColor(160, 200, 240, 50));
		paint.setBrush(brush);
	}
	else
	{
		pen.setWidth(style.linwidth);
		pen.setBrush(style.qcolor);
		paint.setBrush(QBrush());
	}

	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	paint.setPen(pen);

	paint.drawEllipse(start_point_x, start_point_y, end_point_x - start_point_x, end_point_y - start_point_y);
}

void CEllipse::Move(int move_x, int move_y)
{
	start_point_x += move_x;
	start_point_y += move_y;
	end_point_x += move_x;
	end_point_y += move_y;
}

CRectangle::CRectangle(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_)
{
	start_point_x = start_point_x_;
	start_point_y = start_point_y_;
	end_point_x = end_point_x_;
	end_point_y = end_point_y_;

	style = style_;
	is_choose_ = false;
}

bool CRectangle::Intersect(int start_x, int start_y, int end_x, int end_y)
{
	int rx_min, rx_max, ry_min, ry_max;
	if (start_x < end_x) { rx_min = start_x; rx_max = end_x; }
	else { rx_max = start_x; rx_min = end_x; }
	if (start_y < end_y) { ry_min = start_y; ry_max = end_y; }
	else { ry_max = start_y; ry_min = end_y; }

	return IntervalOverlap(rx_min, rx_max, start_point_x, end_point_x) && IntervalOverlap(ry_min, ry_max, start_point_y, end_point_y);
}

void CRectangle::Draw(QPainter& paint)
{
	QPen pen;
	if (is_choose_)
	{
		pen.setWidth(style.linwidth + 2);
		pen.setBrush(QColor(160, 200, 240, 100));
		QBrush brush(QColor(160, 200, 240, 50));
		paint.setBrush(brush);
	}
	else
	{
		pen.setWidth(style.linwidth);
		pen.setBrush(style.qcolor);
		paint.setBrush(QBrush());
	}

	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	paint.setPen(pen);

	paint.drawRect(start_point_x, start_point_y, end_point_x - start_point_x, end_point_y - start_point_y);
}

void CRectangle::Move(int move_x, int move_y)
{
	start_point_x += move_x;
	start_point_y += move_y;
	end_point_x += move_x;
	end_point_y += move_y;
}

CPolygon::CPolygon(QPoint start_point_, QPoint end_point_, Style style_)
{
	vertices.push_back(start_point_);
	vertices.push_back(end_point_);

	style = style_;
	is_choose_ = false;
}

bool CPolygon::Intersect(int start_x, int start_y, int end_x, int end_y)
{
	int num = vertices.size();

	int rx_min, rx_max, ry_min, ry_max;
	if (start_x < end_x) { rx_min = start_x; rx_max = end_x; }
	else { rx_max = start_x; rx_min = end_x; }
	if (start_y < end_y) { ry_min = start_y; ry_max = end_y; }
	else { ry_max = start_y; ry_min = end_y; }

	bool ss = true;
	double rx_cen, ry_cen, s;
	rx_cen = 1.0 * (rx_min + rx_max) / 2;
	ry_cen = 1.0 * (ry_min + ry_max) / 2;
	s = (vertices[1].ry() - vertices[0].ry()) * (rx_cen - vertices[0].rx()) - (vertices[1].rx() - vertices[0].rx()) * (ry_cen - vertices[0].ry());
	for (int i = 0; i < num; i++) {
		int a_x, a_y, b_x, b_y;
		int j = (i + 1) % num;
		if (vertices[i].ry() > vertices[j].ry()) { a_y = vertices[j].ry(); a_x = vertices[j].rx(); b_y = vertices[i].ry(); b_x = vertices[i].rx(); }
		else { b_y = vertices[j].ry(); b_x = vertices[j].rx(); a_y = vertices[i].ry(); a_x = vertices[i].rx(); }

		if (s * ((vertices[j].ry() - vertices[i].ry()) * (rx_cen - vertices[i].rx()) - (vertices[j].rx() - vertices[i].rx()) * (ry_cen - vertices[i].ry())) < 0) ss = false;

		if (a_y == b_y) {
			if (a_y <= ry_max && a_y >= ry_min) {
				if (IntervalOverlap(rx_min, rx_max, a_x, b_x)) return true;
				else continue;
			}
			else continue;
		}
		// 在线段AB上确定点C和D
		// 两点确定一条直线: (x-x1)/(x2-x1)=(y-y1)/(y2-y1)
		double k = 1.0 * (b_x - a_x) / (b_y - a_y);
		int c_x, c_y, d_x, d_y;
		if (a_y < ry_min) {
			c_y = ry_min;
			c_x = k * (c_y - a_y) + a_x;
		}
		else {
			c_x = a_x;
			c_y = a_y;
		}

		if (b_y > ry_max) {
			d_y = ry_max;
			d_x = k * (d_y - a_y) + a_x;
		}
		else {
			d_x = b_x;
			d_y = b_y;
		}

		if (d_y >= c_y) {
			if (IntervalOverlap(rx_min, rx_max, d_x, c_x)) return true;
			else continue;
		}
		else continue;
	}
	if (num > 2 && ss) return true;
	return false;
}

void CPolygon::Draw(QPainter& paint)
{
	if (!vertices.empty())
	{
		QPen pen;
		if (is_choose_)
		{
			pen.setWidth(style.linwidth + 2);
			pen.setBrush(QColor(160, 200, 240, 100));
			QBrush brush(QColor(160, 200, 240, 50));
			paint.setBrush(brush);
		}
		else
		{
			pen.setWidth(style.linwidth);
			pen.setBrush(style.qcolor);
			paint.setBrush(QBrush());
		}

		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		paint.setPen(pen);

		paint.drawPolygon(&vertices[0], vertices.size());
	}
}

void CPolygon::AddPoint(QPoint point)
{
	vertices.push_back(point);
}

void CPolygon::ModifyPoint(QPoint point)
{
	assert(vertices.size() > 0);

	vertices.pop_back();
	vertices.push_back(point);
}

void CPolygon::Move(int move_x, int move_y)
{
	for (auto& v : vertices)
	{
		v.setX(v.rx() + move_x);
		v.setY(v.ry() + move_y);
	}
}

Freehand::Freehand(QPoint start_point_, Style style_)
{
	vertices.push_back(start_point_);

	style = style_;
	is_choose_ = false;
}

void Freehand::AddPoint(QPoint point)
{
	vertices.push_back(point);
}

bool Freehand::Intersect(int start_x, int start_y, int end_x, int end_y)
{
	int rx_min, rx_max, ry_min, ry_max;
	if (start_x < end_x) { rx_min = start_x; rx_max = end_x; }
	else { rx_max = start_x; rx_min = end_x; }
	if (start_y < end_y) { ry_min = start_y; ry_max = end_y; }
	else { ry_max = start_y; ry_min = end_y; }

	for (int i = 0; i < vertices.size(); i++) {
		int x = vertices[i].rx();
		int y = vertices[i].ry();
		if (IntervalOverlap(rx_min, rx_max, x, x) && IntervalOverlap(ry_min, ry_max, y, y)) return true;
	}
	return false;
}

void Freehand::Draw(QPainter& paint)
{
	if (!vertices.empty())
	{
		QPen pen;
		if (is_choose_)
		{
			pen.setWidth(style.linwidth + 2);
			pen.setBrush(QColor(160, 200, 240, 100));
			QBrush brush(QColor(160, 200, 240, 50));
			paint.setBrush(brush);
		}
		else
		{
			pen.setWidth(style.linwidth);
			pen.setBrush(style.qcolor);
			paint.setBrush(QBrush());
		}

		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		paint.setPen(pen);

		paint.drawPolyline(&vertices[0],vertices.size());
	}
}

void Freehand::Move(int move_x, int move_y)
{
	for (auto& v : vertices)
	{
		v.setX(v.rx() + move_x);
		v.setY(v.ry() + move_y);
	}
}

CSelectRect::CSelectRect(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_)
{
	start_point_x = start_point_x_;
	start_point_y = start_point_y_;
	end_point_x = end_point_x_;
	end_point_y = end_point_y_;

	style = style_;
}


void CSelectRect::Draw(QPainter& paint)
{
	QPen pen;

	pen.setWidth(1);
	pen.setBrush(Qt::black);
	pen.setCapStyle(Qt::RoundCap);
	pen.setStyle(Qt::DashLine);
	pen.setJoinStyle(Qt::RoundJoin);
	paint.setPen(pen);

	QBrush brush(QColor(160, 200, 240, 30));
	paint.setBrush(brush);

	paint.drawRect(start_point_x, start_point_y, end_point_x - start_point_x, end_point_y - start_point_y);
}