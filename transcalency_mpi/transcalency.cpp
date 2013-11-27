#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <iomanip>
#include <mpi.h>

const double A = 0;
const double B = 1;

const double a = 0;
const double b = 1;

const double k = 0.3;

const int CHUNK_SIZE = 50;

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

	int numprocs, rank, namelen;
  	char processor_name[MPI_MAX_PROCESSOR_NAME];

  	MPI_Init(&argc, &argv);
  	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Get_processor_name(processor_name, &namelen);	

  	int beingIndex = ((long long)(N - 2) * rank) / numprocs;
	int endIndex = ((long long)(N - 2) * (rank + 1)) / numprocs;

	for (int iteration = 0; iteration < iterations; ++iteration) 
	{
		for (int index = 1 + beingIndex; index < 1 + endIndex; ++index)
		{
			//std::cerr << "thread is " << omp_get_thread_num() << std::endl;
			u1[index] = u0[index] + k * (u0[index - 1] - 2 * u0[index] + u0[index + 1]);
		}		 

		for (int even = 0; even < 2; ++even)
		{
			if ((even ^ (rank % 2)) == 0)
			{
				if (rank < numprocs - 1)
				{
					MPI_Send(&u1[endIndex], 1, MPI_DOUBLE, rank + 1, iteration * 2 + even, MPI_COMM_WORLD);
				}
				if (rank > 0)
				{
					MPI_Send(&u1[1 + beingIndex], 1, MPI_DOUBLE, rank - 1, iteration * 2 + even, MPI_COMM_WORLD);
				}
			}
			else
			{
				if (rank > 0)
				{
					MPI_Recv(&u1[beingIndex], 1, MPI_DOUBLE, rank - 1, iteration * 2 + even, MPI_COMM_WORLD, NULL);
					// MPI_Send(&u1[1 + beingIndex], 1, MPI_DOUBLE, rank - 1, iteration * 2 + even, MPI_COMM_WORLD);
				}
				if (rank < numprocs - 1)
				{
					MPI_Recv(&u1[1 + endIndex], 1, MPI_DOUBLE, rank + 1, iteration * 2 + even, MPI_COMM_WORLD, NULL);
				}					
			}
		}

		{
			u0.swap(u1);
		}
	}	

	if (N < 10)
	{
		int message = 42;

		if (rank > 0)
		{
			MPI_Recv(&message, 1, MPI_INT, rank - 1, iterations * 2, MPI_COMM_WORLD, NULL);
		}

		if (rank == 0)
		{
			std::cerr << std::setprecision(8) << std::fixed << a + h * 0 << " " << u0[0] << std::endl;
		}
		for (int index = 1 + beingIndex; index < 1 + endIndex; ++index)
		{
			std::cerr << std::setprecision(8) << std::fixed << a + h * index << " " << u0[index] << std::endl;
		}
		if (rank == numprocs - 1)
		{
			std::cerr << std::setprecision(8) << std::fixed << a + h * (N - 1) << " " << u0[N - 1] << std::endl;
		}

		if (rank < numprocs - 1)
		{
			MPI_Send(&message, 1, MPI_INT, rank + 1, iterations * 2, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
	return 0;
}
