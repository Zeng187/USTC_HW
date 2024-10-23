#include "mainwindow.h"
#include <QtWidgets>
#include <QImage>
#include <QPainter>
#include "ImageWidget.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	setGeometry(300, 150, 800, 450);

	imagewidget_ = new ImageWidget();
	setCentralWidget(imagewidget_);

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *e)
{

}

void MainWindow::paintEvent(QPaintEvent* paintevent)
{
	
}

void MainWindow::CreateActions()
{
	action_new_ = new QAction(QIcon(":/MainWindow/Resources/images/new.jpg"), tr("&New"), this);
	action_new_->setShortcut(QKeySequence::New);
	action_new_->setStatusTip(tr("Create a new file"));
	// connect ...

	action_open_ = new QAction(QIcon(":/MainWindow/Resources/images/open.jpg"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, &QAction::triggered, imagewidget_, &ImageWidget::Open);

	action_save_ = new QAction(QIcon(":/MainWindow/Resources/images/save.jpg"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
	connect(action_save_, &QAction::triggered, imagewidget_, &ImageWidget::Save);

	action_saveas_ = new QAction(tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, &QAction::triggered, imagewidget_, &ImageWidget::SaveAs);

	action_invert_ = new QAction(tr("Inverse"), this);
	action_invert_->setStatusTip(tr("Invert all pixel value in the image"));
	connect(action_invert_, &QAction::triggered, imagewidget_, &ImageWidget::Invert);

	action_mirror_H = new QAction(tr("Mirror_H"), this);
	action_mirror_H->setStatusTip(tr("Mirror image horizontally"));
	// The slot requires more arguments than the signal provides.
	connect(action_mirror_H, &QAction::triggered, imagewidget_, &ImageWidget::Mirror_H);

	action_mirror_V = new QAction(tr("Mirror"), this);
	action_mirror_V->setStatusTip(tr("Mirror image vertically"));
	// The slot requires more arguments than the signal provides.
	connect(action_mirror_V, &QAction::triggered, imagewidget_, &ImageWidget::Mirror_V);

	/*connect(action_mirror_, &QAction::triggered, [this]() {
		imagewidget_->Mirror(); // use default arguments
		});*/

	action_gray_ = new QAction(tr("Grayscale"), this);
	action_gray_->setStatusTip(tr("Gray-scale map"));
	connect(action_gray_, &QAction::triggered, imagewidget_, &ImageWidget::TurnGray);

	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, &QAction::triggered, imagewidget_, &ImageWidget::Restore);

	action_IDW_ = new QAction(tr("IDW"), this);
	action_IDW_->setStatusTip(tr("Inverse distance-weighted interpolation method"));
	connect(action_IDW_, &QAction::triggered, imagewidget_, &ImageWidget::DoWarpingIDW);

	action_RBF_ = new QAction(tr("RBF"), this);
	action_RBF_->setStatusTip(tr("Radial basis functions interpolation method"));
	connect(action_RBF_, &QAction::triggered, imagewidget_, &ImageWidget::DoWarpingRBF);

	action_setwarp_ = new QAction(tr("SetWarp"), this);
	action_setwarp_->setStatusTip(tr("Set control points."));
	connect(action_setwarp_, &QAction::triggered, imagewidget_, &ImageWidget::SetWarp);

	action_clearwarp_ = new QAction(tr("ClearWarp"), this);
	action_clearwarp_->setStatusTip(tr("Clear all control points."));
	connect(action_clearwarp_, &QAction::triggered, imagewidget_, &ImageWidget::ClearWarp);
}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	menu_file_->addAction(action_new_);
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);

	menu_edit_ = menuBar()->addMenu(tr("&Edit"));
	menu_edit_->setStatusTip(tr("Edit menu"));
	menu_edit_->addAction(action_invert_);
	//menu_edit_->addAction(action_mirror_H);
	menu_edit_->addAction(action_mirror_V);
	menu_edit_->addAction(action_gray_);
	menu_edit_->addAction(action_restore_);
	menu_edit_->addAction(action_setwarp_);
	menu_edit_->addAction(action_IDW_);
	menu_edit_->addAction(action_RBF_);
	menu_edit_->addAction(action_clearwarp_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	toolbar_file_->addAction(action_new_);
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);

	// Add separator in toolbar 
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_invert_);
	//toolbar_file_->addAction(action_mirror_H);
	toolbar_file_->addAction(action_mirror_V);
	toolbar_file_->addAction(action_gray_);
	toolbar_file_->addAction(action_restore_);
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_setwarp_);
	toolbar_file_->addAction(action_IDW_);
	toolbar_file_->addAction(action_RBF_);
	toolbar_file_->addAction(action_clearwarp_);
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}
