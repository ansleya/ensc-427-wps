from scipy.optimize import minimize
import numpy as np

def gps_solve(distances_to_station, stations_coordinates):
	def error(x, c, r):
		return sum([(np.linalg.norm(x - c[i]) - r[i]) ** 2 for i in range(len(c))])

	l = len(stations_coordinates)
	S = sum(distances_to_station)
	# compute weight vector for initial guess
	W = [((l - 1) * S) / (S - w) for w in distances_to_station]
	# get initial guess of point location
	x0 = sum([W[i] * stations_coordinates[i] for i in range(l)])
	# optimize distance from signal origin to border of spheres
	return minimize(error, x0, args=(stations_coordinates, distances_to_station), method='Nelder-Mead').x



        
if __name__ == "__main__":

    with open('filename.txt', 'r') as f: 
	# read in all lines from the file 
	lines = f.readlines() 
	# initialize an empty matrix to store the values 
	matrix = [] 
	# loop over each line 
	for line in lines: 
		# split the line into a list of values 
		values = line.strip().split(',') 
		# append the list of values to the matrix 
		matrix.append(values) 

	stations = list(np.array([[0,1], [2,0], [1,-1], [1,1]]))
	distances_to_station = [1.4142, 1, 1, 1]
	print(gps_solve(distances_to_station, stations))