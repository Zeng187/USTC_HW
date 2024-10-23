#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>


using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();

	warp_status_ = false;
	draw_status_ = false;
	current_warp_ = NULL;

	pwarping = NULL;
}


ImageWidget::~ImageWidget(void)
{
	release();
}

void ImageWidget::paintEvent(QPaintEvent *paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect( (width()-ptr_image_->width())/2, (height()-ptr_image_->height())/2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_); 

	QLine line = QLine();
	for (auto& pair : warp_pairs_)
	{
		line.setP1(pair->p1() + QPoint(left(), top()));
		line.setP2(pair->p2() + QPoint(left(), top()));
		drawline(line);
	}

	if (current_warp_ != NULL)
	{
		line.setP1(current_warp_->p1() + QPoint(left(), top()));
		line.setP2(current_warp_->p2() + QPoint(left(), top()));
		drawline(line);
	}


	painter.end();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}
	warp_status_ = false;
	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout<<"image size: "<<ptr_image_->width()<<' '<<ptr_image_->height()<<endl;
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

	ptr_image_->save(filename);
	warp_status_ = false;
}

void ImageWidget::Invert()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255-qRed(color), 255-qGreen(color), 255-qBlue(color)) );
		}
	}
	warp_status_ = false;
	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror_H()
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
		}
	}
	warp_status_ = false;
	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::Mirror_V()
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			ptr_image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
		}
	}
	warp_status_ = false;
	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color)+qGreen(color)+qBlue(color))/3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
		}
	}
	warp_status_ = false;
	update();
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	warp_status_ = false;
	update();
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{
	start_point_ =end_point_= event->pos();
	if (event->button() == Qt::LeftButton && warp_status_)
	{
		draw_status_ = true;
		current_warp_ = new QLine();
		current_warp_->setP1(start_point_ - QPoint(left(), top()));
		current_warp_->setP2(end_point_ - QPoint(left(), top()));

		update();
		std::cout << warp_pairs_.size()+1<<'\t'<<
			"start point:(" << start_point_.x() << "," << start_point_.y() << ")" << '\t';
	}

}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
	end_point_ = event->pos();
	if (draw_status_)
	{
		current_warp_->setP2(end_point_ - QPoint(left(), top()));
		update();
	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
	end_point_ = event->pos();
	if (draw_status_)
	{
		current_warp_->setP2(end_point_ - QPoint(left(), top()));
		draw_status_ = false;
		std::cout << "end point:(" << end_point_.x() << "," << end_point_.y() << ")" << std::endl;

		warp_pairs_.push_back(current_warp_);
		current_warp_ = NULL;

		update();
	}
	//std::cout << current_warp_.x1()<< std::endl;

}


int ImageWidget::left()
{
	if (ptr_image_ != NULL)
	{
		return (width() - ptr_image_->width()) / 2;
	}
	else
	{
		return 0;
	}
}

int ImageWidget::top()
{
	if (ptr_image_ != NULL)
	{
		return (height() - ptr_image_->height()) / 2;
	}
	else
	{
		return 0;
	}
}

void ImageWidget::drawline(QLine line)
{
	QPainter painter(this);
	QPen pen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	painter.setPen(pen);
	painter.drawLine(line.p1() - QPoint(3, 3), line.p1() + QPoint(3, 3));
	painter.drawLine(line.p1() - QPoint(3, -3), line.p1() + QPoint(3, -3));
	painter.drawLine(line.p1(), line.p2());
	pen.setColor(Qt::red);
	painter.setPen(pen);
	painter.drawLine(line.p2() - QPoint(3, 3), line.p2() + QPoint(3, 3));
	painter.drawLine(line.p2() - QPoint(3, -3), line.p2() + QPoint(3, -3));
}

void ImageWidget::release()
{
	while (warp_pairs_.size() > 0)
	{
		delete warp_pairs_[warp_pairs_.size() - 1];
		warp_pairs_.pop_back();
	}
	current_warp_ = NULL;
}

void ImageWidget::SetWarp()
{
	warp_status_ = true;
	std::cout << "set warp points:" << std::endl;
}

void ImageWidget::ClearWarp()
{
	warp_status_ = false;
	release();
	update();
	
	std::cout << "clear all warp points!" << std::endl;
}

void ImageWidget::DoWarping(int type)
{
	if (pwarping != NULL)
	{
		delete pwarping;
		pwarping = NULL;
	}

	if (ptr_image_ != NULL)
	{
		if (warp_pairs_.size() > 0)
		{
			
			if (type == 0)
			{
				pwarping = new WarpingIDW(warp_pairs_);
			}
			else
			{
				pwarping = new WarpingRBF(warp_pairs_);
			}

			pwarping->DoWarping(ptr_image_);

		}
	}
}

void ImageWidget::DoWarpingIDW()
{
	cout << "IDW method:\n";
	DoWarping(0);


	update();
}

void ImageWidget::DoWarpingRBF()
{
	cout << "RBF method:\n";
	DoWarping(1);

	update();
}