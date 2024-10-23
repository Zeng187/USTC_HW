#pragma once

#include <QPainter.h>

class Style
{
public:
	Style() {};
	Style(int linwidth_, QColor qcolor_) :linwidth(linwidth_), qcolor(qcolor_) {};
	~Style() {};

	int linwidth;
	QColor qcolor;
};