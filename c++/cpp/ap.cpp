#include "ap.h"

AP::AP(int iterations, double lambda, int elements)
{
	cout << "Initializing ap.." << endl;
	mIter = iterations;
	mLambda = lambda;
	mElements = elements;
	cout << "Iterations: " << mIter << endl;
	cout << "Damping factor: " << mLambda << endl;
	cout << "Elements size: " << mElements << endl;
}

AP::~AP()
{
	delete mR;
	delete mA;
	delete mE;
	cout << "Finishing ap.." << endl;
}

void AP::SetSimMatrix(mat* hash)
{
	mS = hash;
}

void AP::Process()
{
	cout << "Processing AP.." << endl;
	AffinityPropagation();
}


void AP::AffinityPropagation()
{
	mR = mS->clone(0);
	mA = mS->clone(0);
	mE = mS->clone(0);

	cout << "Start AP iteration: " << endl;

 	struct timeval begin, end;

 	/*Measuring time elapsed*/
 	gettimeofday(&begin, NULL);

	for(int m=0; m < mIter; m++) {
		cout << "(" << m + 1 << "/" << mIter << ")" << endl;

		UpdateResponsability();
		UpdateAvailability();
	}
	cout << endl;

	gettimeofday(&end, NULL);
	int tmili = (int) (1000 * (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) / 1000);

	cout << "Time Elapsed: " << tmili << " (milis)" << endl;
	cout << "Time Elapsed: " << (float)tmili/1000 << " (s)" << endl;
}

void AP::UpdateResponsability()
{
	for(int i=0; i < mElements; i++) 
	{
		for(int k=0; k < mElements; k++)
		{
			double max = -1e100;
			for(int kk=0; kk<k; kk++)
			{
				if(mS->get(i,kk)+ mA->get(i,kk) > max)
					max = mS->get(i,kk) + mA->get(i,kk);
			}
			for(int kk=k+1; kk< mElements; kk++)
			{
				if(mS->get(i,kk)+ mA->get(i,kk)>max) 
					max = mS->get(i,kk)+mA->get(i,kk);
			}

			double resp = (1-mLambda)*(mS->get(i,k) - max) + mLambda*mR->get(i,k);
			mR->set(i,k,resp);
		}
	}
}

void AP::UpdateAvailability()
{

	for(int i=0; i< mElements; i++)
	{
		for(int k=0; k < mElements; k++)
		{
			if(i==k)
			{
				double sum = 0.0;
				for(int ii=0; ii<i; ii++)
					sum += max(0.0, mR->get(ii,k));
				for(int ii=i+1; ii < mElements; ii++)
					sum += max(0.0, mR->get(ii,k));
				double avail = (1-mLambda)*sum + mLambda*mA->get(i,k);
				mA->set(i,k,avail);
			} 
			else
			{
				double sum = 0.0;
				int maxik = max(i, k);
				int minik = min(i, k);
				for(int ii=0; ii<minik; ii++)
					sum += max(0.0, mR->get(ii,k));
				for(int ii=minik+1; ii<maxik; ii++)
					sum += max(0.0, mR->get(ii,k));
				for(int ii=maxik+1; ii<mElements; ii++)
					sum += max(0.0, mR->get(ii,k));
				
				double avail = (1-mLambda)*min(0.0, mR->get(k,k)+sum) + mLambda*mA->get(i,k);
				mA->set(i,k, avail);
			}
		}
	}
}