#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>
#include <sstream>
#include <omp.h>

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

int main (int argc, char** argv)
{
	assert(argc > 1);
	const int intervals = getIntervals(argv[1]);
	assert(intervals > 1);
	const double dx = double(MAX_X - MIN_X) / intervals;
	double totalSum = 0;

	std::vector<int> startInterval;
	std::vector<int> length;

	#pragma omp parallel shared(totalSum, startInterval, length)
	{
		#pragma omp single
		{
			int intervalsLeft = intervals;
			int numThreads = omp_get_num_threads();
			
			startInterval.resize(numThreads);
			length.resize(numThreads);		

			startInterval[0] = 0;
			length[0] = intervalsLeft / numThreads;
			intervalsLeft -= length[0];

			for (int threadId = 1; threadId < numThreads; ++threadId)
			{
				length[threadId] = intervalsLeft / (numThreads - threadId);
				startInterval[threadId] = startInterval[threadId - 1] + length[threadId - 1];
				intervalsLeft -= length[threadId];
			}
		}
		double sum = 0;
		int first = startInterval[omp_get_thread_num()];
		int len = length[omp_get_thread_num()];
		for (int index = first; index < first + len; ++index) 
		{
			sum += dx * f(MIN_X + dx * index);
		}
		
		#pragma omp critical
		totalSum += sum;
	}

	std::cout << "Total sum: " << std::setprecision(10) << totalSum << std::endl;
	return 0;	
}
