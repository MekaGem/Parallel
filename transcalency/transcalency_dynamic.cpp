#include <iostream>
#include <omp.h>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <iomanip>

const double A = 0;
const double B = 1;

const double a = 0;
const double b = 1;

const double k = 0.3;

const int CHUNK_SIZE = 100;

void parseArguments(int argc, char** argv, int& N, double& T)
{
	std::stringstream ss;
	assert(argc >= 3);
	ss << argv[1] << " ";
	ss << argv[2];
	ss >> N;
	ss >> T;
}

int main (int argc, char** argv) 
{
	int N;
	double T;
	parseArguments(argc, argv, N, T);
	double h = (b - a) / N;
	double tau = k * h * h;
	int iterations = (int)std::floor(T / tau);
	std::vector<double> u0(N, 0), u1(N, 0);	

	u0[0] = A;
	u0[N - 1] = B;
	u1[0] = A;
	u1[N - 1] = B;

	#pragma omp parallel 
	{
		for (int iteration = 0; iteration < iterations; ++iteration) 
		{
			#pragma omp for schedule(dynamic) 
			for (int index = 1; index < N - 1; ++index)
			{
				//std::cerr << "thread is " << omp_get_thread_num() << std::endl;
				u1[index] = u0[index] + k * (u0[index - 1] - 2 * u0[index] + u0[index + 1]);
			}
			
			#pragma omp barrier
			#pragma omp single
			{
				u0.swap(u1);
			}
		}		 
	}	
	if (N < 10)
	{
		for (int index = 0; index < N; ++index)
		{
			std::cerr << std::setprecision(8) << std::fixed << a + h * index << " " << u0[index] << std::endl;
		}
	}
	return 0;
}
