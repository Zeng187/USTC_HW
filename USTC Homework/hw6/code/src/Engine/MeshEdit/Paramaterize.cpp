#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

using namespace Ubpa;
using namespace Eigen;
using namespace std;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh) 
{
	minSurf = MinSurf::New(triMesh);
	this->triMesh = triMesh;
}

void Paramaterize::Clear() 
{
	minSurf->Clear();
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) 
{
	Clear();
	minSurf->Init(triMesh);
	return true;
}

bool Paramaterize::Run(Paramethod pmethod,ParaBound pbound) {
	if (minSurf->heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	switch(pmethod)
	{
	case kUniform:
		minSurf->GetUniformWeight();
		break;
	case kCot:
		minSurf->GetCotWeight();
		break;
	case kShapePreserving:
		minSurf->GetShapeWeight();
	default:
		minSurf->GetCotWeight();
		break;
	}

	switch (pbound)
	{
	case kRect:
		MapBoundaryRect();
		break;
	case kCircle:
		MapBoundaryRound();
		break;
	default:
		MapBoundaryRect();
		break;
	}

	minSurf->Minimize();
	
	auto& texcoords = triMesh->GetTexcoords();

	for (int i = 0; i < texcoords.size(); i++)
	{
		texcoords[i] = Ubpa::pointf2(minSurf->heMesh->Vertices()[i]->pos[0], minSurf->heMesh->Vertices()[i]->pos[1]);
	}

	return true;
}

bool Paramaterize::ShowParameterizeResult()
{
	size_t nV = triMesh->GetPositions().size();
	std::vector<Ubpa::pointf3> positions;
	positions.reserve(nV);
	for (int j = 0; j < nV; j++)
	{
		positions.push_back(Ubpa::pointf3(triMesh->GetTexcoords()[j][0], triMesh->GetTexcoords()[j][1], 0.f));
	}
	triMesh->Update(positions);

	return true;
}



void Paramaterize::MapBoundaryRect()
{
	auto bound = minSurf->heMesh->Boundaries();
	int m = 0;
	for (int i = 0; i < bound.size(); i++)
	{
		auto tmp = bound[i];
		if (tmp.size() > (bound[m].size()))
			m = i;
	}
	auto boundary = bound[m];

	int n = boundary.size();
	double length_=0;
	std::vector<double> cumlenghth_;

	cumlenghth_.push_back(0.0);
	for (int i = 1; i < n; i++)
	{
		vecf3 dv = (boundary[i]->End()->pos - boundary[i-1]->End()->pos);
		double steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
		cumlenghth_.push_back(length_+steplength);
		length_ += steplength;
		//std::cout << cumlenghth_[cumlenghth_.size() - 1]<<'\t';
	}
	vecf3 dv = (boundary[0]->End()->pos - boundary[n - 1]->End()->pos);
	double steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
	cumlenghth_.push_back(length_ + steplength);
	length_ += steplength;
	//std::cout << cumlenghth_[cumlenghth_.size() - 1];

	double div = 0.25;
	boundary[0]->End()->pos= vecf3(0.f, 0.f, 0.f);
	for (int i = 1; i < n; i++)
	{
		cumlenghth_[i] = cumlenghth_[i] / length_;
		if (cumlenghth_[i] < div)
		{
			boundary[i]->End()->pos = vecf3(4* cumlenghth_[i], 0.f, 0.f);
		}
		else if (cumlenghth_[i] < 2*div)
		{
			boundary[i]->End()->pos = vecf3(1, (cumlenghth_[i]-div)*4, 0.f);
		}
		else if (cumlenghth_[i] < 3 * div)
		{
			boundary[i]->End()->pos = vecf3((3 * div - cumlenghth_[i])*4, 1, 0.f);
		}
		else
		{
			boundary[i]->End()->pos = vecf3(0, (1 - cumlenghth_[i])*4, 0.f);
		}
	}
	


}

void Paramaterize::MapBoundaryRound()
{
	auto bound = minSurf->heMesh->Boundaries();
	int m = 0;
	for (int i = 0; i < bound.size(); i++)
	{
		auto tmp = bound[i];
		if (tmp.size() > (bound[m].size()))
			m = i;
	}
	auto boundary = bound[m];


	int n = boundary.size();
	double length_ = 0;
	std::vector<double> cumlenghth_;

	cumlenghth_.push_back(0.0);
	for (int i = 1; i < n; i++)
	{
		vecf3 dv = (boundary[i]->End()->pos - boundary[i - 1]->End()->pos);
		double steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
		cumlenghth_.push_back(length_ + steplength);
		length_ += steplength;
		//std::cout << cumlenghth_[cumlenghth_.size() - 1]<<'\t';
	}
	vecf3 dv = (boundary[0]->End()->pos - boundary[n - 1]->End()->pos);
	double steplength = sqrt(dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2]);
	cumlenghth_.push_back(length_ + steplength);
	length_ += steplength;

	float pi_ = 3.14159265f ;

	for (int i = 0; i < boundary.size(); i++)
	{
		cumlenghth_[i] = cumlenghth_[i] / length_;
		boundary[i]->End()->pos = Ubpa::vecf3(cos(pi_*2*cumlenghth_[i]), sin(pi_*2*cumlenghth_[i]), 0.f);
	}
}
