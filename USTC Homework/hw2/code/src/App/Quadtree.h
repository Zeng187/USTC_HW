#pragma once
#include <vector>
#include <Eigen\dense>
#include <qpoint.h>

using namespace Eigen;

struct QuadTreeNode {
public:
	Vector2d minPoint, maxPoint, midPoint;  // ��鷶Χ������ҿ���
	bool isLeaf;  // �Ƿ�ΪҶ���
	std::vector<int>* pointIds;  // Ҷ�����������ű��ָ�룬��Ҷ���ΪNULL
	QuadTreeNode* childs[4];  // ��Ҷ�����ӽ��ָ��������ڵ��ӽ��ΪNULL
	QuadTreeNode(Vector2d minPoint, Vector2d maxPoint);
	~QuadTreeNode();
	double SquaredDistanceToPoint(Vector2d point);  // ������㵽�����������
};

class QuadTree {
public:
	QuadTree(Vector2d minPoint, Vector2d maxPoint, int maxPointCountPerNode);  //create a quadtree
	~QuadTree();
	std::vector<Vector2d> points;  // store all the points
	void AddPoint(Vector2d point);  // add a point for quadtree
	void AddPoints(std::vector<QPoint> points_, int n);  // add pointset for quadtree
	int FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int> & pointIds, std::vector<double> & squaredDistance); // find the k-NearestNeighbor
	int FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int> & pointIds); // find the k-NearestNeighbor
	int FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int>& pointIds, double boundary); // find the k-NearestNeighbor
	QuadTreeNode* root;  // the root of quadtree
private:
	int maxPointCountPerNode; // the max count for a node
	void AddPointToNode(QuadTreeNode* node, int pointId);  // add point into node
	void SplitNode(QuadTreeNode* node); // split the node
};

struct QuadTreeSearchNode {
	bool isPoint;  // whether it is a point
	int pointId;  // if it is a point, store the point index
	QuadTreeNode* node;  // if it is a point, store the pointer of node
	double squaredDistanceToTarget;  // the squaredistance
	inline bool operator < (const QuadTreeSearchNode& node) const {
		return squaredDistanceToTarget > node.squaredDistanceToTarget;
	}
	QuadTreeSearchNode(QuadTreeNode* node, double squaredDistance);
	QuadTreeSearchNode(int pointId, double squaredDistance);
};