from scipy.optimize import minimize
import numpy as np
import math
import sys
import matplotlib.pyplot as plt

# Solve Position using known position and distances to routers
def position_solve(distances_to_Routers, Router_coordinates):
	# referenced from : github.com/glucee/Multilateration
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
	np.set_printoptions(threshold= sys.maxsize)

# Read file positoin for the actual position 
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
# Read file positoin for the actual position (different from first as sometimes router fail)
	with open('Actual-Pos.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		Graphing = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			Graphing.append(values) 

	base_file_name = 'router'
	numRouters = 12
	numPings = int(float(actPos[0][3]))
	RoutersBaseMat = [[[0 for k in range(4)] for j in range(numPings+1)] for i in range(numRouters)]
	tempIter = 0

	numClients = int(float(actPos[0][0]))
	# Router(matrix), Ping(line), data(element)
	# Open and store all data in a 3-D matrix for all the routers used in simulation
	# Have to make sure the the ping for a specific client lines up with other routers
	#i.e: each router has a ping from client N on line M in matrix
	for i in range(numRouters):
		filename = base_file_name + str(i) + '.txt'
		with open(filename, 'r') as f: 
			for line in f:
				values = line.strip().split('\t') 
				index = int((int(float(values[0]))-1)*numClients+int(float(values[1]))+1)
				if tempIter == 0:
					
					RoutersBaseMat[i][0] = values
				else:
					RoutersBaseMat[i][index] = values

				tempIter +=1

		tempIter = 0
	
	DistMat= np.zeros((numPings,14))
	RTTs = np.array(np.zeros((numRouters)))
	
	# Arrange data found in text files to coordinate with same ping from other client from other routers
	# Also 
	for i in range(numPings):

		DistMat[i][0] = i%int(numPings/numClients)+1 	# the time for each ping
		DistMat[i][1] = int(i*numClients/numPings)		# the client number
		for j in range(numRouters):					
			RTTs[j] = RoutersBaseMat[j][i+1][3]	#find all the values of the rtts on the same line for each router

		# Sort the routers numbers and RTT values from smallest to largest
		idx_sorted = np.argsort(RTTs)
		sorted_Rtts = RTTs[idx_sorted]

		RouterArr = np.zeros(4)
		count = 0	

		#take the smallest non zero RTT and store the value
		for j in range(len(RTTs)):
			if (sorted_Rtts[j] > 0 and count <4):
				
				RouterArr[count] = idx_sorted[j]
				index = int(int(float(RoutersBaseMat[idx_sorted[j]][i+1][1]))*numPings/numClients+int(float(RoutersBaseMat[idx_sorted[j]][i+1][0])-1))
				DistMat[index][3*count+2] = float(sorted_Rtts[j])
				DistMat[index][3*count+3] = float(RoutersBaseMat[idx_sorted[j]][0][1])
				DistMat[index][3*count+4] = float(RoutersBaseMat[idx_sorted[j]][0][2])
				count += 1

	
	# calculating the distance using the given RTTS
	weight = 1.5	#some scaling was required to fit actual position better
	offset = 500 	#how much offset we want to tak off smaller = more 
	for i in range(numPings):
		divnum = min(DistMat[i][2],DistMat[i][5],DistMat[i][8],DistMat[i][11])
		DistMat[i][2] = (float(DistMat[i][2])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][5] = (float(DistMat[i][5])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][8] = (float(DistMat[i][8])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight
		DistMat[i][11] = (float(DistMat[i][11])-int(float(divnum)/offset)*offset)*.1*2.99/2/weight

	rttX = np.zeros((numClients,int(numPings/numClients)))
	rttY = np.zeros((numClients,int(numPings/numClients)))
	rttTime = np.zeros((numClients,int(numPings/numClients)))
	PosMat= np.zeros((numPings,4))
	prevX = 0
	prevY = 0
	
	# Calculation the position of the position on the client using the distnaces and router positions
	for k in range(numClients):
		for j in range(int(numPings/numClients)):
			i = int(k*numPings/numClients+j)

			Routers = list(np.array([[DistMat[i][3],DistMat[i][4]], [DistMat[i][6],DistMat[i][7]], [DistMat[i][9],DistMat[i][10]], [DistMat[i][12],DistMat[i][13]]]))
			distances_to_Routers = [DistMat[i][2], DistMat[i][5], DistMat[i][8], DistMat[i][11]]
			a = position_solve(distances_to_Routers, Routers)
			PosMat[i][0] = DistMat[i][0]
			PosMat[i][1] = DistMat[i][1]
			PosMat[i][2] = a[0]
			PosMat[i][3] = a[1]
			rttTime[k][j] = int(float(DistMat[i][0]))

			# If the calculated position is not in the bounds repeat the previous value
			if( i < 3 and (a[0] > 0  and a[0] < 150) and (a[1] > 0 and a[1] < 15)):
				rttX[k][j] = a[0] 
				rttY[k][j] = a[1]
				prevX = a[0]
				prevY = a[1]
			elif(((a[0] > 0  and a[0] < 150) and (a[1] > 0 and a[1] < 15) and abs(prevX-a[0])<10 and abs(prevY-a[1]) <10)):

				rttX[k][j] = a[0] 
				rttY[k][j] = a[1]
				prevX = a[0]
				prevY = a[1]

			else:
				rttX[k][j] = prevX
				rttY[k][j] = prevY

	actualX = []
	actualY = []
	actualTime = []
	prevTime = 0
	curTime = 0
	# Putting the actual values of the mobility model inside an array
	for i in range(len(Graphing)-1):	
		curTime = int(float(Graphing[i+1][0]))
		if (curTime != prevTime):
			actualX.append(float(Graphing[i+1][1]))
			actualY.append(float(Graphing[i+1][2]))
			actualTime.append(int(float(Graphing[i+1][0])))
			prevTime = curTime

	RTTx0 = []
	RTTy0 = []
	# Grabbing the whole array of the first client and ignoring 0 values
	for i in range(len(rttX[0])):
		if (rttX[0][i]>0 and rttY[0][i]>0):
			RTTx0.append(rttX[0][i])
			RTTy0.append(rttY[0][i])

	meanXAccuracy = 0
	meanYAccuracy = 0
	
	# Calculating the average directional error from the actual point to the calculated point
	for i in range(len(actualX)):
			meanXAccuracy += abs(rttX[0][i]- actualX[i])
			meanYAccuracy += abs(rttY[0][i]- actualY[i])
	meanXAccuracy = meanXAccuracy/len(actualX)
	meanYAccuracy = meanYAccuracy/len(actualY)

	# Calculating the mean scraed error for distance from the point
	mean2Acc = math.sqrt(meanXAccuracy**2+meanYAccuracy**2)
	print(mean2Acc)

	# for i in range(numClients):
	# 	name = 'Client' + str(i)
	# 	plt.plot(rttX[i],rttY[i], label = name)

	# Plotting results 
	plt.plot(RTTx0,RTTy0,label = "Client0")
	plt.plot(actualX,actualY,label = "Actual")
	plt.xlabel("X-position [m]")
	plt.ylabel("Y-position [m]")
	plt.legend()
	plt.grid()
	plt.show()

