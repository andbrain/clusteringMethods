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

def readRatings(ratingsPath):
	# dict of users
	# standard => user1 = [
	# 	[movie1, ratingOfMovie1],
	# 	[movie2, ratingOfMovie2],
	# 	.
	# 	..
	# 	[movieN, ratingOfMovieN]
	# ]
	ratings = {}
	print "[INFO] Reading user ratings.."

	with open(ratingsPath) as f:
		content = f.readlines()
	f.close()

	user = ""

	for i in range(0, len(content)):
		line = content[i]
		line = line.strip().split()
		if(i % 2 == 0):
			user = int(line[0])
			ratings[user] = []
		else:
			rat = []
			for j in range(0, len(line), 2):
				rat.append([int(line[j]), float(line[j+1])]) 
			ratings[user] = rat
	
	return ratings

def generateClustersFile(kmeans):
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

	with open("clusters.dat", "w") as file:
		for i in range(len(clusters)):
			file.write(str(i))
			users = clusters[i]
			for u in users:
				file.write(" " + str(u))
				# print " " + str(u)
			file.write("\n")
		file.close()
	return clusters

def accumulateRatings(clusters, totalRatings):
	# list of clusters -> list of movies -> list of ratings
	accumulator = {}
	for cluster, users in clusters.items():
		accumCluster = {}
		# print cluster, users
		for user in users:
			userRatings = totalRatings[user];
			# print user, userRatings
			for rating in userRatings:
				movie = rating[0]
				rat = rating[1]

				if(movie in accumCluster):
					m = accumCluster[movie]
					m.append(rat)
				else:
					m = [rat]
				accumCluster[movie] = m
		accumulator[cluster] = accumCluster

	return accumulator

def representativeByFreq(ratings):
	values = {}

	for rating in ratings:
		if(rating in values):
			values[rating] += 1
		else:
			values[rating] = 1
	sortedRatings = sorted(values.items(), key=operator.itemgetter(1), reverse=True)
	
	# TODO:: Check if it returns 0 when 1st != 2nd element, and if the 2nd > 1(freq)
	if(len(sortedRatings) <= 1):
		return sortedRatings[0][0]
	if(sortedRatings[0][1] != sortedRatings[1][1] and sortedRatings[1][1] <= 1):
		return sortedRatings[0][0]
	else:
		return 0

def generateReprByFrequence(ratingsAccumulated):
	with open("recFreq.dat", "w") as file:
		for cluster, movies in ratingsAccumulated.items():
			file.write(str(cluster) + "\n")

			for movie,ratings in movies.items():				
				result = representativeByFreq(ratings)
				if(result != 0):
					file.write(str(movie) + " " + str(int(result)) + "\n")
		file.close()

def representativeByMean(ratings):
	# p as median element
	p = 3
	vHigh = []
	vLow = []

	for rating in ratings:
		if(rating < p):
			vLow.append(rating)
		elif(rating > p):
			vHigh.append(rating)

	if(len(vLow) == len(vHigh)):
		return 0
	elif(len(vLow) > len(vHigh)):
		mean = vLow
	else:
		mean = vHigh

	acc = 0
	for res in mean:
		acc += res

	return acc/len(mean)

def generateReprByMean(ratingsAccumulated):
	with open("recMean.dat", "w") as file:
		for cluster, movies in ratingsAccumulated.items():
			file.write(str(cluster) + "\n")

			for movie,ratings in movies.items():				
				result = representativeByMean(ratings)
				if(result != 0):
					file.write(str(movie) + " " + str(round(result,2)) + "\n")
		file.close()

def representativeByAverage(ratings):
	# p as median element
	average = 0.0

	for rating in ratings:
		average+=rating
	
	return average/size(ratings)

def generateReprByAverage(ratingsAccumulated):
	with open("recAverage.dat", "w") as file:
		for cluster, movies in ratingsAccumulated.items():
			file.write(str(cluster) + "\n")

			for movie,ratings in movies.items():				
				result = representativeByAverage(ratings)
				if(result != 0):
					file.write(str(movie) + " " + str(round(result,2)) + "\n")
		file.close()

def main(simMatPath, ratingsPath, ncluster):
	print "[INFO] Number of clusters: ", ncluster

	i,j,data = readSimMatrix(simMatPath)
	# convert to sparse matrix
	simMatrix = coo_matrix((data,(i, j)))
	# clustering method
	print "[INFO] Processing K means.."
	kmeans = KMeans(n_clusters=int(ncluster)).fit(simMatrix)
	
		# generate clusters.dat
	print "[INFO] Clusters created.."	
	clusters = generateClustersFile(kmeans)

	# read ratings
	ratings = readRatings(ratingsPath)

	### generate representative clusters ###
	# accumulate clusters
	ratingsAccumulated = accumulateRatings(clusters, ratings)
	# generate repr. clusters by frequence
	print "[INFO] Created representative cluster by frequence.."	
	generateReprByFrequence(ratingsAccumulated)
	
	# generate repr. clusters by mean
	print "[INFO] Created representative cluster by mean of more frequents.."	
	generateReprByMean(ratingsAccumulated)
	
	# generate repr. clusters by average
	print "[INFO] Created representative cluster by mean.."	
	generateReprByAverage(ratingsAccumulated)

	print '[INFO] *************** Clusters Information ***************'
	# print '[INFO] Clusters indices: ', af.cluster_centers_indices_
	print '[INFO] Clusters labels: ', kmeans.labels_
	# print '[INFO] Number of clusters: %d' % len(af.cluster_centers_indices_)

if __name__ == '__main__':
	if(len(sys.argv) != 4):
		print "[ERROR] Missing arguments"
		print "$ python ap.py SIMILARITY_MATRIX_PATH RATINGS_PATH N_CLUSTERS"
		exit(1)
	main(sys.argv[1], sys.argv[2], sys.argv[3])