#include "Quadtree.h"
#include <queue>
#include <iostream>

QuadTreeNode::QuadTreeNode(Vector2d minPoint, Vector2d maxPoint) 
{
	this->minPoint = minPoint;
	this->maxPoint = maxPoint;
	midPoint = (minPoint + maxPoint) / 2;
	isLeaf = true;
	pointIds = new std::vector<int>;
	for (int i = 0; i < 4; i++) {
		childs[i] = NULL;
	}
}

QuadTreeNode::~QuadTreeNode() 
{
	if (isLeaf) {
		delete pointIds;
	}
	else {
		for (int i = 0; i < 4; i++) {
			if (childs[i] != NULL) {
				delete childs[i];
			}
		}
	}
}

double QuadTreeNode::SquaredDistanceToPoint(Vector2d point) 
{
	Vector2d differVec;  // �㵽����������ľ�������
	if (point.x() < minPoint.x()) { differVec.x() = minPoint.x() - point.x(); }
	else if (point.x() > maxPoint.x()) { differVec.x() = point.x() - maxPoint.x(); }
	else { differVec.x() = 0; }
	if (point.y() < minPoint.y()) { differVec.y() = minPoint.y() - point.y(); }
	else if (point.y() > maxPoint.y()) { differVec.y() = point.y() - maxPoint.y(); }
	else { differVec.y() = 0; }
	return differVec.squaredNorm();
}


QuadTree::QuadTree(Vector2d minPoint, Vector2d maxPoint, int maxPointCountPerNode) 
{
	root = new QuadTreeNode(minPoint, maxPoint);
	this->maxPointCountPerNode = maxPointCountPerNode;
	//std::cout << maxPointCountPerNode << std::endl;
}

QuadTree::~QuadTree() 
{
	delete root;
}

void QuadTree::AddPoint(Vector2d point) 
{
	points.push_back(point);
	AddPointToNode(root, points.size() - 1);
}

void QuadTree::AddPoints(std::vector<QPoint> points_, int n) 
{
	for (int i = 0; i < n; i++) {
		AddPoint(Vector2d(points_[i].x(),points_[i].y()));
	}
}

void QuadTree::AddPointToNode(QuadTreeNode* node, int pointId) {
	if (node->isLeaf) {
		node->pointIds->push_back(pointId);
		if (node->pointIds->size() > maxPointCountPerNode) {  // �����������������������ʱ���з���
			SplitNode(node);
		}
	}
	else {
		// �ȼ���������Ӧ�������id��id��0��ʼ��˳��Ϊxyz����С����
		int subNodeId = 0;
		if (points[pointId].x() > node->midPoint.x()) { subNodeId += 1; }
		if (points[pointId].y() > node->midPoint.y()) { subNodeId += 2; }
		// ��Ӧ����鲻����ʱ�ȴ���
		if (node->childs[subNodeId] == NULL) {
			Vector2d subNodeMinPoint = node->minPoint;
			Vector2d subNodeMaxPoint = node->maxPoint;
			if (points[pointId].x() > node->midPoint.x()) { subNodeMinPoint.x() = node->midPoint.x(); }
			else { subNodeMaxPoint.x() = node->midPoint.x(); }
			if (points[pointId].y() > node->midPoint.y()) { subNodeMinPoint.y() = node->midPoint.y(); }
			else { subNodeMaxPoint.y() = node->midPoint.y(); }
			node->childs[subNodeId] = new QuadTreeNode(subNodeMinPoint, subNodeMaxPoint);
		}
		AddPointToNode(node->childs[subNodeId], pointId);  // �������ݹ���������
	}
}

void QuadTree::SplitNode(QuadTreeNode* node) {
	node->isLeaf = false;  // ���ý��Ϊ��Ҷ���
	for (int i = 0; i < node->pointIds->size(); i++) {
		AddPointToNode(node, (*(node->pointIds))[i]);
	}
	delete node->pointIds;  // �ͷ����������Ŀռ�
	//std::cout << "split" << std::endl;
}

