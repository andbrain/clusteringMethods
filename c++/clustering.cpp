#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include "include/mat.h"
#include "include/ap.h"
#include "include/sparse_ap.h"
#include "include/graph.h"

using namespace std;

string sim_matrix_path;
string ratings_path;
int pref_type;
int data_points, maxit, convit;
double damping;
Graph *R;
map<int,vector<int>> clusters;


void getDatasetName(int argc, char *argv[]);
void readSimilarities(const char* dfn, mat *S);
void readRatings(const char* dfn, Graph *R);
void PrintCluster(vector<int> examplar);
void OutputClusters(string filename);

void CalculateRepresentative(string reprFreq, string reprMean);
map<int, map<int, vector<double>> > AccumulateRatings();
void MakeRepresentativeByFrequency(string repr,map<int, map<int, vector<double>>> *accumList);
void MakeRepresentativeByMean(string repr,map<int, map<int, vector<double>>> *accumList);

int main(int argc, char *argv[])
{
	getDatasetName(argc, argv);
	mat *S = new mat(data_points);
	readSimilarities(sim_matrix_path.c_str(), S);
  	// Delete S inside the method
	cout << "** Sparse Affinity Propagation **" << endl;	
  	
  	vector<int> examplar = affinityPropagation(S, pref_type, damping, maxit, convit);
	// delete S;
	
	R = new Graph();
	readRatings(ratings_path.c_str(), R);
	
	PrintCluster(examplar);
	OutputClusters("clusters.dat");
	CalculateRepresentative("reprFreq.dat","reprMean.dat");
	delete R;
	return 0;
}

void getDatasetName(int argc, char *argv[])
{
	if(argc == 8)
	{
		sim_matrix_path = argv[1];
		ratings_path = argv[2];
		data_points = atoi(argv[3]);
		pref_type = atoi(argv[4]);
		damping = stod(argv[5]);
		maxit = atoi(argv[6]);
		convit = atoi(argv[7]);
	}
	else if(argc == 4)
	{
		sim_matrix_path = argv[1];
		ratings_path = argv[2];
		data_points = atoi(argv[3]);
		pref_type = 1;
		damping = 0.9;	
		maxit = 1000;
		convit = 50;
	}
	else
	{
		cout << "[Error] Missing parameters!" << endl;
		cout << "[INFO] ap.out <SIM_MATRIX_PATH> <RATINGS_MATRIX_PATH> <QTD_DATA_POINTS> [ <PREF_TYPE=1> <DAMPING=0.9> <MAXIT=1000> <CONVIT=50> ]" << endl;
		cout << "[PARAMS]:" << endl;
		cout << "SIM_MATRIX_PATH -> Similarities file path" << endl;
		cout << "RATINGS_MATRIX_PATH -> Ratings file path" << endl;
		cout << "QTD_DATA_POINTS -> Number of data points in similarity matrix" << endl;
		cout << "PREF_TYPE -> AP Preferences:" << endl;
		cout << "\t1: use median of similarities as preference" << endl;
		cout << "\t2: use minimum of similarities as preference" << endl;
		cout << "\t3: use min - (max - min) of similarities as preference" << endl;
		cout << "MAXIT -> The maximum number of iterations." << endl;
		cout << "DAMPING -> The damping factor. (0.5 <= damping < 1.0)" << endl;
		cout << "CONVIT -> Specify how many iterations this algorithm stops when examplars did not change for." << endl;

		exit(1);
	}

	cout << "[INFO] Number of data points: " << data_points << endl;
	cout << "[INFO] Damping Factor: " << damping << endl;
	cout << "[INFO] Max of iterations: " << maxit << endl;
	cout << "[INFO] Convit: " << convit << endl;
}

