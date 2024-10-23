#pragma once


#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Engine/MeshEdit/MinSurf.h>
#include <Eigen/Sparse>
#include <Eigen/src/SparseLU/SparseLU.h>

namespace Ubpa {
	class TriMesh;
	class MinSurf;
	class Paramaterize;


	// mesh boundary == 1
	class LGParaBase : public HeapObj
	{
	public:
		LGParaBase(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<LGParaBase> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<LGParaBase>(triMesh);
		}


	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		bool ShowParameterizeResult();

		virtual bool Run();

	protected:
		// kernel part of the algorithm
		void InitPara();
		bool GetPrePara();
		void ComputSystemPara();
		void MapBoundaryRound();
		void ReZoom();
		virtual void SetFixed();

		Eigen::Matrix2f ComputeJaccobi(size_t t);
		void ComputeA();		//the coeffients mattix A only need to compute once
		void Computeb();
		virtual Eigen::Matrix2f ComputeLt(Eigen::Matrix2f Lt);

		void Refresh();
		void Renewu();
		void RenewLt();

		int LocAtTri(Ubpa::MinSurf::P* tri, Ubpa::MinSurf::V* v);


	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };

	protected:
		Ptr<MinSurf> minSurf;

		std::vector<bool> is_fixed;
		std::vector<Eigen::Matrix2f> L;
		std::vector<std::vector<float>> cot;	//for each tritangle
		std::vector<std::vector<Eigen::Vector2f>> x;		//for each tritangle
		std::vector<Eigen::Vector2f> u;

		Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
		Eigen::SparseMatrix<float> A;
		Eigen::MatrixXf b;
		Eigen::MatrixXf result;
		
	};
}
