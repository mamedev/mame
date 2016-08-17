import os
import subprocess
import sys
import hashlib
import shutil

def runProcess(cmd):
	#print " ".join(cmd)
	process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(stdout, stderr) = process.communicate()
	if not isinstance(stdout, str): # python 3
		stdout = stdout.decode('latin-1')
	if not isinstance(stderr, str): # python 3
		stderr = stderr.decode('latin-1')
	#if stderr:
	#	print stderr
	return process.returncode, stdout, stderr
	
def compareInfo(info1, info2):
	lines1 = info1.splitlines()
	lines2 = info2.splitlines()
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
			print(lines1[i] + " - " + lines2[i])
	
	return mismatch == False

def sha1sum(path):
	if not os.path.exists(path):
		return ""
	f = open(path, 'rb')
	try:
		sha1 = hashlib.sha1()
		while True:
			data = f.read(8192)
			if data:
				sha1.update(data)
			else:
				break
	finally:
		f.close()
	return sha1.hexdigest()

def extractcdAndCompare(type):
	global failure
	extractFileDir = os.path.join(tempFilePath, type + "_output")
	if not os.path.exists(extractFileDir):
		os.makedirs(extractFileDir)
	extractFileBase = os.path.join(extractFileDir, "extract")
	extractFile = extractFileBase + "." + type
	extractFileBin = extractFileBase + ".bin"
	
	exitcode, stdout, stderr = runProcess([chdmanBin, "extractcd", "-f", "-i", outFile, "-o", extractFile])
	if not exitcode == 0:
		print(d + " - extractcd (" + type + ") failed with " + str(exitcode) + " (" + stderr + ")")
		failure = True
	
	sha1_extract = sha1sum(extractFile)
	sha1_extract_bin = sha1sum(extractFileBin)
	
	extractFileDir = os.path.join(tempFilePath, type + "_temp")
	if not os.path.exists(extractFileDir):
		os.makedirs(extractFileDir)
	extractFileBase = os.path.join(extractFileDir, "extract")
	extractFile = extractFileBase + "." + type
	extractFileBin = extractFileBase + ".bin"
	
	exitcode, stdout, stderr = runProcess([chdmanBin, "extractcd", "-f", "-i", tempFile, "-o", extractFile])
	if not exitcode == 0:
		print(d + " - extractcd (" + type + ") failed with " + str(exitcode) + " (" + stderr + ")")
		failure = True
		
	sha1_extract_2 = sha1sum(extractFile)
	sha1_extract_bin_2 = sha1sum(extractFileBin)
		
	if not sha1_extract == sha1_extract_2:
		print("expected: " + sha1_extract + " found: " + sha1_extract_2)
		print(d + " - SHA1 mismatch (extractcd - " + type + " - toc)")
		failure = True

	if not sha1_extract_bin == sha1_extract_bin_2:
		print("expected: " + sha1_extract_bin + " found: " + sha1_extract_bin_2)
		print(d + " - SHA1 mismatch (extractcd - " + type + " - bin)")
		failure = True
		
def extractAndCompare(command, ext):
	global failure
	extractFileDir = os.path.join(tempFilePath, ext + "_output")
	if not os.path.exists(extractFileDir):
		os.makedirs(extractFileDir)
	extractFileBase = os.path.join(extractFileDir, "extract")
	extractFile = extractFileBase + "." + ext
	
	exitcode, stdout, stderr = runProcess([chdmanBin, command, "-f", "-i", outFile, "-o", extractFile])
	if not exitcode == 0:
		print(d + " - " + command + " (" + ext + ") failed with " + str(exitcode) + " (" + stderr + ")")
		failure = True
	
	sha1_extract = sha1sum(extractFile)
	
	extractFileDir = os.path.join(tempFilePath, ext + "_temp")
	if not os.path.exists(extractFileDir):
		os.makedirs(extractFileDir)
	extractFileBase = os.path.join(extractFileDir, "extract")
	extractFile = extractFileBase + "." + ext
	
	exitcode, stdout, stderr = runProcess([chdmanBin, command, "-f", "-i", tempFile, "-o", extractFile])
	if not exitcode == 0:
		print(d + " - " + command + " (" + ext + ") failed with " + str(exitcode) + " (" + stderr + ")")
		failure = True
		
	sha1_extract_2 = sha1sum(extractFile)
		
	if not sha1_extract == sha1_extract_2:
		print("expected: " + sha1_extract + " found: " + sha1_extract_2)
		print(d + " - SHA1 mismatch (" + command + " - " + ext + ")")
		failure = True

