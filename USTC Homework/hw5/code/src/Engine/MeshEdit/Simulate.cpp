#include <Engine/MeshEdit/Simulate.h>


#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;


void Simulate::Clear() {
	this->positions.clear();
	this->velocity.clear();
}

bool Simulate::Init() {
	//Clear();

	is_AccMode = false;//默认隐式欧拉用牛顿迭代

	this->velocity .resize(positions.size());
	for (size_t i = 0; i < positions.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			this->velocity[i][j] = 0;
		}
	}

	nV = positions.size();
	stiff = 50000;
	g = 9.8;
	m = 1;
	s = edgelist.size() / 2;

	f_ext = Eigen::Vector3f(0, -m * g, 0.25*m*g);

	springlist.clear();

	SetRestlength();

	is_fixed.resize(nV);
	for (size_t i = 0; i < nV; i++)
	{
		is_fixed[i] = false;
	}

	

	return true;
}

bool Simulate::Run() 
{
	
	SimulateOnce();

	// half-edge structure -> triangle mesh

	return true;
}

void Ubpa::Simulate::SetMode(bool IsAcc)
{
	this->is_AccMode = IsAcc;
	if (IsAcc)
		AccInit();

}

void Ubpa::Simulate::SetLeftFix()
{
	//固定网格x坐标最小点

	double xx = 100000;
	for (size_t i = 0; i < positions.size(); i++)
	{
		is_fixed[i] = false;
		if (positions[i][0] < xx)
		{
			xx = positions[i][0];
		}
	}

	for (size_t i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - xx) < 1e-5)
		{
			is_fixed[i] = true;
			fixed_id.push_back(i);
		}
	}

}

void Ubpa::Simulate::SetTopFix()
{
	Init();
	double xmin = 100000, xmax = -100000, ymax = xmax, ymin = xmin;
	for (size_t i = 0; i < positions.size(); i++)
	{
		is_fixed[i] = false;
		if (positions[i][0] < xmin)
		{
			xmin = positions[i][0];
		}

		if (positions[i][0] > xmax)
		{
			xmax = positions[i][0];
		}

		if (positions[i][1] < ymin)
		{
			ymin = positions[i][1];
		}

		if (positions[i][1] > ymax)
		{
			ymax = positions[i][1];
		}

	}

	for (size_t i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][1] - ymax) < 1e-5)
		{
			if (abs(positions[i][0] - xmax) < 1e-5 || abs(positions[i][0] - xmin) < 1e-5)
			{
				is_fixed[i] = true;
				fixed_id.push_back(i);
			}
		}
	}

}

void Ubpa::Simulate::SetCornerFix()
{
	Init();
	double xmin = 100000, xmax = -100000, ymax = xmax, ymin = xmin;
	for (size_t i = 0; i < positions.size(); i++)
	{
		is_fixed[i] = false;
		if (positions[i][0] < xmin)
		{
			xmin = positions[i][0];
		}

		if (positions[i][0] > xmax)
		{
			xmax = positions[i][0];
		}

		if (positions[i][1] < ymin)
		{
			ymin = positions[i][1];
		}

		if (positions[i][1] > ymax)
		{
			ymax = positions[i][1];
		}

	}

	for (size_t i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][1] - ymax) < 1e-5|| abs(positions[i][1] - ymin) < 1e-5)
		{
			if (abs(positions[i][0] - xmax) < 1e-5 || abs(positions[i][0] - xmin) < 1e-5)
			{
				is_fixed[i] = true;
				fixed_id.push_back(i);
			}
		}
	}

}

void Simulate::SimulateOnce() 
{

	//ImplicitEuler();
	AccSimulation();
}


void Simulate::ImplicitEuler()
{

	y.clear();
	y.resize(nV);

	gradient.clear();
	gradient.resize(nV);

	gfun.clear();
	gfun.resize(nV);

	for (int i = 0; i < nV; i++)
	{
		for (int j = 0; j < 3; j++)
			y[i][j] = positions[i][j] + h * velocity[i][j] + h * h * f_ext[j] / m;

		gfun[i] = Eigen::Vector3f(0,0,0);
	}


	int t = 0;
	x = y;		//initial value of x

	while (t < 5)
	{
		for (int i = 0; i < nV; i++)
		{
			gfun[i] = m * (x[i] - y[i]);
			gradient[i] = m * Eigen::Matrix3f::Identity();
		}

		for (int k = 0; k < edgelist.size() - 1; k = k + 2)
		{
			int i = edgelist[k], j = edgelist[k + 1];
			Eigen::Vector3f d = x[j]-x[i];
			float l = d.norm();

			Eigen::Matrix3f gradient_ij = stiff * (springlist[k / 2] / l - 1) * Matrix3f::Identity() - stiff * (springlist[k / 2] / l / l / l) * d * d.transpose();
			gradient[i] -= h * h * gradient_ij;
			gradient[j] += h * h * gradient_ij;

			Eigen::Vector3f f_ij = stiff * (l - springlist[k / 2] ) * d / l;
			gfun[i] -= f_ij * h * h;
			gfun[j] += f_ij * h * h;
		}

		for (int i = 0; i < nV; i++)
		{
			if (is_fixed[i])
			{
				for (int j = 0; j < 3; j++)
				{
					x[i][j] = positions[i][j];
				}
			}
			else
				x[i] = x[i]- gradient[i].inverse()*gfun[i];
		}

		t++;
	}

	for (int i = 0; i < nV; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			velocity[i][j] = (x[i][j] - positions[i][j]) / h;
			positions[i][j] = x[i][j];
		}

	}


	//std::cout<< positions[0][0]<<'\t'<< positions[0][1]<<'\t' << positions[0][2]<<endl;
}


