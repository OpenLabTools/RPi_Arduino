import serial
import sys

portname_start = '/dev/ttyUSB'

# Try opening serial ports /dev/ttyUSB0->9
for i in range (0, 10):
	# Change i to a string
	portnum = str(i)	
	# Append "i" to the end of portname_start										
	portname_full=''.join([portname_start,portnum])
	# Try to open the serial port with i, if this fails try again with i+1
	try:
		ser = serial.Serial(portname_full, 250000, timeout =1)
		break
	except:
		# If we reach i==9 and no port has been opened, exit and print error message
		if (i==9):
			print "No Serial Port Found"
			sys.exit(0)

# Clear input buffer
ser.flushInput()

while True:
	# Get string to send to Arduino from user
	stringToSend = raw_input ("Enter string to send to Arduino: ")
	# If user inputs "EXIT" break from the while loop
	if (stringToSend == "EXIT"):
		break
	# Write the string to the Arduino
	ser.write(stringToSend)
	# Wait until we recieve back the same number of bytes as we sent
	while(ser.inWaiting() < len(stringToSend)):
		pass
	# Read the string in the serial input buffer
	stringRecieved = ser.read(len(stringToSend))
	
	print "String recieved from Arduino: ", stringRecieved
