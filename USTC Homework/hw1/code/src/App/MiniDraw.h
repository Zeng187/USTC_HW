#pragma once

#include <QtWidgets/QMainWindow>
#include<QtWidgets/Qmenu>
#include "viewwidget.h"
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <QLineEdit>
#include <qcombobox.h>
#include <qlistview.h>
#include "style.h"
#include "ui_MiniDraw.h"

class MiniDraw : public QMainWindow
{
    Q_OBJECT

public:
    MiniDraw(QWidget *parent = Q_NULLPTR);

    QMenu* pMenu;

    QToolBar* ptoolbar;

    viewwidget* viewwidget_;

    QAction* pAbout;
    QAction* pLine;
    QAction* pRectangle;
    QAction* pEllipse;
    QAction* pPolygon;
    QAction* pFreehand;
    QAction* pDelete;
    QAction* pChoose;
    QAction* pClear;
    QAction* pSelectColor;
    QLineEdit* pcolorblock;
    QComboBox* plinwidthchoices;

private slots:
    void selectcolor();
    void selectlinwidth(int linwidth_);

private:
    Ui::MiniDrawClass ui;
    Style style;

};
