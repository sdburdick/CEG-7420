#TODO write a description for this script
#@author Scott Burdick, Michael Celesti
#@category Buffer Overflow Detection
#@keybinding 
#@menupath 
#@toolbar 
#references:
#https://github.com/HackOvert/GhidraSnippets#get-a-function-name-by-address
#https://github.com/jasonkimprojects/ghidra-scripts/blob/master/find_dangerous_functions.py

#functions called out in "Automatic Detection of Network Interface Buffer Overflow Errors in C++"
allocators = [ "memcpy", "strcpy", "gets", "scanf", "strcat" ]



#functions called out in "Automatic Detection of Network Interface Buffer Overflow Errors in C++"
network = ["accept", "recv", "bind", "opensocket"]

program = ghidra.program.flatapi.FlatProgramAPI(currentProgram)
listing = currentProgram.getListing()
func = program.getFirstFunction()

errorMessages = []

while func is not None:
    if str(func.getName()) in allocators:
        print "Dangerous function found: {}".format(func.getName())
        print "at address: {}".format(func.getEntryPoint())
        dummyTM = ghidra.util.task.TaskMonitor.DUMMY
        called_by = func.getCallingFunctions(dummyTM)
        for caller in called_by:
		#we have an allocator, see if there is a network call colocated in the parent function
	        print "Warning: {} is called by: {} at {}\n".format(
        		func.getName(), caller.getName(), caller.getEntryPoint())
	        dummy2 = ghidra.util.task.TaskMonitor.DUMMY
        	parent = caller.getCalledFunctions(dummy2)
        	for functionsWithin in parent:
	            print "Info: {} is called: {} at {}\n".format(
        	            functionsWithin.getName(), caller.getName(), functionsWithin.getEntryPoint())
		    if str(functionsWithin.getName()) in network:
			
			print ("************")
			print ("ISSUE FOUND!")
			print "Do Not Use This Application: ", currentProgram.getName()
			print ("ISSUE FOUND!")
			print ("************")
			errorMessages.append("************")
			errorMessages.append("Do Not Use This Application: ")
			errorMessages.append(currentProgram.getName())
			errorMessages.append("Bad functions co-located:")
			errorMessages.append(functionsWithin.getName())
			errorMessages.append("*with*")
			errorMessages.append(func.getName())
			errorMessages.append("************")
    func = program.getFunctionAfter(func)



#in the end, repeat the error messages

for msg in errorMessages:
	print(msg)
