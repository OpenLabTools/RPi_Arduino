import serial
import string
import time
import numpy as np

datafile = open('data.npy', 'w')			# Open 'data.npy' in write mode to clear any existing contents
datafile.close()

portname_start = '/dev/ttyUSB'

for i in range (0, 10):						# Try opening serial ports /dev/ttyUSB0-9
	portnum = str(i)
	portname_full=''.join([portname_start,portnum])
	try:
		ser = serial.Serial(portname_full, 1000000, timeout =1)
		print "Serial Port: ", portname_full
		break
	except:
		if (i==9):
			print "No Serial Port Found"
			sys.exit(0)
			
ser.flushInput();							# Clear input buffer

# Continuously send "a\E" to the Arduino to ask if it is ready until we recieve an "r" back from the Arduino
# signalling that the device is ready
while (ser.readline(1) != "r") :	
	ser.write("a\E")
	time.sleep(1)

time.sleep(1)							
ser.flushInput()
print "Arduino Ready"

datafile = open('data.npy', 'ab')			# Open data file in binary append mode

print "Press ENTER to start sampling"		# Ask for user input to start sampling
raw_input()

ser.write("c\E")							# Write "c\E" to start start sampling
i = 0

# Loop during which we read and save the burst sampled data, before each 1000 byte buffer a flag
# is transmitted, if this flag is an "e" then this signals for the program to exit so we break from
# the sampling loop
while True:
	while(ser.inWaiting() < 3):								# Wait until there are three bytes in the buffer (flag + "\n")
		pass
	if (string.rstrip(ser.readline(),'\r\n') == "e"):		# If we read the flag "e" break from the while loop to stop sampling
		break
	while(ser.inWaiting() < 1000):							# Wait for the buffer to be recieved	
		pass
	data0 = ser.read(1000)									# Read buffer, convert from ascii to numpy array and save to file	
	a=np.fromstring(data0,dtype='uint8')
	np.save(datafile, a)
	
	print "Data Sample :", i								# Print the number of the buffer just saved
	i += 1
	
datafile.close()
ser.close()

