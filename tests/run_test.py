#!/usr/bin/env python

import os;
import subprocess;

def execCommand(cmd):
  print("Exec command: '{0}'".format(cmd));
  process = subprocess.Popen(cmd, shell = True);
  error = process.wait();
  if error:
    raise Exception("Executing failed.");

def execPyCommand(cmd):
  execCommand("python {0}".format(cmd));

fileName = "data.txt"
sortedFileName = "data_sorted.txt";
linesCount = 1 * 1024 * 1024;
maxLineLength = 128;

if os.path.exists(sortedFileName) and os.path.isfile(sortedFileName):
  os.remove(sortedFileName)

mergeCommand = "./ExternalSort input={0} output={1} temp_dir=./temp/ max_memory_usage_Mb=8 max_write_buffer_Kb=128 remove_temp_files=yes".format(fileName, sortedFileName);

if not os.path.exists(fileName):
  execPyCommand("generate_data_file.py {0} {1} {2}".format(fileName, linesCount, maxLineLength));
execCommand(mergeCommand);
execPyCommand("check_file_sorted.py {0}".format(sortedFileName));
