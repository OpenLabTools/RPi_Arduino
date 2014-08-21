import serial
import time
import sys
import numpy as np

datafile = open('data.npy', 'w')		# Open 'data.npy' in write mode to clear any existing contents
datafile.close()

portname_start = '/dev/ttyUSB'

for i in range (0, 10):					# Try opening serial ports /dev/ttyUSB0-9
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
				
ser.flushInput()						# Clear serial input buffer	

# Continuously send "a\E" to the Arduino to ask if it is ready until we recieve an "r" back from the Arduino
# signalling that the device is ready
while (ser.readline(1) != "r") :		
	ser.write("a\E")
	time.sleep(1)

time.sleep(1)							
ser.flushInput()
print "Arduino Ready"

while True:								# Get sampling mode from the user
	mode = input ("Enter sampling mode [0 - 100sps] [1-1ksps] [2-10ksps] [3-25ksps]:  ")
	if (0 <= mode <= 4):
		break
	print "Invalid Input for mode"

if (mode == 0):							# Set the sampling frequency based on which mode is chosen
	sampfreq = 100
	
elif (mode == 1):
	sampfreq = 1000
	
elif (mode == 2):
	sampfreq = 10000
	
elif (mode == 3):
	sampfreq = 25000
	
bufFreq = sampfreq / 500.0

while True:									# Get sampling time from the user	
	sampletime = input("Enter the sampling time in seconds:    ")
	p = int((sampfreq*sampletime)/500)
	if (p == 0):							# If the sampling time is too low then we we round to an int in the line above
		p = 1								# we may set p to 0, if this is the case set p=1
	if (0 <= p <= 65535):					# Check if p is within allowed limits
		break
	print "Invalid sampletime, p must be below 65536"

b = np.zeros(6)							# Create numpy array of 6 bytes

b[0] = mode		# First byte corresponds to the mode

# Second two bytes correspond to the value of p required for the desired sampling time
b[1] = ((p & 0xFF00) >> 8)   # MSByte
b[2] = (p & 0xFF)			 # LSByte

# The end of the buffer is "b\E"
b[3] = 98		# "b"
b[4] = 92		# "\"
b[5] = 69		# "E"

b=b.astype(int)								# Convert b to integers, join the 3 bytes together to form a string, the write this string to serial
sendstring=''.join([chr(i) for i in b])
ser.write(sendstring)

time.sleep(1)								# Give the arduino time to configure registers

datafile = open('data.npy', 'ab')			# Open 'data.npy' in binary append mode

print "Press ENTER to start sampling"		# Get user input to start sampling
raw_input()

ser.flushInput()							# Make sure input buffer is empty

q=0			# Set q to 0, q is the number of 500 byte buffers we will read from serial
serBuf=0	# Set a to 0, a is how many bytes are waiting in the serial input buffer

ser.write("c\E")	# Write "c\E" to start sampling

# Read sampled data from serial port and save to file
while (q<p):
	while(serBuf < 500):			# Wait until a full buffer (500 bytes) is available
		serBuf = ser.inWaiting()
		if (serBuf >= 4000):		# Check if a is >= to 4000, if so exit as buffer is full (buffer size is 4096 bytes)
			print "Sampling Frequency too high, Serail input buffer full, exiting...!!"
			sys.exit(1)
	data0 = ser.read(500)						# Read a buffer
	a=np.fromstring(data0,dtype='uint8')		# Convert ascii characters to numpy array of bytes
	np.save(datafile, a)						# Save the data to a numpy file
	serBuf=0
	q +=1
	# Could add extra code here to output some sort of notification to the terminal of how
	# long is left to sample

# Check there is no data left in the serial input buffer
time.sleep(1)
if (ser.inWaiting()!=0):
	print "serial input buffer contains non-zero number of bytes, errors maybe have occured during sampling"

ser.close()
datafile.close()

