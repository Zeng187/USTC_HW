#pragma once
#include <QWidget>
#include <vector>
#include "Warping.h"
#include "WarpingIDW.h"
#include "WarpingRBF.h"

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

protected:
	void paintEvent(QPaintEvent *paintevent);

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror_H();		                                        // Mirror image horizontally
	void Mirror_V();		                                        // Mirror image vertically 
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin
	void SetWarp();												//Set the warp points
	void ClearWarp();											//Clear all warp points
	void DoWarpingIDW();											//use the IDW method to warp the image
	void DoWarpingRBF();											//use the RBF method to warp the image


private:
	QImage		*ptr_image_;				// image 
	QImage		*ptr_image_backup_;

	bool		warp_status_;
	bool		draw_status_;
	QPoint		start_point_;
	QPoint		end_point_;
	QLine		*current_warp_;
	std::vector<QLine*> warp_pairs_;

	int			left();
	int			top();
	void		drawline(QLine line);
	void		release();

	Warping* pwarping;
	void		DoWarping(int type);

};

