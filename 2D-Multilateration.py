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

	np.set_printoptions(suppress=True)
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

	base_file_name = 'router'
	numRouters = 12
	numPings = int(float(actPos[0][3]))
	RoutersBaseMat = [[[0 for k in range(4)] for j in range(numPings+1)] for i in range(numRouters)]
	tempIter = 0

	# Router(matrix), Ping(line), data(element)
	
	for i in range(numRouters):
		filename = base_file_name + str(i) + '.txt'
		with open(filename, 'r') as f: 
			for line in f:
				values = line.strip().split('\t') 
				RoutersBaseMat[i][tempIter] = values
				tempIter +=1
				
		tempIter = 0


	numClients = int(float(actPos[0][0]))
	DistMat= np.zeros((numPings,14))
	RTTs = np.array(np.zeros((numRouters)))
	weight = 1.5
	offset = 500
	
	for i in range(numPings):

		DistMat[i][0] = i%int(numPings/numClients)+1
		DistMat[i][1] = int(i*numClients/numPings)
		for j in range(numRouters):
			RTTs[j] = RoutersBaseMat[j][i+1][3]

		idx_sorted = np.argsort(RTTs)

		sorted_Rtts = RTTs[idx_sorted]
		Router_sorted = np.arange(len(RTTs))

		count = 0	

		for j in range(len(RTTs)):
			if (sorted_Rtts[j] > 0 and count <4):
				DistMat[i][3*count+2] = float(sorted_Rtts[j])
				DistMat[i][3*count+3] = float(RoutersBaseMat[idx_sorted[j]][0][1])
				DistMat[i][3*count+4] = float(RoutersBaseMat[idx_sorted[j]][0][2])
				count += 1
		

		# index0 = int(int(float(Router0[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		# index1 = int(int(float(Router1[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		# index2 = int(int(float(Router2[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))
		# index3 = int(int(float(Router3[n][1]))*numPings/numClients+int(float(Router0[n][0])-1))

	for i in range(numPings):
		divnum = min(DistMat[i][2],DistMat[i][5],DistMat[i][8],DistMat[i][11])
		DistMat[i][2] = (float(DistMat[i][2])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][5] = (float(DistMat[i][5])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][8] = (float(DistMat[i][8])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][11] = (float(DistMat[i][11])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight

	actualX = []
	actualY = []
	rttX = []
	rttY = []
	PosMat= np.zeros((numPings,4))
	print(DistMat)

	for i in range(numPings):
		Routers = list(np.array([[DistMat[i][3],DistMat[i][4]], [DistMat[i][6],DistMat[i][7]], [DistMat[i][9],DistMat[i][10]], [DistMat[i][12],DistMat[i][13]]]))
		distances_to_Routers = [DistMat[i][2], DistMat[i][5], DistMat[i][8], DistMat[i][11]]
		a = gps_solve(distances_to_Routers, Routers)
		PosMat[i][0] = DistMat[i][1]
		PosMat[i][1] = DistMat[i][0]

		PosMat[i][2] = a[0]
		PosMat[i][3] = a[1]
		

		if((a[0] >= 0  and a[0] <= 150) and (a[1] > 0 and a[1] < 15) ):
			rttX.append(a[0])
			rttY.append(a[1])

	for i in range(len(actPos)):	
		actualX.append(float(actPos[i][1]))
		actualY.append(float(actPos[i][2]))
	
	# print(actualX)
	# print(actualY)
	# print(rttX)
	# print(rttY)
	print(PosMat)

	plt.plot(actualX,actualY,label = "Actual")
	plt.plot(rttX,rttY, label = "Calculated")
	plt.legend()
	plt.grid()
	plt.show()