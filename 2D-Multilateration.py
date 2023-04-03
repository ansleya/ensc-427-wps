from scipy.optimize import minimize
import numpy as np

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

	with open('test.txt', 'r') as f: 
		# read in all lines from the file 
		lines = f.readlines() 
		# initialize an empty matrix to store the values 
		matrix = [] 
		# loop over each line 
		for line in lines: 
			# split the line into a list of values 
			values = line.strip().split('\t') 
			# append the list of values to the matrix 
			matrix.append(values) 

	DistArr = []
	for n in range(len(matrix)):
		DistArr.append(float(matrix[n][3])*.1*3/2/4000)
	print(DistArr)
	
	

	Routers = list(np.array([[12.5,12.5], [-12.5,12.5], [12.5,-12.5], [-12.5,-12.5]]))
	distances_to_Routers = [8, 8, 9, 8]
	print(gps_solve(distances_to_Routers, Routers))