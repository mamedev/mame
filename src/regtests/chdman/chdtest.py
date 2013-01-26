import os
import subprocess
import sys

def runProcess(cmd):
	process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(stdout, stderr) = process.communicate()
	return process.returncode, stdout, stderr
	
def compareInfo(info1, info2):
	lines1 = info1.splitlines();
	lines2 = info2.splitlines();
	if not len(lines1) == len(lines2):
		return False
	
	mismatch = False
	for i in range(len(lines1)):
		if lines1[i].startswith("chdman - ") and lines2[i].startswith("chdman - "):
			continue
		if lines1[i].startswith("Input file:") and lines2[i].startswith("Input file:"):
			continue
		if not lines1[i] == lines2[i]:
			mismatch = True
			print lines1[i] + " - " + lines2[i]
	
	return mismatch == False

currentDirectory = os.path.dirname(os.path.realpath(__file__))
inputPath = os.path.join(currentDirectory, 'input')
outputPath = os.path.join(currentDirectory, "output")
tempPath = os.path.join(currentDirectory, "temp")
if os.name == 'nt':
	chdmanBin = os.path.normpath(os.path.join(currentDirectory, "..", "..", "..", "chdman.exe"))
else:		
	chdmanBin = os.path.normpath(os.path.join(currentDirectory, "..", "..", "..", "chdman"))

if not os.path.exists(chdmanBin):
	print chdmanBin + " does not exist"
	sys.exit(1)

if not os.path.exists(inputPath):
	print inputPath + " does not exist"
	sys.exit(1)
	
if not os.path.exists(inputPath):
	print inputPath + " does not exist"
	sys.exit(1)
	
failure = False

for root, dirs, files in os.walk(inputPath):
	for d in dirs:
		if d.startswith("."):
			continue
	
		inFile = os.path.join(root, d, "in")
		# TODO: make this better
		outFile = os.path.join(root, d, "out.chd").replace("input", "output")
		tempFilePath = os.path.join(tempPath, d)
		tempFile = os.path.join(tempFilePath, "out.chd")
		cmd = []
		if not os.path.exists(tempFilePath):
			os.makedirs(tempFilePath)
		if d.startswith("createcd"):
			ext = d.split("_", 2)[1]
			inFile += "." + ext
			cmd = [chdmanBin, "createcd", "-f", "-i", inFile, "-o", tempFile]
		elif d.startswith("createhd"):
			inFile += ".params"
			f = open(inFile, 'r')
			paramsstr = f.read()
			f.close()
			params = paramsstr.split(" ")
			cmd = [chdmanBin, "createhd", "-f", "-o", tempFile] + params
		elif d.startswith("copy"):
			inFile += ".chd"
			cmd = [chdmanBin, "copy", "-f", "-i", inFile, "-o", tempFile]
		else:
			print "unsupported mode"
			continue
		exitcode, stdout, stderr = runProcess(cmd)
		if not exitcode == 0:
			print d + " - command failed with " + str(exitcode) + " (" + stderr + ")"
			failure = True
		exitcode, stdout, stderr = runProcess([chdmanBin, "verify", "-i", tempFile])
		if not exitcode == 0:
			print d + " - verify failed with " + str(exitcode) + " (" + stderr + ")"
			failure = True
		# TODO: store exected output of reference file as well and compare
		exitcode, info1, stderr = runProcess([chdmanBin, "info", "-v", "-i", tempFile])
		if not exitcode == 0:
			print d + " - info (temp) failed with " + str(exitcode) + " (" + stderr + ")"
			failure = True
		exitcode, info2, stderr = runProcess([chdmanBin, "info", "-v", "-i", outFile])
		if not exitcode == 0:
			print d + " - info (output) failed with " + str(exitcode) + " (" + stderr + ")"
			failure = True
		if not compareInfo(info1, info2):
			print d + " - info output differs"
			failure = True
		# TODO: extract and compare

if not failure:
	print "All tests finished successfully"