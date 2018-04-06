#ifndef AP_H
#define AP_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <cmath>
#include <algorithm>

#include <ctime>
#include <sys/time.h>
#include "mat.h"

using namespace std;

struct PairValue
{
	double index;
	int freq;
};
	
class AP
{
public:
	AP(int iterations, double lambda, int elements);
	~AP();
	void SetSimMatrix(mat* graph);
	void Process();
private:
	mat *mS, *mR, *mA, *mE; 
	int mIter, mElements;
	double mLambda;
	void AffinityPropagation();
	void UpdateResponsability();
	void UpdateAvailability();
};

#endif