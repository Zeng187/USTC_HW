#include "viewwidget.h"
#include <qmessagebox.h>

viewwidget::viewwidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setStyleSheet("background-color: rgb(255,255, 255)");

	draw_status_ = false;
	pos_visible = false;
	choose_mod = false;
	selecting = false;

	current_figure_ = nullptr;

	figure_type_ = kDefault;

	style = Style(2, QColor(0, 0, 0));
}

viewwidget::~viewwidget()
{
	choose_figure_array_.clear();

	for (size_t i = 0; i < figure_array_.size(); i++)
	{
		if (figure_array_[i])
		{
			delete figure_array_[i];
			figure_array_[i] = nullptr;
		}
	}

}

void viewwidget::mousePressEvent(QMouseEvent* event)
{
	pos_visible = true;

	start_point_ = end_point_ = end_point0_ = event->pos();

	if (Qt::LeftButton == event->button() && draw_status_==false && !choose_mod)
	{

		switch (figure_type_)
		{
		case kDefault:
			draw_status_ = false;
			current_figure_ = nullptr;
			break;

		case kLine:
			draw_status_ = true;
			current_figure_ = new CLine(start_point_.rx(), start_point_.ry(), end_point_.rx()+1, end_point_.ry()+1,style);
			break;

		case kRect:
			draw_status_ = true;
			current_figure_ = new CRectangle(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry(), style);
			break;

		case kEllipse:
			draw_status_ = true;
			current_figure_ = new CEllipse(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry(), style);
			break;

		case kPolygon:
			draw_status_ = true;
			current_figure_ = new CPolygon(start_point_,end_point_+QPoint(1,1), style);
			setMouseTracking(true);
			break;

		case kFreehand:
			draw_status_ = true;
			current_figure_ = new Freehand(start_point_, style);
			break;

		default:
			break;
		}

	}
	else if (Qt::LeftButton == event->button() && draw_status_ && figure_type_ == kPolygon && current_figure_ != nullptr && !choose_mod)
	{

		CPolygon* pPolygon = (CPolygon*)current_figure_;
		pPolygon->ModifyPoint(end_point_);
		pPolygon->AddPoint(end_point_ + QPoint(1, 1));

	}
	else if (Qt::RightButton == event->button() && draw_status_ && figure_type_==kPolygon && current_figure_!=nullptr && !choose_mod)
	{
		CPolygon* pPolygon = (CPolygon*)current_figure_;
		pPolygon->ModifyPoint(end_point_);


		figure_array_.push_back(current_figure_);
		current_figure_ = NULL;

		draw_status_ = false;
		setMouseTracking(false);
	}
	else if (Qt::LeftButton == event->button() && choose_mod)
	{
		current_figure_ = new CSelectRect(start_point_.rx(), start_point_.ry(), end_point_.rx() , end_point_.ry() , style);

		if (choose_figure_array_.size() == 0)
		{
			for (auto& term : figure_array_)
			{
				if (term->Intersect(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry()))
				{
					term->Set_Is_Choose(true);
				}
			}
		}
		else
		{
			selecting=false;
			for (const auto& term : choose_figure_array_)
			{
				if (term->Intersect(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry()))
				{
					selecting = true;
					break;
				}
			}

			if (!selecting)
			{
				choose_figure_array_.clear();

				for (auto& term : figure_array_)
				{
					if (term->Intersect(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry()))
					{
						term->Set_Is_Choose(true);
					}
				}
			}
		}


	}
}

void viewwidget::mouseMoveEvent(QMouseEvent* event)
{
	end_point_ = event->pos();

	if (draw_status_ && current_figure_!= nullptr && !choose_mod)
	{
		if(figure_type_ != kPolygon && figure_type_!=kFreehand)
			current_figure_->SetEndPoint(end_point_);
		else if (figure_type_ == kFreehand)
		{
			Freehand* freehand = (Freehand*)current_figure_;
			freehand->AddPoint(end_point_);
		}
		else
		{
			CPolygon* pPolygon = (CPolygon*)current_figure_;
			pPolygon->ModifyPoint(end_point_);
		}
	}
	else if (!draw_status_ && current_figure_ != nullptr && choose_mod && !selecting)
	{
		current_figure_->SetEndPoint(end_point_);

		for (auto& term : figure_array_)
		{
			if (term->Intersect(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry()))
			{
				term->Set_Is_Choose(true);
			}
			else
			{
				term->Set_Is_Choose(false);
			}
		}
	}
	else if (selecting)
	{
		for (const auto& term : choose_figure_array_)
		{
			term->Move((end_point_ - end_point0_).x(), (end_point_ - end_point0_).y());
		}
	}

	end_point0_ = event->pos();
}

