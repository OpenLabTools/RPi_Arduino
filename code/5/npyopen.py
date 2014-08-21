import numpy as np

datafile = open('data.npy', 'rb')

a = np.zeros(0)

while 1:
	try:
		a=np.append(a, np.load(datafile))
	except IOError:
		break
		
datafile.close()
		




		
