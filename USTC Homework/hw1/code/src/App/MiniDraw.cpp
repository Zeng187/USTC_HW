#include "MiniDraw.h"

MiniDraw::MiniDraw(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    resize(1200, 800);
    style = Style(2, QColor(0, 0, 0));

    pAbout = new QAction(tr("&About"), this);
    pLine = new QAction(tr("&Line"), this);
    pRectangle = new QAction(tr("&Rectangle"), this);
    pEllipse = new QAction(tr("&Ellipse"), this);
    pPolygon = new QAction(tr("&Polygon"), this);
    pFreehand = new QAction(tr("&Freehand"), this);
    pDelete = new QAction(tr("&Delete"), this);
    pChoose = new QAction(tr("&Choose"), this);
    pClear = new QAction(tr("&Clear"), this);
    pSelectColor = new QAction(tr("&Color:"), this);

    pMenu = menuBar()->addMenu("&Figure");
    pMenu->addAction(pChoose);
    pMenu->addAction(pLine);
    pMenu->addAction(pRectangle);
    pMenu->addAction(pEllipse);
    pMenu->addAction(pPolygon);
    pMenu->addAction(pFreehand);
    pMenu->addAction(pDelete);
    pMenu->addAction(pAbout);
    pMenu->addAction(pSelectColor);

    ptoolbar = addToolBar(tr("&Tool"));
    ptoolbar->addAction(pChoose);
    ptoolbar->addAction(pDelete);
    ptoolbar->addAction(pClear);
    ptoolbar->addAction(pAbout);
    ptoolbar->addAction(pSelectColor);

    pcolorblock = new QLineEdit();
    pcolorblock->setFixedWidth(15);
    pcolorblock->setFixedHeight(15);
    pcolorblock->setStyleSheet(QString::fromStdString("background-color:rgb(" +
        std::to_string(style.qcolor.red()) + "," + std::to_string(style.qcolor.green()) + ","
        + std::to_string(style.qcolor.blue()) + ")"));
    pcolorblock->setReadOnly(true);
    ptoolbar->addWidget(pcolorblock);

    ptoolbar->addSeparator();
    plinwidthchoices = new QComboBox();
    plinwidthchoices->setStyleSheet("QComboBox{border:1px solid gray;}"
        "QComboBox QAbstractItemView::item{height:20px;}" );
    plinwidthchoices->setFixedWidth(125);
    plinwidthchoices->setView(new QListView());
    plinwidthchoices->addItem(" 1", tr("1"));
    plinwidthchoices->addItem(" 2", tr("2"));
    plinwidthchoices->addItem(" 3", tr("3"));
    plinwidthchoices->addItem(" 4", tr("4"));
    plinwidthchoices->addItem(" 5", tr("5"));
    plinwidthchoices->addItem(" 6", tr("6"));
    plinwidthchoices->addItem(" 7", tr("7"));
    plinwidthchoices->addItem(" 8", tr("8"));
    plinwidthchoices->addItem(" 9", tr("9"));
    plinwidthchoices->setItemText(0, " 1 ");
    plinwidthchoices->setEditable(true);
    plinwidthchoices->setCurrentIndex(1);
    plinwidthchoices->setCurrentText(QString::fromStdString("linewidth: "+std::to_string(plinwidthchoices->currentIndex()+1)));
    //plinwidthchoices->setEditable(false);
    ptoolbar->addWidget(plinwidthchoices);


    this->addToolBarBreak(Qt::TopToolBarArea);
    ptoolbar = addToolBar(tr("&Shape"));
    ptoolbar->addAction(pLine);
    ptoolbar->addAction(pRectangle);
    ptoolbar->addAction(pEllipse);
    ptoolbar->addAction(pPolygon);
    ptoolbar->addAction(pFreehand);


    viewwidget_ = new viewwidget();
    setCentralWidget(viewwidget_);

    connect(pLine, &QAction::triggered, viewwidget_, &viewwidget::set_figure_type_to_line);
    connect(pRectangle, &QAction::triggered, viewwidget_, &viewwidget::set_figure_type_to_Rect);
    connect(pEllipse, &QAction::triggered, viewwidget_, &viewwidget::set_figure_type_to_Ellipse);
    connect(pPolygon, &QAction::triggered, viewwidget_, &viewwidget::set_figure_type_to_Polygon);
    connect(pFreehand, &QAction::triggered, viewwidget_, &viewwidget::set_figure_type_to_Freehand);
    connect(pDelete, &QAction::triggered, viewwidget_, &viewwidget::delete_figure);
    connect(pChoose, &QAction::triggered, viewwidget_, &viewwidget::choose_figures);
    connect(pAbout, &QAction::triggered, viewwidget_, &viewwidget::choose_about);
    connect(pClear, &QAction::triggered, viewwidget_, &viewwidget::clearall);
    connect(pSelectColor, &QAction::triggered, this, &MiniDraw::selectcolor);
    connect(plinwidthchoices, SIGNAL(currentIndexChanged(int)),this, SLOT(selectlinwidth(int)));


}


void MiniDraw::selectcolor()
{
    style.qcolor = QColorDialog::getColor(style.qcolor, this);
    viewwidget_->SetStyle(style);

    pcolorblock->setStyleSheet(QString::fromStdString("background-color:rgb(" +
        std::to_string(style.qcolor.red()) + "," + std::to_string(style.qcolor.green()) + ","
        + std::to_string(style.qcolor.blue()) + ")"));
}

void MiniDraw::selectlinwidth(int index)
{
    int linwidth_ = index + 1;
    //QMessageBox::information(NULL, "lalal", QString::fromStdString(std::to_string(linwidth_)),QMessageBox::Yes);
    style.linwidth = linwidth_;
    viewwidget_->SetStyle(style);

    plinwidthchoices->setCurrentText(QString::fromStdString("linewidth: " + std::to_string(plinwidthchoices->currentIndex() + 1)));
}

