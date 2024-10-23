
#include <Engine/MeshEdit/LGParaBase.h>
#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/SVD>

using namespace Ubpa;
using namespace Eigen;
using namespace std;

LGParaBase::LGParaBase(Ptr<TriMesh> triMesh)
{
	minSurf = MinSurf::New(triMesh);
}

void LGParaBase::Clear()
{
	minSurf->Clear();
}

bool LGParaBase::Init(Ptr<TriMesh> triMesh)
{
	Clear();
	minSurf->Init(triMesh);
	return true;
}

bool LGParaBase::Run() {
	if (minSurf->heMesh->IsEmpty() || !minSurf->triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !minSurf->triMesh\n");
		return false;
	}


	return true;
}

void LGParaBase::InitPara()
{
	minSurf->GetShapeWeight();
	MapBoundaryRound();
	minSurf->Minimize();

	auto& texcoords = minSurf->triMesh->GetTexcoords();
	for (size_t i = 0; i < texcoords.size(); i++)
	{
		texcoords[i] = Ubpa::pointf2(minSurf->heMesh->Vertices()[i]->pos[0], minSurf->heMesh->Vertices()[i]->pos[1]);
	}


}

bool LGParaBase::GetPrePara()
{
	u.clear();

	auto& texcoords = minSurf->triMesh->GetTexcoords();

	if (texcoords.size() == 0)
	{
		return false;
	}

	for (size_t i = 0; i < texcoords.size(); i++)
	{
		u.push_back(Vector2f(minSurf->triMesh->GetTexcoords()[i][0], minSurf->triMesh->GetTexcoords()[i][1]));
	}
	return true;

}

void LGParaBase::SetFixed()
{
	auto bound = minSurf->heMesh->Boundaries();
	size_t nb = bound.size();

	std::vector<Ubpa::MinSurf::V*> vertices = minSurf->heMesh->Vertices();
	size_t n = vertices.size();

	for (size_t i = 0; i < n; i++)
		is_fixed.push_back(false);

	size_t m = 0;
	for (size_t j = 0; j < nb; j++)
	{
		m = bound[j].size() > bound[m].size() ? j : m;
	}

	size_t lnb = bound[m].size();
	is_fixed[minSurf->heMesh->Index(bound[m][0]->End())]=true;
	is_fixed[minSurf->heMesh->Index(bound[m][lnb/2]->End())] = true;


}



bool LGParaBase::ShowParameterizeResult()
{
	size_t nV = minSurf->triMesh->GetPositions().size();
	std::vector<Ubpa::pointf3> positions;
	positions.reserve(nV);
	for (size_t j = 0; j < nV; j++)
	{
		positions.push_back(Ubpa::pointf3(minSurf->triMesh->GetTexcoords()[j][0], minSurf->triMesh->GetTexcoords()[j][1], 0.f));
	}
	minSurf->triMesh->Update(positions);

	return true;
}


void LGParaBase::ComputSystemPara()
{
	
	auto tri_list = minSurf->triMesh->GetTriangles();
	const std::vector<Ubpa::pointf3>& positions = minSurf->triMesh->GetPositions();
	size_t nt = tri_list.size();
	L.clear();

	std::vector<MinSurf::V*> tri_v;
	Vector2f v;
	std::vector<Vector2f> tri_x;
	std::vector<float> tri_cot;
	float d1, d2,cosa,sina;

	size_t t = 0;
	for (const auto& tri : tri_list)
	{
		//take a translation or rotation transformation to triangle
		tri_x.clear();
		tri_cot.clear();

		Ubpa::vecf3 tri_v[3];
		for (size_t i = 0; i < 3; i++)
		{
			size_t j = tri->idx[i];
			tri_v[i] = Ubpa::vecf3(positions[j][0], positions[j][1], positions[j][2]);
		}

		d1 = (tri_v[1] - tri_v[0]).norm();
		d2 = (tri_v[2] - tri_v[0]).norm();
		v = Vector2f(0, 0);
		tri_x.push_back(v);
		v = Vector2f(d1, 0);
		tri_x.push_back(v);
		cosa = Ubpa::vecf3::cos_theta(tri_v[1] - tri_v[0], tri_v[2] - tri_v[0]);
		sina = sqrt(1.0 - cosa * cosa);
		v = Vector2f(d2 * cosa, d2 * sina);
		tri_x.push_back(v);
		tri_cot.push_back(cosa / sina);
		cosa = Ubpa::vecf3::cos_theta(tri_v[2] - tri_v[1], tri_v[0] - tri_v[1]);
		sina = sqrt(1.0 - cosa * cosa);
		tri_cot.push_back(cosa / sina);
		cosa = Ubpa::vecf3::cos_theta(tri_v[0] - tri_v[2], tri_v[1] - tri_v[2]);
		sina = sqrt(1.0 - cosa * cosa);
		tri_cot.push_back(cosa / sina);

		x.push_back(tri_x);
		cot.push_back(tri_cot);

		Eigen::Matrix2f Jt=ComputeJaccobi(t);
		Eigen::Matrix2f Lt = ComputeLt(Jt);
		L.push_back(Lt);
		//std::cout << Lt << endl;
		t++;


	}




}