void readSimilarities(const char* dfn, mat *S) {
		
	int j, k;
	double val;
	vector<double> tmpS;
	
	ifstream myfile(dfn);
	//compute preferences for all data points 
	while(myfile >> j >> k >> val) {
		j--;
		k--;
		S->set(j,k,val);
		S->set(k,j,val);
		tmpS.push_back(S->get(j,k)); 
	}
	myfile.close();
}

void readRatings(const char* dfn, Graph *R)
{
	int i, j;
	double rat, aux;
	string line;
	
	ifstream myfile(dfn);
	//compute all ratings 
	while(myfile >> i) {
		if(!R->HasVertex(i))
			R->AddVertex(i);
		do
		{
			myfile >> j >> rat;
			R->AddEdge(i,j,rat);
		}while(myfile.peek() != '\n' && myfile.peek() != EOF);
	}
	myfile.close();
}

void PrintCluster(vector<int> examplar)
{

	for(int i=0; i< data_points; i++) {

		map<int,vector<int>>::iterator itC = clusters.find(examplar[i]);

		if(itC != clusters.end())
		{
			itC->second.push_back(i);
		}
		else
		{
			vector<int> vCluster;
			vCluster.push_back(i);
			clusters.insert(make_pair(examplar[i], vCluster));
		}
	}

	cout << endl;
	cout << "***** Clusters Information ****" << endl;	
	map<int, vector<int>>::iterator it = clusters.begin();

	// vector<int> vec;
	// for (; it != clusters.end(); ++it)
	// {
	// 	cout << "Cluster " << it->first << ":";
	// 	cout << "[";
	// 	vec = it->second;

	// 	for (std::vector<int>::iterator i = vec.begin(); i != vec.end(); ++i)
	// 	{
	// 		cout  << " " << (*i);
	// 	}

	// 	cout << " ]" << endl;
	// }

	cout << "Total of clusters: " << clusters.size() << endl;
	cout << "*****************************" << endl;	
}

void OutputClusters(string filename)
{
	/*
		Standard of output

		cluster1 document1 document2
		cluster4 document3 document4
		.
		.
		.
		clusterN documentX documentY ...
	 */

	fstream fs(filename, ios::out);
	vector<int> vec;

	for (map<int, vector<int>>::iterator it = clusters.begin(); it!=clusters.end(); ++it)
	{
		vec = it->second;
		fs << it->first;
		for (vector<int>::iterator itDocs = vec.begin(); itDocs!=vec.end(); ++itDocs)
			fs << " " << *(itDocs);
		fs << endl;
	}

	fs.close();
	cout << "[NEW] >> " << filename << endl;
}


void CalculateRepresentative(string reprFreq, string reprMean)
{
	map<int, map<int, vector<double>>> accumList = AccumulateRatings();

	MakeRepresentativeByFrequency(reprFreq,&accumList);
	MakeRepresentativeByMean(reprMean,&accumList);
}

map<int, map<int, vector<double>> > AccumulateRatings()
{
	map<int, map<int, vector<double>> > accClusterRatings; //<cluster, <movie, vector of ratings> >
	map<int,vector<int>>::iterator it = clusters.begin();
	map<int,vector<double>>::iterator foundMov;
	//run over all clusers
	for(; it != clusters.end(); ++it)
	{
		map<int, vector<double> > accRatings; // <movie, vector of ratings>
		vector<int> users;
		users = it->second;

		Vertex *v;
		//get user ratings
		for (vector<int>::iterator i = users.begin(); i != users.end(); ++i)
		{
			v = R->at((*i));

			//for each movie rated by user
			Edge::iterator itAux;
			for(itAux = v->begin(); itAux!= v->end(); ++itAux)
			{
				vector<double> accVec;
				foundMov = accRatings.find(itAux->first);

				//if found movie, add rating
				if(foundMov != accRatings.end())
				{
					accVec = foundMov->second;
					accVec.push_back(itAux->second);
					foundMov->second = accVec;
				}
				else
				{

					accVec.push_back(itAux->second);
					accRatings[itAux->first] = accVec;
				}
			}
		}

		//add in cluster acc
		accClusterRatings[it->first] = accRatings;
	}


	return accClusterRatings;
}