void viewwidget::mouseReleaseEvent(QMouseEvent* event)
{
	//Figure* current_figure_=NULL;
	pos_visible = false;

	end_point_ = event->pos();

	if (draw_status_ &&current_figure_ != nullptr && !choose_mod)
	{
		if (figure_type_ != kPolygon)
		{
			current_figure_->SetEndPoint(end_point_);

			figure_array_.push_back(current_figure_);
			current_figure_ = nullptr;

			draw_status_ = false;
		}
		else
		{
			CPolygon* pPolygon = (CPolygon*)current_figure_;
			pPolygon->ModifyPoint(end_point_);
			pPolygon->AddPoint(end_point_ + QPoint(1, 1));
		}

	}
	else if (!draw_status_ && current_figure_ != nullptr && choose_mod && !selecting)
	{
		for (auto& term : figure_array_)
		{
			if (term->Intersect(start_point_.rx(), start_point_.ry(), end_point_.rx(), end_point_.ry()))
			{
				choose_figure_array_.push_back(term);
			}
			//term->Set_Is_Choose(false);
		}

		delete current_figure_;
		current_figure_ = nullptr;
	}
	else
	{
		selecting = false;
		delete current_figure_;
		current_figure_ = nullptr;
	}


}

void viewwidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	QPen pen;

	painter.begin(this);

	for (size_t i = 0; i < figure_array_.size(); i++)
	{
		figure_array_[i]->Draw(painter);
	}

	if (current_figure_ != nullptr)
	{
		current_figure_->Draw(painter);
	}


	if (pos_visible)
	{
		QRect rec = QRect(end_point_.x() + 30, end_point_.y() + 10, 80, 25);
		painter.setPen(QPen(QColor(20, 20, 140), 2.));
		painter.setBrush(QBrush(Qt::NoBrush));
		painter.drawRect(rec);
		painter.fillRect(rec, QColor(0, 255, 255));
		painter.setPen(QPen(QColor(0, 0, 0), 3.5));
		painter.drawText(rec, QString::number(end_point_.x()) + "," + QString::number(end_point_.y()));
	}

	painter.end();

	update();

}

void viewwidget::set_figure_type_to_line()
{
	choose_mod = false;

	figure_type_ = kLine;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::set_figure_type_to_Rect()
{
	choose_mod = false;

	figure_type_ = kRect;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::set_figure_type_to_Ellipse()
{
	choose_mod = false;

	figure_type_ = kEllipse;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::set_figure_type_to_Polygon()
{
	choose_mod = false;

	figure_type_ = kPolygon;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::set_figure_type_to_Freehand()
{
	choose_mod = false;

	figure_type_ = kFreehand;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::delete_figure()
{
	if (choose_figure_array_.size() > 0)
	{
		auto itr = figure_array_.begin();
		while (itr != figure_array_.end())
		{
			if ((*itr)->Get_Is_Choose())
			{
				//itr++;
				itr = figure_array_.erase(itr);
			}
			else
			{
				itr++;
			}
			//break;
		}

		while (choose_figure_array_.size() > 0)
		{
			current_figure_=choose_figure_array_[choose_figure_array_.size() - 1];

			delete current_figure_;
			current_figure_ = nullptr;
			choose_figure_array_.pop_back();
		}
	}
	else if (figure_array_.size() > 0)
	{
		current_figure_ = figure_array_[figure_array_.size() - 1];

		delete current_figure_;
		current_figure_ = nullptr;

		figure_array_.pop_back();

	}

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
	
}

void viewwidget::choose_figures()
{
	choose_mod = true;
	figure_type_ = kDefault;

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::choose_about()
{
	choose_mod = false;
	figure_type_ = kDefault;

	QString information = information.fromLocal8Bit("华南理工大学\n曾亚军");
	QMessageBox::information(NULL, "About", information, QMessageBox::Yes);

	for (auto& term : figure_array_)
	{
		term->Set_Is_Choose(false);
	}
}

void viewwidget::clearall()
{
	choose_mod = false;
	figure_type_ = kDefault;

	while (figure_array_.size() > 0)
	{
		current_figure_ = figure_array_[figure_array_.size() - 1];

		delete current_figure_;
		current_figure_ = nullptr;
		figure_array_.pop_back();
	}

	choose_figure_array_.clear();
}

void viewwidget::SetStyle(Style style)
{
	this->style = style;
}