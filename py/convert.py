# Project   : Scratch3Convert
# File      : convert.py
# Author    : Alex Cui

import json
import project
import os
import copy
import shutil

# Plan:
#	objname		y
#	variables	y
#	lists		y
#	scripts		y
#	comments	n
#	costumes	y
#	sounds		y

def convert(path):
	project.unpack(path)
	# convert
	file = open("./data.json")
	opcode = json.load(file)
	file.close()
	file = open(path + "t/project.json")
	src = json.load(file)
	file.close()
	# don't change! very important!
	result = {
		"objName": "Stage",
		"variables": [],
		"lists": [],
		"children": [],
		"scripts": [],
		"scriptComments": [],
		"costumes": [],
		"sounds": [],
		"info": {
			"flashVersion": "WIN 32,0,0,100",
			"swfVersion": "",
			"videoOn": False,
		#	"scriptCount": 0,
			"userAgent": "Scratch 2.0 Offline Editor",
		#	"spriteCount": 0,
			"metaData": "ad82caf04f3a3976353656cf87ace2ae"
		}
	}
	identifer = {}
	md5costume = []
	md5sound = []
	costumeid = 1
	soundid = 0
	for target in src["targets"]:
		item = None
		# objname
		if target["isStage"]:
			item = result
		else:
			result["children"] += [{
				"objName": target["name"],
				"variables": [],
				"lists": [],
				"scripts": [],
				"scriptComments": [],
				"costumes": [],
				"sounds": [],
				"spriteInfo": {
					"metaData": "ad82caf04f3a3976353656cf87ace2ae"
				}
			}]
			item = result["children"][-1]
		# variables
		for key, value in target["variables"].items():
			identifer[key] = value[0]
			item["variables"] += [{
				"name": value[0],
				"value": value[1],
				"isPersistent": False
			}]
		# lists
		for key, value in target["lists"].items():
			identifer[key] = value[0]
			item["lists"] += [{
				"listName": value[0],
				"contents": value[1],
				"isPersistent": False,
				"x": 0,
				"y": 0,
				"width": 0,
				"height": 0,
				"visible": False
			}]
		# scripts
		for key, value in target["blocks"].items():
			if value["topLevel"] == True:
				block_item = value
				# for header, position
				item["scripts"] += [[block_item["x"], block_item["y"], convert_script(opcode, target["blocks"], key)]]
		# costumes
		for costume in target["costumes"]:
			if costume["assetId"] in md5costume :
				item["costumes"] += [{
					"costumeName": costume["name"],
					"baseLayerID": md5costume.index(costume["assetId"]),
					"baseLayerMD5": costume["md5ext"],
					"bitmapResolution": costume["bitmapResolution"] if "bitmapResolution" in costume else 1,
					"rotationCenterX": costume["rotationCenterX"],
					"rotationCenterY": costume["rotationCenterY"]
				}]
			else:
				item["costumes"] += [{
					"costumeName": costume["name"],
					"baseLayerID": costumeid,
					"baseLayerMD5": costume["md5ext"],
					"bitmapResolution": costume["bitmapResolution"] if "bitmapResolution" in costume else 1,
					"rotationCenterX": costume["rotationCenterX"],
					"rotationCenterY": costume["rotationCenterY"]
				}]
				md5costume += [costume["assetId"]]
				shutil.copyfile(path + "t/" + costume["md5ext"], path + "t/e/" + str(costumeid) + "." + costume["dataFormat"])
				costumeid += 1
		# current costume
		item["currentCostumeIndex"] = target["currentCostume"]
		# sounds
		for sound in target["sounds"]:
			if sound["assetId"] in md5sound:
				item["sounds"] += [{
					"soundName": sound["name"],
					"soundID": md5sound.index(sound["assetId"]),
					"md5": sound["md5ext"],
					"sampleCount": sound["sampleCount"],
					"rate": sound["rate"],
					"format": sound["format"]
				}]
			else:
				item["sounds"] += [{
					"soundName": sound["name"],
					"soundID": soundid,
					"md5": sound["md5ext"],
					"sampleCount": sound["sampleCount"],
					"rate": sound["rate"],
					"format": sound["format"]
				}]
				md5sound += [sound["assetId"]]
				shutil.copyfile(path + "t/" + sound["md5ext"], path + "t/e/" + str(soundid) + "." + sound["dataFormat"])
				soundid += 1
		# stage only
		if target["isStage"]:
			# video alpha
			item["videoAlpha"] = target["videoTransparency"] / 100
			# bpm
			item["tempoBPM"] = target["tempo"]
			# pen layer
			#item["penLayerMD5"] = "5c81a336fab8be57adc039a8a2b33ca9.png"
			#item["penLayerID"] = 0
			#shutil.copyfile("./penlayer.png", path + "t/e/0.png")
		# sprite only
		else:
			# position
			item["scratchX"] = target["x"]
			item["scratchY"] = target["y"]
			# size/scale
			item["scale"] = target["size"] / 100
			# direction
			item["direction"] = target["direction"]
			# draggable
			item["isDraggable"] = target["draggable"]
			# todo: rotation style
			# visible
			item["visible"] = target["visible"]
	file = open(path + "t/e/project.json", "w")
	json.dump(result, file)
	file.close()
	project.pack(path)
	return 0

def convert_script(opcode, blocks, name):
	result = []
	block_item = blocks[name]
	while True:
		# get name in scratch2
		block = [opcode[block_item["opcode"]][0]]
		# for each param of block
		for k in opcode[block_item["opcode"]][1]:
			# value in inputs
			if k in block_item["inputs"]:
				# [1, *] a immediate value (including field)
				if block_item["inputs"][k][0] == 1:
					# [1, [*, *]] a immediate value (string & int)
					if type(block_item["inputs"][k][1]) == list:
					#	# [4, *] type as integer
					#	if block_item["inputs"][k][1][0] == 4:
					#		block += int(block_item["inputs"][k][1][1])
					#	# [10, *] type as string
					#	elif block_item["inputs"][k][1][0] == 10:
					#		block += str(block_item["inputs"][k][1][1])
						block += [str(block_item["inputs"][k][1][1])]
					# [1, *] a emun value (field)
					elif type(block_item["inputs"][k][1]) == str:
						block += [blocks[block_item["inputs"][k][1]]["fields"][k][0]]
				# [3, *] a block-insert value
				elif block_item["inputs"][k][0] == 3:
					block += convert_script(opcode, blocks, block_item["inputs"][k][1])
			# value in fields
			elif k in block_item["fields"]:
				block += [block_item["fields"][k][0]]
		result += [block]
		if block_item["next"] == None:
			break
		else:
			block_item = blocks[block_item["next"]]
	return result

if __name__ == "__main__":
    path = input("sb3 path:")
    convert(path)
