#pragma once

#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Engine/MeshEdit/MinSurf.h>
#include <Eigen/Sparse>
#include <Eigen/src/SparseLU/SparseLU.h>
#include <Engine/MeshEdit/LGParaBase.h>


namespace Ubpa {
	class TriMesh;
	class MinSurf;
	class Paramaterize;


	// mesh boundary == 1
	class ARAP : public LGParaBase
	{
	public:
		ARAP(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<ARAP> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<ARAP>(triMesh);
		}


	public:
		bool Run();

	protected:
		void SetFixed();
		Eigen::Matrix2f ComputeLt(Eigen::Matrix2f Lt);

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



	};
}