currentDirectory = os.path.dirname(os.path.realpath(__file__))
inputPath = os.path.join(currentDirectory, 'input')
outputPath = os.path.join(currentDirectory, "output")
tempPath = os.path.join(currentDirectory, "temp")
if os.name == 'nt':
	chdmanBin = os.path.normpath(os.path.join(currentDirectory, "..", "..", "chdman.exe"))
else:		
	chdmanBin = os.path.normpath(os.path.join(currentDirectory, "..", "..", "chdman"))

if not os.path.exists(chdmanBin):
	sys.stderr.write(chdmanBin + " does not exist\n")
	sys.exit(1)

if not os.path.exists(inputPath):
	sys.stderr.write(inputPath + " does not exist\n")
	sys.exit(1)
	
if not os.path.exists(outputPath):
	sys.stderr.write(outputPath + " does not exist\n")
	sys.exit(1)

if os.path.exists(tempPath):	
	shutil.rmtree(tempPath)
	
failure = False

for root, dirs, files in os.walk(inputPath):
	for d in dirs:
		if d.startswith("."):
			continue

		command = ext = d.split("_", 2)[0]			
		inFile = os.path.join(root, d, "in")
		# TODO: make this better
		outFile = os.path.join(root, d, "out.chd").replace("input", "output")
		tempFilePath = os.path.join(tempPath, d)
		tempFile = os.path.join(tempFilePath, "out.chd")
		inParams = inFile + ".params"
		params = []
		if os.path.exists(inParams):
			f = open(inParams, 'r')
			paramsstr = f.read()
			f.close()
			params = paramsstr.split(" ")
		if not os.path.exists(tempFilePath):
			os.makedirs(tempFilePath)
		if command == "createcd":
			ext = d.split("_", 2)[1]
			inFile += "." + ext
		elif command == "createhd":
			ext = d.split("_", 2)[1]
			inFile += "." + ext
		elif command == "createld":
			ext = d.split("_", 2)[1]
			inFile += "." + ext
		elif command == "copy":
			inFile += ".chd"
		else:
			print("unsupported mode '%s'" % command)
			continue
		if os.path.exists(inFile):
			cmd = [chdmanBin, command, "-f", "-i", inFile, "-o", tempFile] + params
		else:
			cmd = [chdmanBin, command, "-f", "-o", tempFile] + params

		exitcode, stdout, stderr = runProcess(cmd)
		if not exitcode == 0:
			print(d + " - command failed with " + str(exitcode) + " (" + stderr + ")")
			failure = True
		
		# verify
		exitcode, stdout, stderr = runProcess([chdmanBin, "verify", "-i", tempFile])
		if not exitcode == 0:
			print(d + " - verify failed with " + str(exitcode) + " (" + stderr + ")")
			failure = True
			
		# compare info
		# TODO: store expected output of reference file as well and compare
		exitcode, info1, stderr = runProcess([chdmanBin, "info", "-v", "-i", tempFile])
		if not exitcode == 0:
			print(d + " - info (temp) failed with " + str(exitcode) + " (" + stderr + ")")
			failure = True
		exitcode, info2, stderr = runProcess([chdmanBin, "info", "-v", "-i", outFile])
		if not exitcode == 0:
			print(d + " - info (output) failed with " + str(exitcode) + " (" + stderr + ")")
			failure = True
		if not compareInfo(info1, info2):
			print(d + " - info output differs")
			failure = True
			
		# extract and compare
		if command == "createcd":
			extractcdAndCompare("toc")
			extractcdAndCompare("cue")
		elif command == "createhd":
			extractAndCompare("extracthd", "raw")
		elif command == "createld":
			extractAndCompare("extractld", "avi")
		# TODO: add extraction for "copy" as well
			
		# compare SHA1 of output files
		sha1_out = sha1sum(outFile)
		sha1_temp = sha1sum(tempFile)
		if not sha1_out == sha1_temp:
			print("expected: " + sha1_out + " found: " + sha1_temp)
			print(d + " - SHA1 mismatch (output file)")
			failure = True

if not failure:
	print("All tests finished successfully")
