# from sklearn.cluster import AffinityPropagation
from sklearn.cluster import KMeans
from scipy.sparse import coo_matrix
from numpy import *
import sys
import operator

def readSimMatrix(simMatPath):
	i = array([])
	j = array([])
	data = array([])
	counter = 0;
	accum = 0;

	print "[INFO] Reading sim matrix.."
	# with open("Similarities_toy_Sim.txt") as f:
	# with open("Similarities.txt") as f:
	with open(simMatPath) as f:
		content = f.readlines()
	f.close()
	# content = [line.strip().split() for line in content]
	for line in content:
		line = line.strip()
		line = line.split()
		i = append(i, int(line[0]) - 1)
		j = append(j, int(line[1]) - 1)
		value = float(line[2])
		data = append(data, value)
		accum += value;
		counter += 1
	
	return i,j,data

def generateClustersFile(kmeans, times):
	# indices = kmeans.cluster_centers_indices_
	labels = kmeans.labels_
	labels_len = len(labels)

	clusters = {}
	count_cluster = 0;

	for i in range(labels_len):
		if(labels[i] in clusters):
			c = clusters[labels[i]]
			c.append(i)
		else:
			c = [i]

		clusters[labels[i]] = c

	with open("clusters"+ str(times) +".dat", "w") as file:
		for i in range(len(clusters)):
			file.write(str(i))
			users = clusters[i]
			for u in users:
				file.write(" " + str(u))
				# print " " + str(u)
			file.write("\n")
		file.close()
	return clusters


def main(simMatPath, ncluster):
	print "[INFO] Number of clusters: ", ncluster

	i,j,data = readSimMatrix(simMatPath)
	# convert to sparse matrix
	simMatrix = coo_matrix((data,(i, j)))

	for x in xrange(1,11):
		# clustering method
		print("[INFO] Processing K means -> ", x)
		kmeans = KMeans(n_clusters=int(ncluster), init='random').fit(simMatrix)
		
		# generate clusters.dat
		print("[INFO] Clusters created -> ", x)	
		clusters = generateClustersFile(kmeans, x)
		print clusters

if __name__ == '__main__':
	if(len(sys.argv) != 3):
		print "[ERROR] Missing arguments"
		print "$ python whatever.py <SIMILARITY_MATRIX_PATH> <N_CLUSTERS>"
		exit(1)
	main(sys.argv[1], sys.argv[2])