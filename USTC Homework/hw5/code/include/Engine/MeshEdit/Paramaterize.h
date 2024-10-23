#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

namespace Ubpa {
	class TriMesh;
	class MinSurf;



	// mesh boundary == 1
	class Paramaterize : public HeapObj 
	{
	public:
		Paramaterize(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Paramaterize>(triMesh);
		}

		enum Paramethod
		{
			kUniform,
			kCot,
			kShapePreserving
		};

		enum ParaBound
		{
			kCircle,
			kRect,
		};

	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		bool Run(Paramethod pmethod, ParaBound pbound);
		bool ShowParameterizeResult();


	private:
		// kernel part of the algorithm
		void MapBoundaryRound();
		void MapBoundaryRect();

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

		Ptr<MinSurf> minSurf;
		Ptr<TriMesh> triMesh;

		friend class LocalGlobalPara;
	};
}
