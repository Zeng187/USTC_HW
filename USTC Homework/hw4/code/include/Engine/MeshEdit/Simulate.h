#pragma once

#include <Basic/HeapObj.h>
//#include <Engine/Primitive/MassSpring.h>
#include <UGM/UGM>
#include <Eigen/dense>
#include <Eigen/Sparse>


namespace Ubpa {
	class Simulate : public HeapObj {
	public:
		Simulate(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			edgelist = elist;
			this->positions.resize(plist.size());
			for (int i = 0; i < plist.size(); i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->positions[i][j] = plist[i][j];
				}
			}
		};
	public:
		static const Ptr<Simulate> New(const std::vector<pointf3>& plist,
			const std::vector<unsigned> &elist) {
			return Ubpa::New<Simulate>(plist, elist);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init();
		//bool Init();

		// call it after Init()
		bool Run();
		
		const std::vector<pointf3>& GetPositions() const { return positions; };

		const float GetStiff() { return stiff; };
		void SetStiff(float k) { stiff = k; Init();};
		const float GetTimeStep() { return h; };
		void SetTimeStep(float k) { h = k; Init();};
		std::vector<unsigned>& GetFix() { return this->fixed_id; };
		void SetFix(const std::vector<unsigned>& f) { this->fixed_id = f; Init();};
		const std::vector<pointf3>& GetVelocity() { return velocity; };
		//void SetVelocity(const std::vector<pointf3>& v) { velocity = v; };

		void SetLeftFix();
		void SetTopFix();
		void SetCornerFix();

		void SetMode(bool isAcc);

	private:
		// kernel part of the algorithm
		void SimulateOnce();
		void ImplicitEuler();
		void AccSimulation();

		void AccInit();
		void SetRestlength();

		void ComputeA();
		void ComputeB();

		void LocalStep();
		void GlobalStep();



	private:
		float h = 0.002f;  //²½³¤
		float stiff;
		float damping;
		float g;	//gravity parameter
		float m;	//mass
		size_t s;   //the spring size
		size_t nV;

		Eigen::Vector3f f_ext;

		std::vector<bool> is_fixed;  //fixed point id
		std::vector<unsigned> fixed_id;


		//mesh data
		std::vector<unsigned> edgelist;
		std::vector<double> springlist;	//the rest length of spring	
		std::vector<Eigen::Vector3f> y;						
		std::vector<Eigen::Vector3f> x;						
		std::vector<Eigen::Vector3f> gfun;
		std::vector<Eigen::Matrix3f> gradient;


		//simulation data
		std::vector<pointf3> positions;
		std::vector<pointf3> velocity;

		Eigen::SparseMatrix<float> M;
		Eigen::SparseMatrix<float> L;
		Eigen::SparseMatrix<float> A;
		Eigen::SparseMatrix<float> B;

		Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
		Eigen::VectorXf X;
		Eigen::VectorXf d;
		Eigen::VectorXf Y;
		Eigen::VectorXf b;
		Eigen::VectorXf F_ext;

		bool is_AccMode;



	};
}
