
#include <Engine/MeshEdit/ASAP.h>
#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/SVD>

using namespace Ubpa;
using namespace Eigen;
using namespace std;

ASAP::ASAP(Ptr<TriMesh> triMesh) :LGParaBase(triMesh)
{

}

bool ASAP::Run()
{
	if (minSurf->heMesh->IsEmpty() || !minSurf->triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !minSurf->triMesh\n");
		return false;
	}

	InitPara();  //to give the initial parameterize result
	SetFixed(); //set fixed points
	GetPrePara();

	ComputSystemPara();
	ComputeA();
	Computeb();
	ReZoom();

	for (int times = 0; times < 10; times++)
	{
		Renewu();
		RenewLt();
		Computeb();
		ReZoom();
	}

	Refresh();

	return true;
}

Eigen::Matrix2f ASAP::ComputeLt(Eigen::Matrix2f Lt)
{
	JacobiSVD<Eigen::Matrix2f> svd(Lt, ComputeFullU | ComputeFullV);
	auto m = svd.singularValues();
	return ((m[0]+m[1])/2)*svd.matrixU() * (svd.matrixV().transpose());
}

void ASAP::SetFixed()
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
	is_fixed[minSurf->heMesh->Index(bound[m][0]->End())] = true;
	//is_fixed[minSurf->heMesh->Index(bound[m][lnb / 2]->End())] = true;


}