int QuadTree::FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int>& pointIds, std::vector<double> & squaredDistance) {
	int count = 0;
	std::priority_queue<QuadTreeSearchNode> heap;  // �������ȶ��н���k��������
	heap.push(QuadTreeSearchNode(root, 0));  // ��Ӱ˲������ڵ�
	while (!heap.empty() && count < k) {
		QuadTreeSearchNode node = heap.top();
		heap.pop();
		if (node.isPoint) {  // ��ǰ����������Ϊһ�㣬��������
			pointIds[count] = node.pointId;
			squaredDistance[count] = sqrt(node.squaredDistanceToTarget);
			count++;
		}
		else {  // ��ǰ����������Ϊһ��飬��Ⲣ�������
			if (node.node->isLeaf) {  // ���ΪҶ���ʱ����������������
				for (int i = 0; i < node.node->pointIds->size(); i++) {
					int id = (*(node.node->pointIds))[i];
					double squaredDistance = (points[id] - targetPoint).squaredNorm();
					heap.push(QuadTreeSearchNode(id, squaredDistance));
				}
			}
			else {  // ���Ϊ��Ҷ���ʱ����������������
				for (int i = 0; i < 4; i++) {
					if (node.node->childs[i] == NULL) { continue; }
					QuadTreeNode* subNode = node.node->childs[i];
					double squaredDistance = subNode->SquaredDistanceToPoint(targetPoint);
					heap.push(QuadTreeSearchNode(subNode, squaredDistance));
				}
			}
		}
	}
	return count;
}

int QuadTree::FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int> & pointIds) 
{

	int count = 0;
	std::priority_queue<QuadTreeSearchNode> heap;  // �������ȶ��н���k��������
	heap.push(QuadTreeSearchNode(root, 0));  // ��Ӱ˲������ڵ�
	while (!heap.empty() && count < k) {
		QuadTreeSearchNode node = heap.top();
		heap.pop();
		if (node.isPoint) 
		{  // ��ǰ����������Ϊһ�㣬��������
			//std::cout << node.pointId << std::endl;
			pointIds.push_back(node.pointId);
			count++;
		}
		else {  // ��ǰ����������Ϊһ��飬��Ⲣ�������
			if (node.node->isLeaf) {  // ���ΪҶ���ʱ����������������
				for (int i = 0; i < node.node->pointIds->size(); i++) {
					int id = (*(node.node->pointIds))[i];
					double squaredDistance = (points[id] - targetPoint).squaredNorm();
					//std::cout << "leaf"<<squaredDistance << std::endl;
					heap.push(QuadTreeSearchNode(id, squaredDistance));
				}
			}
			else {  // ���Ϊ��Ҷ���ʱ����������������
				for (int i = 0; i < 4; i++) {
					if (node.node->childs[i] == NULL) { continue; }
					QuadTreeNode* subNode = node.node->childs[i];
					double squaredDistance = subNode->SquaredDistanceToPoint(targetPoint);
					//std::cout <<"node"<< squaredDistance << std::endl;
					heap.push(QuadTreeSearchNode(subNode, squaredDistance));
				}
			}
		}
	}
	//std::cout << pointIds.size()<<'\t'<<count << std::endl;
	return count;
}

int QuadTree::FindKNearestNeighbors(Vector2d targetPoint, int k, std::vector<int>& pointIds,double boundary)
{

	int count = 0;
	std::priority_queue<QuadTreeSearchNode> heap;  // �������ȶ��н���k��������
	heap.push(QuadTreeSearchNode(root, 0));  // ��Ӱ˲������ڵ�
	while (!heap.empty() && count < k) {
		QuadTreeSearchNode node = heap.top();
		heap.pop();
		if (node.isPoint)
		{  // ��ǰ����������Ϊһ�㣬��������
			//std::cout << node.pointId << std::endl;
			pointIds.push_back(node.pointId);
			count++;
		}
		else {  // ��ǰ����������Ϊһ��飬��Ⲣ�������
			if (node.node->isLeaf) {  // ���ΪҶ���ʱ����������������
				for (int i = 0; i < node.node->pointIds->size(); i++) {
					int id = (*(node.node->pointIds))[i];
					double squaredDistance = (points[id] - targetPoint).squaredNorm();
					if (squaredDistance > boundary)
						continue;
					//std::cout << "leaf"<<squaredDistance << std::endl;
					heap.push(QuadTreeSearchNode(id, squaredDistance));
				}
			}
			else {  // ���Ϊ��Ҷ���ʱ����������������
				for (int i = 0; i < 4; i++) {
					if (node.node->childs[i] == NULL) { continue; }
					QuadTreeNode* subNode = node.node->childs[i];
					double squaredDistance = subNode->SquaredDistanceToPoint(targetPoint);
					if (squaredDistance > boundary)
						continue;
					heap.push(QuadTreeSearchNode(subNode, squaredDistance));
				}
			}
		}
	}
	//std::cout << pointIds.size()<<'\t'<<count << std::endl;
	return count;
}

QuadTreeSearchNode::QuadTreeSearchNode(QuadTreeNode* node, double squaredDistance) {
	isPoint = false;
	this->node = node;
	squaredDistanceToTarget = squaredDistance;
}
QuadTreeSearchNode::QuadTreeSearchNode(int pointId, double squaredDistance) {
	isPoint = true;
	this->pointId = pointId;
	squaredDistanceToTarget = squaredDistance;
}
