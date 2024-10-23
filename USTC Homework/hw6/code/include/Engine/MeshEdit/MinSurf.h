#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

namespace Ubpa {
	class TriMesh;
	class Paramaterize;
	class LGParaBase;
	class ARAP;
	class ASAP;

	class MinSurf : public HeapObj 
	{
	public:
		MinSurf(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<MinSurf> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<MinSurf>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool Run();

	private:
		// kernel part of the algorithm
		void Minimize();
		bool IsBoundaries(std::vector<int>, int);


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
	private:
		friend class Paramaterize;
		friend class LGParaBase;
		friend class ARAP;
		friend class ASAP;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh

		std::vector<std::vector<double>> w;
		void GetUniformWeight();
		void GetCotWeight();
		void GetShapeWeight();
		std::vector<double> GetUniformWeightAt(Ubpa::MinSurf::V* v);
		std::vector<double> GetCotWeightAt(Ubpa::MinSurf::V* v);
		std::vector<double> GetShapeWeightAt(Ubpa::MinSurf::V* v);
	};
}