bool funcDec (PairValue i,PairValue j) { return (i.freq > j.freq); }

void MakeRepresentativeByFrequency(string representative_freq,map<int, map<int, vector<double>>> *accumList)
{
	fstream fs(representative_freq, ios::out);
	
	for (map<int, map<int, vector<double>> >::iterator it = accumList->begin(); it != accumList->end(); ++it)
	{
		fs << it->first << endl;
		map<int, vector<double>> movieAux = it->second;

		for(map<int, vector<double>>::iterator it2 = movieAux.begin(); it2 != movieAux.end(); ++it2)
		{
			// calculate highest frequency of rating
			vector<double> userRatings = it2->second;
			map<double,int> counter;
			map<double,int>::iterator itCounter;

			for (std::vector<double>::iterator i = userRatings.begin(); i != userRatings.end(); ++i)
			{
				itCounter = counter.find((*i));

				if(itCounter == counter.end())
				{
					counter[(*i)] = 1;
				}
				else
				{
					itCounter->second++;
				}
			}

			vector<PairValue> vecMax;

			for (itCounter = counter.begin(); itCounter != counter.end(); ++itCounter)
			{
				PairValue pv;
				pv.index = itCounter->first;
				pv.freq = itCounter->second;
				vecMax.push_back(pv);
			}

			sort(vecMax.begin(), vecMax.end(), funcDec);

			if(vecMax[0].freq != vecMax[1].freq)
				fs << it2->first << " " << vecMax[0].index << endl;
		}
	}

	fs.close();
	cout << "[NEW] >> " << representative_freq << endl;
}

void MakeRepresentativeByMean( string repr, map<int, map<int, vector<double>>> *accumList)
{
	fstream fs(repr, ios::out);
	int upperBound; //value to validate mean of ratings
	int p = 3; //parameter for delimite what is highest ratings or lowest ratings
	double alpha = 0.6;

	for (map<int, map<int, vector<double>> >::iterator it = accumList->begin(); it != accumList->end(); ++it)
	{
		fs << it->first << endl;
		map<int, vector<double>> movieAux = it->second;

		// get number of users for actual cluster
		vector<int> users = clusters.at(it->first);
		
		upperBound = users.size() * alpha;

		for(map<int, vector<double>>::iterator it2 = movieAux.begin(); it2 != movieAux.end(); ++it2)
		{
			vector<double> userRatings = it2->second;
			// calculate frequency of rating highest than p
			map<double,int> counterHighest;
			// calculate frequency of rating lowest than p
			map<double,int> counterLowest;
			// the most frequency ratings for cluster
			map<double,int> counter;
			map<double,int>::iterator itCounter;

			for (std::vector<double>::iterator i = userRatings.begin(); i != userRatings.end(); ++i)
			{
				if(*i > p)
				{
					itCounter = counterHighest.find((*i));

					if(itCounter == counterHighest.end())
						counterHighest[(*i)] = 1;
					else
						itCounter->second++;
				}
				else if(*i < p)
				{
					itCounter = counterLowest.find((*i));

					if(itCounter == counterLowest.end())
						counterLowest[(*i)] = 1;
					else
						itCounter->second++;
				}
			}

			if(counterHighest.size() == counterLowest.size())
				continue;
			else if( counterHighest.size() > counterLowest.size() )
				counter = counterHighest;
			else
				counter = counterLowest;

			double reprValue = 0;
			int elements = 0;

			for (itCounter = counter.begin(); itCounter != counter.end(); ++itCounter)
			{
				reprValue += itCounter->first * itCounter->second;
				elements += itCounter->second;
			}

			reprValue = reprValue / elements;

			if(elements >= upperBound)
				fs << it2->first << " " << reprValue << endl;
		}
	}

	fs.close();
	cout << "[NEW] >> " << repr << endl;
}