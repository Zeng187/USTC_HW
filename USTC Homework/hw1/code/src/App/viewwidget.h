#pragma once

#include <QWidget>
#include <qevent.h>
#include <qpainter.h>
#include <vector>
#include "shape.h"
#include "ui_viewwidget.h"

class viewwidget : public QWidget
{
	Q_OBJECT

public:
	viewwidget(QWidget *parent = Q_NULLPTR);
	~viewwidget();

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	void paintEvent(QPaintEvent*);
	void SetStyle(Style style);

public slots:
	void set_figure_type_to_line();
	void set_figure_type_to_Rect();
	void set_figure_type_to_Ellipse();
	void set_figure_type_to_Polygon();
	void set_figure_type_to_Freehand();
	void delete_figure();
	void choose_figures();
	void choose_about();
	void clearall();

private:
	bool draw_status_;
	bool pos_visible;

	QPoint start_point_;
	QPoint end_point_;
	QPoint end_point0_;//记录移动操作上一次的结束位置

	std::vector<Figure* >  figure_array_;
	Figure* current_figure_;

	bool choose_mod;
	bool selecting;
	std::vector<Figure* >  choose_figure_array_;

	Style style;

	Ui::viewwidget ui;

	enum Type
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse = 3,
		kPolygon = 4,
		kFreehand = 5,
	};
	Type  figure_type_;
};
