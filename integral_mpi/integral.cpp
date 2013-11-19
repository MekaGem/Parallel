#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <sstream>
#include <mpi.h>

const double MIN_X = -2;
const double MAX_X = 2;

double f(double x)
{
	assert(x >= MIN_X && x <= MAX_X);
	return sqrt(4 - x * x);
}

int getIntervals(char* arg)
{
	std::stringstream ss;
	ss << arg;
	int intervals = 1;
	ss >> intervals;
	return intervals;
}

int main(int argc, char** argv)
{
	// Init
	assert(argc > 1);
	const int intervals = getIntervals(argv[1]);
	assert(intervals > 0);
	const double dx = double(MAX_X - MIN_X) / intervals;

	int numprocs, rank, namelen;
  	char processor_name[MPI_MAX_PROCESSOR_NAME];

  	MPI_Init(&argc, &argv);
  	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Get_processor_name(processor_name, &namelen);	

  	// Integration
	int beingIndex = ((long long)intervals * rank) / numprocs;
	int endIndex = ((long long)intervals * (rank + 1)) / numprocs;

	double sum = 0;
	for (int index = beingIndex; index < endIndex; ++index) 
	{
		sum += dx * f(MIN_X + dx * index);
	}

	// Summary
	int step = 0;
	while (numprocs > 1)
	{
		if (rank > numprocs) break;
		int listeners = (numprocs + 1) / 2;
		int speakers = numprocs - listeners;

		// fprintf(stderr, "I'm process number %d\n", rank);

		double msg;
		if (rank < listeners && rank < speakers)
		{
			// fprintf(stderr, "I'm process %d and I'm listening\n", rank);
			MPI_Status status;
			MPI_Recv(&msg, 1, MPI_DOUBLE, listeners + rank, step, MPI_COMM_WORLD, &status);

			sum += msg;
		}
		else if (rank >= listeners)
		{
			// fprintf(stderr, "I'm process %d and I'm speaking\n", rank);
			MPI_Send(&sum, 1, MPI_DOUBLE, rank - listeners, step, MPI_COMM_WORLD);
		}

		numprocs = listeners;
		step++;
	}

	if (rank == 0)
	{
		std::cout << sum << std::endl;
	}
	MPI_Finalize();
}