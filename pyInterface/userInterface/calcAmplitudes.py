#!/usr/bin/env python

import argparse
import sys

import pyRootPwa
import pyRootPwa.core

def readWaveList(waveListFileName):
	pyRootPwa.utils.printInfo("reading amplitude names and thresholds from wave list file "
	          + "'" + waveListFileName + "'.")
	try:
		with open(waveListFileName, 'r') as waveListFile:
			waveNamesFromWavelist = []
			waveThresholds = []
			lineNmb = 0
			for line in waveListFile:
				if (line[0] == '#'):  # comments start with #
					continue
				line = line.replace('\n', '')
				lineArray = line.split(" ")
				if(len(lineArray) >= 1 and len(lineArray) <= 2):
					waveName = lineArray[0]
					if(len(lineArray) == 1):
						threshold = 0
					else:
						threshold = lineArray[1]
					waveNamesFromWavelist.append(waveName)
					waveThresholds.append(float(threshold))
				else:
					pyRootPwa.utils.printWarn("cannot parse line '" + line + "' in wave list file "
					          + "'" + waveListFileName + "'.")
	#  			if (_debug):
	#  				printDebug("reading line " + lineNmb + 1 + ": " + waveName + ", "
	#  				           + "threshold = " + threshold + " MeV/c^2")
				lineNmb += 1
	except IOError:
		printError("cannot open file '" + waveListFileName + "'. aborting.")
		sys.exit(1)
	pyRootPwa.utils.printInfo("read " + str(lineNmb) + " lines from wave list file " + "'" + waveListFileName + "'")
	
	return (waveNamesFromWavelist, waveThresholds)

if __name__ == "__main__":

	pyRootPwa.utils.stdoutisatty = sys.stdout.isatty()
	pyRootPwa.utils.stderrisatty = sys.stderr.isatty()

	# parse command line arguments
	parser = argparse.ArgumentParser(
	                                 description="calculates decay amplitudes "
	                                             "for given wave for events in "
	                                             "input data files and "
	                                             "writes amplitudes to file"
	                                )

	parser.add_argument("-c", type=str, metavar="configFileName", default="rootpwa.config", dest="configFileName", help="path to config file (default: ./rootpwa.config)")
	parser.add_argument("-n", type=int, metavar="#", default=-1, dest="maxNmbEvents",  help="maximum number of events to read (default: all)")
	parser.add_argument("-b", type=str, metavar="massBin(s)", default="all", dest="massBins", help="mass bins to be calculated (default: all)")
	parser.add_argument("-j", type=int, metavar=("jobs"), default=1, dest="nJobs", help="number of jobs (default: 1)")
	parser.add_argument("-f", "--no-progress-bar", action="store_true", dest="noProgressBar", help="disable progress bars (decreases computing time)")
	parser.add_argument("-k", "--keyfiles", type=str, metavar="keyfiles", dest="keyfiles", nargs="*", help="keyfiles to calculate amplitude for (overrides settings from the config file)")
	parser.add_argument("-w", type=str, metavar="wavelistFileName", default="", dest="wavelistFileName", help="path to wavelist file (default: none)")
#	parser.add_argument("-v", action="store_true", dest="debug", help="verbose; print debug output (default: false)")
	parser.add_argument("--profiler", type=str, metavar="profiler-output", dest="proFile", help="path to profiler output file")
	args = parser.parse_args()

	config = pyRootPwa.rootPwaConfig()
	if not config.initialize(args.configFileName):
		pyRootPwa.utils.printErr("loading config file '" + args.configFileName + "' failed. Aborting...")
		sys.exit(1)
	pyRootPwa.core.particleDataTable.readFile(config.pdgFileName)
	fileManager = pyRootPwa.loadFileManager(config.fileManagerPath)
	if not fileManager:
		pyRootPwa.utils.printErr("loading the file manager failed. Aborting...")
		sys.exit(1)
	if (not args.wavelistFileName==""):
		(waveNamesFromWavelist, waveThresholds) = readWaveList(args.wavelistFileName)
	waveList = waveNamesFromWavelist
	if (len(waveNamesFromWavelist) == 0):
		waveList = fileManager.getWaveNameList()

	for binID in fileManager.getBinIDList():
		for waveName in waveList:
			for eventsType in [ pyRootPwa.core.eventMetadata.REAL, pyRootPwa.core.eventMetadata.GENERATED, pyRootPwa.core.eventMetadata.ACCEPTED, pyRootPwa.core.eventMetadata.OTHER ]:
				dataFile = fileManager.getDataFile(binID, eventsType)
				if not dataFile:
					continue
				if not pyRootPwa.calcAmplitude(dataFile.dataFileName, fileManager.getKeyFile(waveName), fileManager.getAmplitudeFilePath(binID, waveName, eventsType), args.maxNmbEvents, not args.noProgressBar):
					pyRootPwa.utils.printWarn("could not calculate amplitude.")
