#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();
	w.clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

bool MinSurf::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	GetCotWeight();
	Minimize();

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);

	return true;
}

void MinSurf::Minimize() {

	//determine the coefficient matrix
	const auto &vertices= heMesh->Vertices();		//find all the vertices
	auto bound = heMesh->Boundaries()[0];		//find all the boundaries (organized as vector of halfedge)

	if (bound.size()>0)
	{
		int n = vertices.size() - bound.size();
		SparseMatrix<double> A(n, n);		//the left side of the inear system
		MatrixX3d b;		//the right side of the linear system
		b.resize(n, 3);
		b.fill(0);

		vector<int> bound_indices;
		for (auto v : bound)
		{
			bound_indices.push_back(heMesh->Index(v->End()));		//find all the indice of points in the boundaries
		}

		std::vector<int> new_indices;		//record the new indices in the linear system
		int count = 0;

		//reorganize the points
		for (auto v : vertices)
		{

			int index = heMesh->Index(v);
			bool is_bound = IsBoundaries(bound_indices, index);
			if (!is_bound)
			{
				new_indices.push_back(count);
				count++;
			}
			else
			{
				new_indices.push_back(-1);
			}

		}

		
		//constructe the linear system
		for (int j = 0; j < vertices.size(); j++)
		{
			auto v = vertices[j];

			if (new_indices[j] != -1)
			{
				//inner points
				int index = new_indices[j];
				auto adj_v = v->AdjVertices();
				double degree = adj_v.size();
				A.coeffRef(index, index) = 1;

				std::vector<double> wi = w[j];
				for (int i=0;i<adj_v.size();i++)
				{
					auto v1 = adj_v[i];
					int adj_index = new_indices[heMesh->Index(v1)];
					if (adj_index == -1)
					{
						b(index, 0) += v1->pos[0] * wi[i];
						b(index, 1) += v1->pos[1] * wi[i];
						b(index, 2) += v1->pos[2] * wi[i];
					}
					else
					{
						A.coeffRef(index, adj_index) = -1.0 * wi[i];
					}
				}
			}


		}
		
		SparseLU<SparseMatrix<double>> solver;
		solver.compute(A);
		
		if (solver.info() != Success)
		{
			cerr << "Unable to decompose the matrix" << endl;
		}

		auto result = solver.solve(b);
		//std::cout <<"rows:"<< result.rows() << std::endl;
		
		for (int j = 0; j < vertices.size(); j++)
		{
			int index = new_indices[j];

			if (new_indices[j] != -1)
			{
				//innner point
				for (int i = 0; i < 3; i++)
				{
					vertices[j]->pos[i] = result(index, i);
				}
			}
		}
		
	}

}

bool MinSurf::IsBoundaries(std::vector<int> b, int index)
{
	for (int i = 0; i < b.size(); i++)
	{
		if (index == b[i])
			return true;
	}

	return false;
}

void MinSurf::GetCotWeight()
{
	std::vector<double> wi;
	for (const auto& v : heMesh->Vertices())
	{
		wi = GetCotWeightAt(v);
		w.push_back(wi);
	}

}

std::vector<double> MinSurf::GetCotWeightAt(Ubpa::MinSurf::V* v)
{
	std::vector<double> wi;
	auto adj_v = v->AdjVertices();
	int n = adj_v.size();
	double sumw = 0; 
	for (int i=0;i<n;i++)
	{
		auto pre = adj_v[(i + n - 1) % n]->pos;
		auto next = adj_v[(i + 1) % n]->pos;
		double cosa = Ubpa::vecf3::cos_theta(v->pos - pre, adj_v[i]->pos - pre);
		double cosb = Ubpa::vecf3::cos_theta(v->pos - next, adj_v[i]->pos - next);
		double sina = sqrt(1 - cosa * cosa);
		double sinb = sqrt(1 - cosb * cosb);
		wi.push_back(cosa / sina + cosb / sinb);
		sumw += wi[wi.size() - 1];
	}

	for (int i = 0; i < n; i++)
	{
		wi[i] = wi[i] / sumw;
	}
	return wi;

}

void MinSurf::GetUniformWeight()
{
	std::vector<double> wi;
	for (const auto& v : heMesh->Vertices())
	{
		wi = GetUniformWeightAt(v);
		w.push_back(wi);
	}

}

std::vector<double> MinSurf::GetUniformWeightAt(Ubpa::MinSurf::V* v)
{
	std::vector<double> wi;
	auto adj_v = v->AdjVertices();
	double n = adj_v.size();

	for (int i = 0; i < n; i++)
	{
		wi.push_back(1.0 / n);
	}
	return wi;

}

void MinSurf::GetShapeWeight()
{
	std::vector<double> wi;
	for (const auto& v : heMesh->Vertices())
	{
		wi = GetShapeWeightAt(v);
		w.push_back(wi);
	}

}

std::vector<double> MinSurf::GetShapeWeightAt(Ubpa::MinSurf::V* v)
{
	float pi_ = 3.14159265f;

	std::vector<double> wi;
	std::vector<double> lambda;
	auto adj_v = v->AdjVertices();
	int n = adj_v.size();

	//construct a angle list for the points projecting into plane
	std::vector<double> angle; double sumangle=0;
	for (int i = 0; i < n; i++)
	{
		auto now = adj_v[i]->pos;
		auto next = adj_v[(i + 1) % n]->pos;
		double cosa = Ubpa::vecf3::cos_theta(now - v->pos, next - v->pos);
		angle.push_back(acos(cosa));
		sumangle += acos(cosa);
	}

	lambda.resize(n);
	for (int i = 0; i < n; i++)
	{
		lambda[i] = 0;
		angle[i] = 2*pi_*angle[i] / sumangle;
	}

	double sumw = 0,cumangle=0;
	for (int i = 0; i < n; i++)
	{
		cumangle = 0;
		auto now= adj_v[i]->pos;

		int pre_idx = i, next_idx = i;
		while (cumangle < pi_)
		{
			pre_idx = next_idx;
			cumangle += angle[pre_idx];
			next_idx = (next_idx + 1) % n;
		}

		auto pre = adj_v[pre_idx]->pos;
		auto next = adj_v[next_idx]->pos;

		double sinnow = sin(angle[pre_idx]);
		double sinpre =sin(2*pi_-cumangle);
		double sinnext = sin(cumangle - angle[pre_idx]);

		double dnow = (now - v->pos).norm();
		double dpre = (pre - v->pos).norm();
		double dnext = (next - v->pos).norm();

		double S1 = dpre * dnext * sinnow;
		double S2 = dnow * dnext * sinpre;
		double S3 = dnow * dpre * sinnext;

		double S = S1 + S2 + S3;

		lambda[i] += S1 / S;
		lambda[pre_idx] += S2 / S;
		lambda[next_idx] += S3 / S;
	}

	for (int i = 0; i < n; i++)
	{
		wi.push_back(lambda[i] / n);
	}

	return wi;

}