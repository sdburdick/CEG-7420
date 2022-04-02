#TODO write a description for this script
#@author Scott Burdick, Michael Celesti
#@category Buffer Overflow Detection
#@keybinding 
#@menupath 
#@toolbar 

#need to find three things,
#when the network card is receiving data
#when data is copied into memory
#determine if the cop is restricted to proper size

funcList = []
func = getFirstFunction()
while func is not None:
	if (func.getName() == "recv"):
		#we've found net traffic
		print("recv")
		#figure out what variable is used as the buffer
		#then figure out that buffer size
	if (func.getName() == "memcpy"):
		#we've found
		print("memcpy")
		#see if memcpy is restricted to the buffer size
	func = getFunctionAfter(func)

from collections import Counter
fcounter = (Counter(funcList))
fx = sorted((fcounter.items()))
print fx
