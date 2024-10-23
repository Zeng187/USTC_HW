#pragma once

#include "style.h"

class Figure
{
public:

	Figure() {};
	virtual ~Figure() {};

	void virtual Draw(QPainter& paint) {};
	bool IntervalOverlap(double x1, double x2, double x3, double x4);
	bool virtual Intersect(int start_x, int start_y, int end_x, int end_y) { return false; };
	void virtual Move(int move_x, int move_y) {};

	void SetStartPoint(const QPoint& start_point_);
	//void GetStartPoint(const QPoint& start_point_);
	void SetEndPoint(const QPoint& end_point_);
	//void GetEndPoint()

	void Set_Is_Choose(bool is_choose_);
	bool Get_Is_Choose() { return is_choose_; };

	void SetStyle(int linwidth_, QColor qcolor_);
	int GetLinwidth();
	QColor GetQColor();



protected:
	int	start_point_x, start_point_y, end_point_x, end_point_y;
	Style style;
	bool is_choose_;
};



class CLine:public Figure
{
public:
	CLine(void) {};
	CLine(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_,Style style_);
	~CLine(void) {};

	void Draw(QPainter& paint);
	bool Intersect(int start_x, int start_y, int end_x, int end_y);
	void Move(int move_x, int move_y) ;

};

class CEllipse :public Figure
{
public:
	CEllipse(void) {};
	CEllipse(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_);
	~CEllipse(void) {};

	void Draw(QPainter& paint);
	bool Intersect(int start_x, int start_y, int end_x, int end_y);
	void Move(int move_x, int move_y);
};

class CRectangle :public Figure
{
public:
	CRectangle(void) {};
	CRectangle(int start_point_x_, int start_point_y_, int end_point_x_, int end_point_y_, Style style_);
	~CRectangle(void) {};

	void Draw(QPainter& paint);
	bool Intersect(int start_x, int start_y, int end_x, int end_y);
	void Move(int move_x, int move_y) ;
};

class CPolygon : public Figure
{
public:
	CPolygon(void) {};

	CPolygon(QPoint start_point_, QPoint end_point_, Style style_);
	~CPolygon(void) {};

	void AddPoint(QPoint point);
	void ModifyPoint(QPoint point);//只允许修改栈顶元素

	void Draw(QPainter& paint);
	bool Intersect(int start_x, int start_y, int end_x, int end_y);
	void Move(int move_x, int move_y) ;

private:
	std::vector<QPoint> vertices;

};

class Freehand : public Figure
{
public:
	Freehand(void) {};

	Freehand(QPoint start_point, Style style_);
	~Freehand(void) {};

	void AddPoint(QPoint point);

	void Draw(QPainter& paint);
	bool Intersect(int start_x, int start_y, int end_x, int end_y);
	void Move(int move_x, int move_y) ;

private:
	std::vector<QPoint> vertices;

};

class CSelectRect :public CRectangle
{
public:
	CSelectRect(void) {};
	CSelectRect(int start_point_x, int start_point_y, int end_point_x, int end_point_y, Style style_);
	~CSelectRect(void) {};

	void Draw(QPainter& paint);


};