Eigen::Matrix2f LGParaBase::ComputeJaccobi(size_t t)
{
	Eigen::Matrix2f Jt;
	Jt.fill(0.0);

	auto tri = minSurf->heMesh->Polygons()[t];
	std::vector<Vector2f> tri_xlist=x[t];
	std::vector<float> tri_cotlist=cot[t];

	size_t u_index[3] = { minSurf->heMesh->Index(tri->BoundaryVertice()[0]),
		minSurf->heMesh->Index(tri->BoundaryVertice()[1]),
		minSurf->heMesh->Index(tri->BoundaryVertice()[2]) };


	for (size_t i = 0; i < 3; i++)
	{
		Vector2f dx = tri_xlist[i] - tri_xlist[(i + 1) % 3];
		Vector2f du = u[u_index[i]] - u[u_index[(i + 1) % 3]];
		size_t loc_theta = LocAtTri(tri, tri->BoundaryVertice()[(i + 2) % 3]);
		Matrix2f temp=du*dx.transpose() ;
		Jt += temp * tri_cotlist[loc_theta];
	}
	//std::cout << Jt << endl;;

	return Jt;
}

Eigen::Matrix2f LGParaBase::ComputeLt(Eigen::Matrix2f Lt)
{
	JacobiSVD<Eigen::Matrix2f> svd(Lt, ComputeFullU | ComputeFullV);
	auto m = svd.singularValues();
	return svd.matrixU() * (svd.matrixV().transpose());
}

void LGParaBase::ComputeA()
{
	std::vector<Ubpa::MinSurf::V*> vertices = minSurf->heMesh->Vertices();
	size_t n = vertices.size();
	A.resize(n, n);
	size_t i = 0; 
	std::vector<Eigen::Triplet<float>> coeffients;
	float diagcoeff, coeff_ij;

	//std::cout << "A:" << std::endl;

	for (const auto &v:vertices)
	{
		// judge whether the point is fixed
		//std::cout << i<<'\t';
		if (!is_fixed[i])
		{
			std::vector<Ubpa::MinSurf::V*> adj_vlist = v->AdjVertices();
			size_t adj_n = adj_vlist.size();
			diagcoeff = 0;

			for (size_t j = 0; j < adj_n; j++)
			{
				auto adj_v = adj_vlist[j];
				size_t adj_index = minSurf->heMesh->Index(adj_v);

				auto adj_next = adj_vlist[(j + 1) % adj_n];
				auto adj_pre = adj_vlist[(j - 1 + adj_n) % adj_n];
				coeff_ij = 0;


				auto he1 = v->HalfEdgeTo(adj_v);
				if (he1->Polygon() != NULL)
				{
					Ubpa::MinSurf::P* tri = he1->Polygon();
					size_t tri_index = minSurf->heMesh->Index(tri);
					size_t loc = LocAtTri(tri, adj_pre);

					diagcoeff += cot[tri_index][loc];
					coeff_ij += cot[tri_index][loc];
				}

				auto he2 = adj_v->HalfEdgeTo(v);
				if (he2->Polygon() != NULL)
				{
					Ubpa::MinSurf::P* tri = he2->Polygon();
					size_t tri_index = minSurf->heMesh->Index(tri);
					size_t loc = LocAtTri(tri, adj_next);

					diagcoeff += cot[tri_index][loc];
					coeff_ij += cot[tri_index][loc];
				}

				coeffients.push_back(Eigen::Triplet<float>(i, adj_index, -coeff_ij));
				//cout << -coeff_ij << '\t';
				
			}

			coeffients.push_back(Eigen::Triplet<float>(i, i, diagcoeff));
			//cout << diagcoeff << endl;
		}
		else
		{
			//fixed point
			coeffients.push_back(Eigen::Triplet<float>(i, i, 1));
		}

		i++;

	}

	A.setFromTriplets(coeffients.begin(), coeffients.end());

	solver.compute(A);

}