void Simulate::SetRestlength()
{

	for (int k = 0; k < edgelist.size()-1; k = k + 2)
	{
		int i = edgelist[k], j = edgelist[k + 1];
		springlist.push_back((positions[j] - positions[i]).norm());
	}
}

void Simulate::AccInit() 
{
	ComputeA();

	F_ext.resize(3 * nV);

	for (int i = 0; i < nV; i++)
	{
		for (int j = 0; j < nV; j++)
		{
			F_ext[3 * i + j] = f_ext[j];
		}
	}

	cout << "Simulation Init finish!" << endl;
}


void Simulate::AccSimulation() 
{
	//initial

	X.resize(3*nV);	

	for (size_t i = 0; i < nV; i++) 
	{
		for (int j = 0; j < 3; j++) 
		{

			X[3 * i + j] = positions[i][j];
		}
	}

	Y.resize(3 * nV);

	for (size_t i = 0; i < nV; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			//Y[3 * i + j] = positions[i][j] + h * velocity[i][j] ;
			Y[3 * i + j] = positions[i][j] + h * velocity[i][j] ;
		}
	}

	b.resize(3 * nV);

	int t = 5;
	while (t>0)
	{
		LocalStep();
		GlobalStep();
		t--;
	}

	//std::cout << X(0) << '\t' << X(1) << '\t' << X(2) << endl;

	for (size_t i = 0; i < nV; i++)
	{
		for (int j = 0; j < 3; j++) 
		{
			velocity[i][j] = (X[3 * i + j] - positions[i][j]) / h;
			positions[i][j] = X[3 * i + j];
		}
	}


}

void Simulate::ComputeA()
{
	std::vector<Eigen::Triplet<float>> coeffients;

	M .resize(3*nV,3*nV);
	for (int i = 0; i < nV; i++) 
	{
		if (!is_fixed[i])
		{
			coeffients.push_back(Triplet(3 * i, 3 * i, m));
			coeffients.push_back(Triplet(3 * i + 1, 3 * i + 1, m));
			coeffients.push_back(Triplet(3 * i + 2, 3 * i + 2, m));
		}
		else
		{
			coeffients.push_back(Triplet(3 * i, 3 * i, 1.0f));
			coeffients.push_back(Triplet(3 * i + 1, 3 * i + 1, 1.0f));
			coeffients.push_back(Triplet(3 * i + 2, 3 * i + 2, 1.0f));
		}
	}
	M.setFromTriplets(coeffients.begin(), coeffients.end());


	coeffients.clear();

	L.resize(3*nV, 3*nV);
	for (size_t k = 0; k < s; k++)
	{
		int i = edgelist[2 * k];
		int j = edgelist[2 * k + 1];

		if (!is_fixed[i])
		{
			for (int t = 0; t < 3; t++)
			{
				coeffients.push_back(Triplet(3 * i + t, 3 * i + t, stiff));
				coeffients.push_back(Triplet(3 * i + t, 3 * j + t, -stiff));
			}
		}

		if (!is_fixed[j])
		{
			for (int t = 0; t < 3; t++)
			{
				coeffients.push_back(Triplet(3 * j + t, 3 * i + t, -stiff));
				coeffients.push_back(Triplet(3 * j + t, 3 * j + t, stiff));
			}

		}
	}

	L.setFromTriplets(coeffients.begin(), coeffients.end());

	coeffients.clear();

	A = M + h * h * L;
	A.makeCompressed();

	solver.compute(A);
}

void Simulate::ComputeB()
{
	B.resize(3*nV, 3*s);

	std::vector<Eigen::Triplet<float>> coeffients;

	for (int k = 0; k < s; k++) 
	{
		int i = edgelist[2 * k];
		int j = edgelist[2 * k + 1];

		for (int t = 0; t < 3; t++) 
		{
			coeffients.push_back(Triplet(3 * i + t, 3 * k + t, -h * h * stiff));
			coeffients.push_back(Triplet(3 * j + t, 3 * k + t, h * h * stiff));
		}
	}

	B.setFromTriplets(coeffients.begin(), coeffients.end());
	

}


void Simulate::LocalStep()
{
	d.resize(3 * s);

	for (int k = 0; k < s; k++)
	{
		int i = edgelist[2 * k];
		int j = edgelist[2 * k + 1];
		
		Eigen::Vector3f dv = Eigen::Vector3f(X(3 * i) - X(3 * j), X(3 * i + 1) - X(3 * j + 1), X(3 * i + 2) - X(3 * j + 2));
		dv = dv / dv.norm();
		for (int t = 0; t < 3; t++)
		{
			d[3 * k + t] = springlist[k] * dv[t];
		}

	}
	//cout << "local step finish!" << endl;
}

void Simulate::GlobalStep()
{
	ComputeB();

	//cout << "compute B finish!" << endl;

	b = -(B * d - m * Y - h * h * F_ext);

	for (size_t i = 0; i < nV; i++)
	{
		if (is_fixed[i])
		{
			for (int j = 0; j < 3; j++)
			{
				b[3 * i + j] = X[3 * i + j];
			}
		}

	}

	X = solver.solve(b);
	//std::cout << X(0) << '\t' << X(1) << '\t' << X(2) << endl;

	//cout << "global step finish!" << endl;
}