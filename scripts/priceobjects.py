#!/usr/bin/python

import os
import fnmatch
import sys
import fileinput
import re
import math

count = 0

for root, dir, files in os.walk("../assets/packed"):
        for items in fnmatch.filter(files, "*.object"):
        	filePath = os.path.join(root, items)
        	newFile = open(filePath + '.new', 'w+')
        	for line in fileinput.input(filePath):
        		priceMatch = re.search("\"price\"\s?: (\d+)", line)
        		if priceMatch:
        			count += 1
        			price = int(priceMatch.groups()[0])
        			newPrice = int(math.floor(price * float(sys.argv[1])))
        			print str(price) + " -> " + str(newPrice) + " | in: " + filePath
        			line = line.replace(str(price), str(newPrice))
    			newFile.write(line)
    		newFile.close()
    		os.remove(filePath)
    		os.rename(filePath + '.new', filePath)
        		

print '\n\n' + str(count) + " files changed"