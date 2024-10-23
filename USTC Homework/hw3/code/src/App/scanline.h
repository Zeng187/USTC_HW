#pragma once
#include <QWidget>
#include <vector>
#include <list>
#include <Eigen/Dense>
#include "enumstructure.h"

class QPolygon;

using namespace Eigen;

// the structure for edge
class Edge
{
public:
    int ymax;
    double x;
    double dx;


    Edge() {};
    Edge(int ymax_, double x_, double dx_) :ymax(ymax_), x(x_),dx(dx_) {};
};

class ScanLine
{

public:

    ScanLine();                                 //constructot
    ~ScanLine();                                //deconstructor
    void init(QImage*, QPolygon, Matrix<PointType, Dynamic, Dynamic> &);
    void FindPointType();                       //determine the type of all the points
    PointType GetPointAt(int x, int y);         //get the type of point at (x,y)
    Matrix<PointType, Dynamic, Dynamic> GetPointType();
    int gettop();
    int getbottom();
    int getleft();
    int getright();

private:
    QImage                                  *img;
    int                                     width_, height_;
    int                                     top_, bottom_, left_, right_;    // a bounding box of the polygon area

    QPolygon                                polygon_;
    Matrix<PointType, Dynamic, Dynamic>     points_type;         //the citation of point_type in imagewidget

    std::vector<std::list<Edge>>            net_;
    std::list<Edge>					        aet_;
    std::vector<double>				        intersects_;

    void ConstructNet();                        //construct the new edege table 
    void RenewAET(int);                         //renew the active edge table
    void FindPointTypeAt(int);                  //determine the type of the points at y
    void FindBoundary();

    void DeleteOldEdges(int);                   //delete all the old edge in the aet_
    void TransCurrentEdges();                   //translate the current edge in aet_ at y_i  into y_i+1
    void InsertNewEdges(int);                   //insert new edges into aet_
    void FindIntersects(int);                   //find the intersects
    static bool CompareX(Edge e1, Edge e2);                 //set the compare method for edge in aet_
   



};
