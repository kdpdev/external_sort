#!/usr/bin/env python

import math;
import random;
import string;
import sys;

fileName = sys.argv[1];
linesCount = int(sys.argv[2]);
maxLineLength = int(sys.argv[3]);

print("Generating data file ...");
print("  fileName      : {0}".format(fileName));
print("  linesCount    : {0}".format(linesCount));
print("  maxLineLength : {0}".format(maxLineLength));

file = open(fileName, "w");

choice = string.ascii_lowercase + string.ascii_uppercase + string.digits;
iter = 0;
percents = -1;
while (iter < linesCount):
  lineLength = random.randint(1, maxLineLength);
  line = ''.join(random.choice(choice) for _ in range(lineLength));
  file.write(line);
  file.write("\n");
  iter += 1;
  p = math.trunc(iter * 100.0 / linesCount);
  if percents < 0 or p / 10 != percents / 10:
    percents = p;
    print("  generate progress: {0} %".format(percents));

file.close();

print("DONE");
