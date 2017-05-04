#!/usr/bin/env python

import os;
import sys;

fileName = sys.argv[1];

print("Check sorted ...");
print("  fileName : {0}".format(fileName));

if not os.path.exists(fileName):
  raise Exception("Fs entry does not exist (entry = '{0}').".format(fileName));

if not os.path.isfile(fileName):
  raise Exception("Fs entry is not a file (entry = '{0}').".format(fileName));
  
file = open(fileName, "r");
prevLine = file.readline();
lineNumber = 1;
for line in file:
  lineNumber += 1;
  if prevLine > line:
    raise Exception("Not sorted:\n{0}:{1}{2}:{3}".format(lineNumber-1, prevLine, lineNumber, line));
  prevLine = line;

file.close();

print("The '{0}' file is sorted.".format(fileName));
print("DONE");
