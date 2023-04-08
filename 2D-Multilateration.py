from scipy.optimize import minimize
import numpy as np
import matplotlib.pyplot as plt

def gps_solve(distances_to_Routers, Router_coordinates):
	def error(x, c, r):
		return sum([(np.linalg.norm(x - c[i]) - r[i]) ** 2 for i in range(len(c))])

	l = len(Router_coordinates)
	S = sum(distances_to_Routers)
	# compute weight vector for initial guess
	W = [((l - 1) * S) / (S - w) for w in distances_to_Routers]
	# get initial guess of point location
	x0 = sum([W[i] * Router_coordinates[i] for i in range(l)])
	# optimize distance from signal origin to border of spheres
	return minimize(error, x0, args=(Router_coordinates, distances_to_Routers), method='Nelder-Mead').x



        
if __name__ == "__main__":

	with open('client0-pos.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		actPos = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			actPos.append(values) 

	with open('router0.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		Router0 = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			Router0.append(values) 

	with open('router1.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		Router1 = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			Router1.append(values) 

	with open('router2.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		Router2 = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			Router2.append(values) 

	with open('router3.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		Router3 = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			Router3.append(values) 

	numPings = len(Router0)-1
	numClients = int(float(Router0[0][0]))
	DistMat= np.zeros((numPings,6))


	for i in range(numPings):
		n = int(i+1)
		DistMat[i][0] = i%int(numPings/numClients)+1
		DistMat[i][1] = int(i*numClients/numPings)
		index0 = int(int(float(Router0[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		index1 = int(int(float(Router1[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		index2 = int(int(float(Router2[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		index3 = int(int(float(Router3[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))

		DistMat[index0][2] = float(Router0[n][3])
		DistMat[index1][3] = float(Router1[n][3])
		DistMat[index2][4] = float(Router2[n][3])
		DistMat[index3][5] = float(Router3[n][3])

	weight = 1.5
	offset = 600
	for i in range(numPings):
		divnum = min(DistMat[i][2],DistMat[i][3],DistMat[i][4],DistMat[i][5])
		DistMat[i][2] = (float(DistMat[i][2])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][3] = (float(DistMat[i][3])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][4] = (float(DistMat[i][4])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][5] = (float(DistMat[i][5])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight

	print(DistMat)
	np.set_printoptions(suppress=True)

	print(DistMat)
	actualX = []
	actualY = []
	rttX = []
	rttY = []
	PosMat= np.zeros((numPings,4))
	Routers = list(np.array([[12.5,12.5], [-12.5,12.5], [12.5,-12.5], [-12.5,-12.5]]))
	for i in range(numPings):
		distances_to_Routers = [DistMat[i][2], DistMat[i][3], DistMat[i][4], DistMat[i][5]]
		a = gps_solve(distances_to_Routers, Routers)
		PosMat[i][0] = DistMat[i][1]
		PosMat[i][1] = DistMat[i][0]

		PosMat[i][2] = a[0]
		PosMat[i][3] = a[1]
		

		if(not(abs(a[0]) > 15 or abs(a[1]) > 15)):
			rttX.append(a[0])
			rttY.append(a[1])

	for i in range(len(actPos)):	
		actualX.append(float(actPos[i][1]))
		actualY.append(float(actPos[i][2]))
	
	# print(actualX)
	# print(actualY)
	# print(rttX)
	# print(rttY)
	# print(PosMat)

	plt.plot(actualX,actualY,label = "Actual")
	plt.plot(rttX,rttY, label = "Calculated")
	plt.legend()
	plt.grid()
	plt.show()