void LGParaBase::Computeb()
{
	std::vector<Ubpa::MinSurf::V*> vertices = minSurf->heMesh->Vertices();
	size_t n = vertices.size();
	b.resize(n, 2);

	size_t i = 0;

	for (const auto& v : vertices)
	{
		// judge whether the point is fixed
		if (!is_fixed[i])
		{
			std::vector<Ubpa::MinSurf::V*> adj_vlist = v->AdjVertices();
			size_t adj_n = adj_vlist.size();
			Vector2f coeff = Vector2f(0,0);
			for (size_t j = 0; j < adj_n; j++)
			{
				auto adj_v = adj_vlist[j];
				size_t adj_index = minSurf->heMesh->Index(adj_v);

				auto adj_next = adj_vlist[(j + 1) % adj_n];
				auto adj_pre = adj_vlist[(j - 1 + adj_n) % adj_n];

				auto he1 = v->HalfEdgeTo(adj_v);
				if (he1->Polygon() != NULL)
				{
					Ubpa::MinSurf::P* tri = he1->Polygon();
					size_t tri_index = minSurf->heMesh->Index(tri);
					auto Lt = L[tri_index];
					size_t loc_theta = LocAtTri(tri, adj_pre);
					size_t loc_i = LocAtTri(tri, v);
					size_t loc_j = LocAtTri(tri, adj_v);

					Vector2f temp = (cot[tri_index][loc_theta]) * Lt * (x[tri_index][loc_i] - x[tri_index][loc_j]);
					coeff += temp;

				}

				auto he2 = adj_v->HalfEdgeTo(v);
				if (he2->Polygon() != NULL)
				{
					Ubpa::MinSurf::P* tri = he2->Polygon();
					size_t tri_index = minSurf->heMesh->Index(tri);
					auto Lt = L[tri_index];
					size_t loc_theta = LocAtTri(tri, adj_next);
					size_t loc_i = LocAtTri(tri, v);
					size_t loc_j = LocAtTri(tri, adj_v);

					Vector2f temp = (cot[tri_index][loc_theta]) * Lt * (x[tri_index][loc_i] - x[tri_index][loc_j]);
					coeff += temp;

				}

				

			}
			b(i, 0) = coeff[0];
			b(i, 1) = coeff[1];
		}
		else
		{
			//fixed point

			b(i, 0) = u[i][0];
			b(i, 1) = u[i][1];


		}

		i++;

	}


	result=solver.solve(b);

}

int LGParaBase::LocAtTri(Ubpa::MinSurf::P* tri, Ubpa::MinSurf::V* v)
{
	auto vlist = tri->BoundaryVertice();
	for (int i = 0; i < 3; i++)
	{
		if (minSurf->heMesh->Index(vlist[i]) == minSurf->heMesh->Index(v))
		{
			return i;
		}
	}
	return -1;
}

void LGParaBase::Refresh()
{
	auto& texcoords = minSurf->triMesh->GetTexcoords();
	for (size_t i = 0; i < texcoords.size(); i++)
	{
		texcoords[i] = Ubpa::pointf2(result(i,0), result(i,1));
	}
}

void LGParaBase::Renewu()
{
	std::vector<Ubpa::MinSurf::V*> vertices = minSurf->heMesh->Vertices();
	size_t n = vertices.size();
	for (size_t i = 0; i < n; i++)
	{
		u[i][0] = result(i, 0);
		u[i][1] = result(i, 1);
	}
}

void LGParaBase::RenewLt()
{
	L.clear();
	size_t nt = minSurf->heMesh->Polygons().size();
	for (size_t t = 0; t < nt; t++)
	{
		Eigen::Matrix2f Jt = ComputeJaccobi(t);
		Eigen::Matrix2f Lt = ComputeLt(Jt);
		L.push_back(Lt);
	}

}

void LGParaBase::ReZoom()
{
	size_t nV = minSurf->heMesh->NumVertices();
	size_t m1 = 0, m2 = 0, m3 = 0, m4 = 0;
	for (size_t i = 0; i < nV; i++)
	{
		if (result(i, 0) > result(m1, 0))
			m1 = i;
		if (result(i, 0) < result(m2, 0))
			m2 = i;
		if (result(i, 1) > result(m3, 1))
			m3 = i;
		if (result(i, 1) < result(m4, 1))
			m4 = i;
	}
	float scale = 1.f / max(result(m1, 0) - result(m2, 0), result(m3, 1) - result(m4, 1));
	Eigen::Vector2f  sh = Eigen::Vector2f(result(m2, 0), result(m4, 1));
	for (size_t i = 0; i < nV; i++)
	{
		result.row(i) = (result.row(i) - sh.transpose()) * scale;
	}
}

void LGParaBase::MapBoundaryRound()
{
	auto bound = minSurf->heMesh->Boundaries();
	size_t m = 0;
	for (size_t i = 0; i < bound.size(); i++)
	{
		auto tmp = bound[i];
		if (tmp.size() > (bound[m].size()))
			m = i;
	}
	auto boundary = bound[m];

	size_t n = boundary.size();
	float length_ = 0;
	std::vector<float> cumlenghth_;

	cumlenghth_.push_back(0.0);
	for (size_t i = 1; i < n; i++)
	{
		vecf3 dv = (boundary[i]->End()->pos - boundary[i - 1]->End()->pos);
		float steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
		cumlenghth_.push_back(length_ + steplength);
		length_ += steplength;

	}
	vecf3 dv = (boundary[0]->End()->pos - boundary[n - 1]->End()->pos);
	float steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
	cumlenghth_.push_back(length_ + steplength);
	length_ += steplength;

	float pi_ = 3.14159265f;

	for (size_t i = 0; i < boundary.size(); i++)
	{
		cumlenghth_[i] = cumlenghth_[i] / length_;
		boundary[i]->End()->pos = Ubpa::vecf3(cos(pi_ * 2 * cumlenghth_[i]), sin(pi_ * 2 * cumlenghth_[i]), 0.f);
	}

}