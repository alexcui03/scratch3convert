# 
# scratch3convert
#
# Copyright (c) 2019 Alex Cui. All rights reserved.
# 
# This is a library for you to convert scratch3 project
# (*.sb3 file) to scratch2 project(*.sb2 file). You can
# call function "sc3convert_convert", the first argument
# is your project path, like "aaa/bbb/cc.sb3" or "aa.sb3".
# The output file is your file's name plus ".sb2", if
# your file name is "aaa.sb3", then the output file is
# "aaa.sb3.sb2". During converting the project, the program
# need a temp directory for save some data or extract the
# data from project, the temp directory name must be your
# project name plus "t", and your project name plus "t/e".
#

import ctypes

__sc3convert_dll = ctypes.CDLL("scratch3convert.dll")

def sc3convert(filename):
	return __sc3convert_dll.sc3convert_convert(ctypes.c_char_p(bytes(filename, encoding="utf-8")))

