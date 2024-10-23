#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include "ChildWindow.h"

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;
	is_existing_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);
	points_type.resize(width(), height());

	rect_ = NULL;
	polygon_ = NULL;

	source_window_ = NULL;
	scanline_ = NULL;
	poissonedit_ = NULL;
}

ImageWidget::~ImageWidget(void)
{
	if (rect_ != NULL)
	{
		delete rect_;
		rect_ = NULL;
	}
	if (polygon_ != NULL)
	{
		delete polygon_;
		polygon_ = NULL;
	}
	if (scanline_ != NULL)
	{
		delete scanline_;
		scanline_ = NULL;
	}
	if (poissonedit_ != NULL)
	{
		delete poissonedit_;
		poissonedit_ = NULL;
	}
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::init()
{
	if (rect_ != NULL)
	{
		delete rect_;
		rect_ = NULL;
	}
	if (polygon_ != NULL)
	{
		delete polygon_;
		polygon_ = NULL;
	}
	if (poissonedit_ != NULL)
	{
		delete poissonedit_;
		poissonedit_ = NULL;
	}
	is_existing_ = false;
}

void ImageWidget::set_draw_status_to_choose()
{
	is_pasting_ = false;
	draw_status_ = kChoose;	
	setMouseTracking(false);
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

void ImageWidget::set_draw_shape(ShapeType type_)
{
	shape_type_ = type_;
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);


	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);

	switch (shape_type_)
	{
	case kRect:
		if (rect_ != NULL)
		{
			painter.drawRect(*rect_);
			break;
		}
	case kPoly:
	case kFree:
		if (polygon_ != NULL)
		{
			painter.drawPolygon(*polygon_);
			break;
		}
	default:
		break;
	}
	

	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		setMouseTracking(false);

		switch (draw_status_)
		{
		case kChoose:
			is_choosing_ = true;
			point_start_ = point_end_ = mouseevent->pos();

			switch (shape_type_)
			{
			case kRect:
				if (is_existing_)
					init();
				rect_ = new QRect(point_start_,point_end_);
				is_existing_ = true;
				break;

			case kPoly:
				if (is_existing_)
				{
					// a complete shape has existed, make sure only one shape in the imagewidget
					init();
				}
				if (polygon_ == NULL)
				{
					//while the pointer polygon_ is NULL, we need to add the first point into it
					polygon_ = new QPolygon();
					polygon_->push_back(point_start_);
					polygon_->push_back(point_end_);
				}
				else
				{
					polygon_->push_back(point_end_);
				}
				setMouseTracking(true);
				break;

			case kFree:
				if (is_existing_)
				{
					// a complete shape has existed, make sure only one shape in the imagewidget
					init();
				}
				if (polygon_ == NULL)
				{
					//while the pointer polygon_ is NULL, we need to add the first point into it
					polygon_ = new QPolygon();
				}
				polygon_->push_back(point_start_);
				break;
			default:
				break;
			}
			break;

		case kPaste:
		{
			is_pasting_ = true;
			is_choosing_ = false;

			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			int h, w, left, top;

			switch (source_window_->imagewidget_->shape_type_)
			{
			case kRect:

				// Width and Height of rectangle region
				w = source_window_->imagewidget_->rect_->width();
				h = source_window_->imagewidget_->rect_->height();

				left = source_window_->imagewidget_->rect_->topLeft().x();
				top = source_window_->imagewidget_->rect_->topLeft().y();

				if (source_window_->imagewidget_->poissonedit_ != NULL)
				{
					if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
					{
						//it requests that the target image can hold the choosing part of source image
						source_window_->imagewidget_->poissonedit_->ImageChange(image_, xpos, ypos);

						for (int i = 0; i < w; i++)
						{
							for (int j = 0; j < h; j++)
							{
								QPoint q = QPoint(xpos + i, ypos + j);
								image_->setPixelColor(q, source_window_->imagewidget_->poissonedit_->GetNewRGB(q));
							}
						}

						//std::cout << "Set new finish!" << std::endl;
					}
				}
				break;

			case kPoly:
			case kFree:

				// Width and Height of rectangle region
				w = source_window_->imagewidget_->scanline_->getright() - source_window_->imagewidget_->scanline_->getleft() + 1;
				h = source_window_->imagewidget_->scanline_->getbottom() - source_window_->imagewidget_->scanline_->gettop() + 1;

				left = source_window_->imagewidget_->scanline_->getleft();
				top = source_window_->imagewidget_->scanline_->gettop();
				
				if (source_window_->imagewidget_->poissonedit_ != NULL)
				{
					if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
					{
						//it requests that the target image can hold the choosing part of source image
						source_window_->imagewidget_->poissonedit_->ImageChange(image_, xpos, ypos);

						for (int i = 0; i < w; i++)
						{
							for (int j = 0; j < h; j++)
							{   
								QPoint q = QPoint(xpos + i, ypos + j);
								image_->setPixelColor(q, source_window_->imagewidget_->poissonedit_->GetNewRGB(q));
							}
						}

						//std::cout << "Set new finish!" << std::endl;
					}
				}
				break;


			}




			// Paste
			
			{
				// Restore image
			//	*(image_) = *(image_backup_);


				// Paste

			}
		}

		update();
		break;

		default:
			break;
		}
	}
	else if (mouseevent->button() == Qt::RightButton)
	{
		point_end_ = mouseevent->pos();
		setMouseTracking(false);

		if (draw_status_ == kChoose && is_choosing_ && shape_type_ == kPoly)
		{
			polygon_->pop_back();
			polygon_->push_back(point_end_);
			is_choosing_ = false;
			draw_status_ = kNone;
			is_existing_ = true;

			scanline_ = new ScanLine();
			scanline_->init(image_,*polygon_,points_type);
			scanline_->FindPointType();
			points_type = scanline_->GetPointType();

			poissonedit_ = new PoissonEdit(QPoint(scanline_->getleft(), scanline_->gettop()),
				QPoint(scanline_->getright(), scanline_->getbottom()));
			poissonedit_->Init(image_, points_type);

			std::cout << "Poisson Init finish!" << std::endl;
			

		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		// Store point position for rectangle region
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();

			switch (shape_type_)
			{
			case kRect:
				rect_->setBottomRight(point_end_);
				break;

			case kPoly:
				polygon_->pop_back();
				polygon_->push_back(point_end_);
				break;

			case kFree:
				polygon_->push_back(point_end_);

			default:
				break;
			}
		}
		break;

	case kPaste:
		// Paste rectangle region to object image
		if (is_pasting_)
		{
			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			int h, w, left, top;

			switch (source_window_->imagewidget_->shape_type_)
			{
			case kRect:

				// Width and Height of rectangle region
				w = source_window_->imagewidget_->rect_->width();
				h = source_window_->imagewidget_->rect_->height();

				left = source_window_->imagewidget_->rect_->topLeft().x();
				top = source_window_->imagewidget_->rect_->topLeft().y();

				if (source_window_->imagewidget_->poissonedit_ != NULL)
				{
					if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
					{
						//it requests that the target image can hold the choosing part of source image
						source_window_->imagewidget_->poissonedit_->ImageChange(image_, xpos, ypos);

						*(image_) = *(image_backup_);

						for (int i = 0; i < w; i++)
						{
							for (int j = 0; j < h; j++)
							{
								QPoint q = QPoint(xpos + i, ypos + j);
								image_->setPixelColor(q, source_window_->imagewidget_->poissonedit_->GetNewRGB(q));
							}
						}

						//std::cout << "Set new finish!" << std::endl;
					}
				}
				break;

			case kPoly:
			case kFree:

				// Width and Height of rectangle region
				w = source_window_->imagewidget_->scanline_->getright() - source_window_->imagewidget_->scanline_->getleft() + 1;
				h = source_window_->imagewidget_->scanline_->getbottom() - source_window_->imagewidget_->scanline_->gettop() + 1;

				left = source_window_->imagewidget_->scanline_->getleft();
				top = source_window_->imagewidget_->scanline_->gettop();

				if (source_window_->imagewidget_->poissonedit_ != NULL)
				{
					if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
					{
						//it requests that the target image can hold the choosing part of source image
						source_window_->imagewidget_->poissonedit_->ImageChange(image_, xpos, ypos);

						for (int i = 0; i < w; i++)
						{
							for (int j = 0; j < h; j++)
							{
								QPoint q = QPoint(xpos + i, ypos + j);
								image_->setPixelColor(q, source_window_->imagewidget_->poissonedit_->GetNewRGB(q));
							}
						}

						//std::cout << "Set new finish!" << std::endl;
					}
				}
				break;


			}
		}

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();

			switch (shape_type_)
			{
			case kRect:
				rect_->setBottomRight(point_end_);
				is_choosing_ = false;
				draw_status_ = kNone;

				FindRectInnerAndBound();
				poissonedit_ = new PoissonEdit(rect_->topLeft(), rect_->bottomRight());
				poissonedit_->Init(image_, points_type);

				break;

			case kPoly:
				polygon_->pop_back();
				polygon_->push_back(point_end_);
				break;

			case kFree:
				polygon_->push_back(point_end_);
				is_choosing_ = false;
				draw_status_ = kNone;
				is_existing_ = true;

				scanline_ = new ScanLine();
				scanline_->init(image_, *polygon_, points_type);
				scanline_->FindPointType();
				points_type = scanline_->GetPointType();

				poissonedit_ = new PoissonEdit(QPoint(scanline_->getleft(), scanline_->gettop()),
					QPoint(scanline_->getright(), scanline_->getbottom()));
				poissonedit_->Init(image_, points_type);


			default:
				break;
			}

		}

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
		}

	default:
		break;
	}

	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);
	}

	//	setFixedSize(image_->width(), image_->height());
	//	relate_window_->setWindowFlags(Qt::Dialog);
	//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
	//	relate_window_->setWindowFlags(Qt::SubWindow);

		//image_->invertPixels(QImage::InvertRgb);
		//*(image_) = image_->mirrored(true, true);
		//*(image_) = image_->rgbSwapped();
	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	update();
}

void ImageWidget::FindRectInnerAndBound()
{
	points_type.fill(kout);
	for (int y = rect_->topLeft().y(); y <= rect_->bottomRight().y(); y++)
	{
		for (int x = rect_->topLeft().x(); x <= rect_->bottomRight().x(); x++)
		{
			if (y == rect_->topLeft().y() || y == rect_->bottomRight().y() ||
				x == rect_->topLeft().x() || x == rect_->bottomRight().x())
			{
				points_type(x, y) = kbound;
			}
			else
			{
				points_type(x, y) = kinner;
			}
		}
	}
}

