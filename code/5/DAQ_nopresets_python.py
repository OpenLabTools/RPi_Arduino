import serial
import time
import numpy as np
import sys

# Open 'data.npy' in write mode to clear any existing contents
datafile = open('data.npy', 'w')
datafile.close()

portname_start = '/dev/ttyUSB'

# Try opening serial ports /dev/ttyUSB0-9
for i in range (0, 10):
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
			
# Clear serial input buffer		
ser.flushInput()

# Continuously send "a\E" to the Arduino to ask if it is ready until we recieve an "r" back from the Arduino
# signalling that the device is ready
while (ser.readline(1) != "r") :
	ser.write("a\E")
	time.sleep(1)

time.sleep(1)
ser.flushInput()

print "Arduino Ready"

# Open 'data.npy' in append mode
datafile = open('data.npy', 'ab')

# Get user input to start sampling
print "Press ENTER to start sampling"
raw_input()

# Set q to 0, q is the number of buffers we will read in
q=0
# Set serBuf to 0, serBuf is how many bytes are waiting in the serial input buffer
serBuf=0

# Write "c" to serial to start sampling
ser.write("c\E")

# Read sampled data from serial port and save to file
while (q<250):
	# Wait until a full buffer (500 bytes) is available
	while(serBuf < 500):
		serBuf = ser.inWaiting()
		# Check if serBuf is >= to 4000, if so exit as buffer is full (buffer size is 4096 bytes)
		if (serBuf >= 4000):
			print "Sampling Frequency too high, Serail input buffer full, exiting...!!"
			sys.exit(1)
	
	# Read a buffer
	data0 = ser.read(500)
	# Convert ascii characters to numpy array of bytes
	a=np.fromstring(data0,dtype='uint8')
	# Save the data to a numpy file
	np.save(datafile, a)
	a=0
	q +=1

# Check there is no data left in the input buffer
time.sleep(1)
if (ser.inWaiting()!=0):
	print "serial input buffer contains non-zero number of bytes, errors maybe have occured during sampling"
	
ser.close()
datafile.close()

