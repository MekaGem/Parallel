#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
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
	
	#pragma omp parallel shared(totalSum)
	{
		double sum = 0;
		#pragma omp for
		for (int index = 0; index < intervals; ++index) 
		{
			sum += dx * f(MIN_X + dx * index);
		}
		
		#pragma omp critical
		totalSum += sum;
	}

	std::cout << "Total sum: " << std::setprecision(10) << totalSum << std::endl;
	return 0;	
}
