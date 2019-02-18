# Project   : Scratch3Convert
# File      : project.py
# Author    : Alex Cui

import zipfile
import os
import shutil

def unpack(path):
	if not os.path.exists(path + "t/"):
		os.mkdir(path + "t/")
	if os.path.exists(path + "t/e/"):
		shutil.rmtree(path + "t/e/")
	os.mkdir(path + "t/e/")
	file = zipfile.ZipFile(path, "r")
	for item in file.namelist():
		file.extract(item, path + "t/")

def pack(path):
	name = path[:-3] + "sb2"
	file = zipfile.ZipFile(name, "w")
	for parent, _, filename in os.walk(path + "t/e/"):
		for name in filename:
			file.write(os.path.join(parent, name))
	#shutil.rmtree(path + "t/")
	file.close()

