#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "./lib/win32/json-c.lib")
#pragma comment(lib, "./lib/win32/minizip.lib")
#pragma comment(lib, "./lib/win32/zlib.lib")
#endif

#include "scratch3convert.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <direct.h>
#include <direct.h>

#include "minizip/zip.h"
#include "minizip/unzip.h"
#include "jsonc/json.h"

#include "single_list.h"
#include "cppfunc.h"

#define BUFFER_SIZE 8192
#define MAX_FILENAME 512
#define DIR_DELIMTER '/'

typedef enum { true = 1, false = 0 } bool;

int sc3convert_unpack_file(const char *in_file_path);
int sc3convert_pack_file(const char *dir_path, const char *out_file);
void sc3convert_pack_file_from_dir(void *obj, const char *path, const char *file_name);
int sc3convert_convert_project(const char *name);
json_object *sc3convert_load_opcode();
json_object *sc3convert_new_object(const char *name);
json_object *sc3convert_new_stage();
json_object *sc3convert_new_sprite(const char *name);
json_object *sc3convert_new_variable(json_object *object);
json_object *sc3convert_new_list(json_object *object);
json_object *sc3convert_new_script(json_object *opcode, json_object *blocks, json_object *block_item);

int sc3convert_convert(const char *name) {
	return sc3convert_convert_project(name);
}

int sc3convert_unpack_file(const char *in_file_path) {
	// Open zip file
	unzFile *zip_file = unzOpen(in_file_path);
	if (zip_file == NULL) {
		return SC3CONVERT_EXCEPTION_UNZIPOPENZIP;
	}

	// Get global info
	unz_global_info global_info;
	if (unzGetGlobalInfo(zip_file, &global_info) != UNZ_OK) {
		unzClose(zip_file);
		return SC3CONVERT_EXCEPTION_UNZIPGLOBALINFO;
	}

	// Create buffer
	char buffer[BUFFER_SIZE];

	// Make dir
	char dir_path[MAX_FILENAME];
	strcpy(dir_path, in_file_path);
	strcat(dir_path, "t/");
	_mkdir(dir_path);

	// Extract all files
	for (size_t i = 0; i < global_info.number_entry; ++i) {
		// Get info of current file
		unz_file_info file_info;
		char file_name[MAX_FILENAME];
		char tmp_file_name[MAX_FILENAME];
		if (unzGetCurrentFileInfo(zip_file, &file_info, tmp_file_name, MAX_FILENAME, NULL, 0, NULL, 0) != UNZ_OK) {
			unzClose(zip_file);
			return SC3CONVERT_EXCEPTION_UNZIPFILEINFO;
		}

		// Add file name
		strcpy(file_name, dir_path);
		strcat(file_name, "/");
		strcat(file_name, tmp_file_name);

		// Check if is directory or file
		const size_t path_length = strlen(file_name);
		if (file_name[path_length - 1] == DIR_DELIMTER) {
			_mkdir(file_name);
		}
		else {
			// Open current file
			if (unzOpenCurrentFile(zip_file) != UNZ_OK) {
				unzClose(zip_file);
				return SC3CONVERT_EXCEPTION_UNZIPOPENFILE;
			}

			// Open a file to write data
			FILE *file_out = fopen(file_name, "wb");
			if (file_out == NULL) {
				unzCloseCurrentFile(zip_file);
				unzClose(zip_file);
				return SC3CONVERT_EXCEPTION_UNZIPCREATEFILE;
			}

			int code = UNZ_OK;
			do {
				code = unzReadCurrentFile(zip_file, buffer, BUFFER_SIZE);
				if (code < 0) {
					unzCloseCurrentFile(zip_file);
					unzClose(zip_file);
					return SC3CONVERT_EXCEPTION_UNZIPREADFILE;
				}

				if (code > 0) {
					fwrite(buffer, code, 1, file_out);
				}
			} while (code > 0);

			fclose(file_out);
		}

		// Close current file
		unzCloseCurrentFile(zip_file);

		if (i + 1 < global_info.number_entry) {
			if (unzGoToNextFile(zip_file) != UNZ_OK) {
				unzClose(zip_file);
				return SC3CONVERT_EXCEPTION_UNZIPREADNEXTFILE;
			}
		}
	}

	// Close zip file
	unzClose(zip_file);
	
	return SC3CONVERT_SUCCESS;
}

int sc3convert_pack_file(const char *dir_path, const char *out_file) {
	// Create zip file
	zipFile zip_file = zipOpen(out_file, APPEND_STATUS_CREATE);
	if (zip_file == NULL) {
		return SC3CONVERT_EXCEPTION_ZIPCREATEZIP;
	}

	foreachdir(zip_file, dir_path, sc3convert_pack_file_from_dir);

	zipClose(zip_file, NULL);

	return SC3CONVERT_SUCCESS;
}

void sc3convert_pack_file_from_dir(void *obj, const char *path, const char *file_name) {
	zip_fileinfo zi;
	zi.tmz_date.tm_sec = 0;
	zi.tmz_date.tm_min = 0;
	zi.tmz_date.tm_hour = 0;
	zi.tmz_date.tm_mday = 0;
	zi.tmz_date.tm_mon = 0;
	zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	zipOpenNewFileInZip3(obj, file_name, &zi, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, NULL, 0);

	char name[MAX_FILENAME];
	strcpy(name, path);
	strcat(name, file_name);
	FILE *file = fopen(name, "rb");

	char buffer[100 * 1024];
	int size = 0;
	while (!feof(file)) {
		size = fread(buffer, 1, sizeof(buffer), file);
		zipWriteInFileInZip(obj, buffer, size);
		if (ferror(file)) {
			break;
		}
	}
	fclose(file);

	zipCloseFileInZip(obj);
}

json_object *sc3convert_load_opcode() {
	FILE *json_file = fopen("./opcode.json", "r");
	fseek(json_file, 0L, SEEK_END);
	size_t file_size = ftell(json_file);
	char *file_data = (char*)malloc(file_size + 1);
	fseek(json_file, 0L, SEEK_SET);
	fread(file_data, 1, file_size, json_file);
	fclose(json_file);
	json_object *opcode = json_tokener_parse(file_data);
	return opcode;
}

int sc3convert_convert_project(const char *name) {
	// Init md5 list
	single_list_string *md5_image = single_list_string_new();
	single_list_string *md5_sound = single_list_string_new();

	// Unpack file
	int code = sc3convert_unpack_file(name);
	if (code != SC3CONVERT_SUCCESS) {
		return code;
	}

	// Make input file path
	char input_path[MAX_FILENAME];
	strcpy(input_path, name);
	strcat(input_path, "t/");

	// Make output file path
	char output_path[MAX_FILENAME];
	strcpy(output_path, name);
	strcat(output_path, "t/e/");
	_mkdir(output_path);

	// project.json file
	char json_path[MAX_FILENAME];
	strcpy(json_path, name);
	strcat(json_path, "t/project.json");

	// Open json file
	FILE *json_file = fopen(json_path, "r");
	if (json_file == NULL) {
		return SC3CONVERT_EXCEPTION_CONVOPENFILE;
	}

	// Get file size
	fseek(json_file, 0L, SEEK_END);
	size_t file_size = ftell(json_file);
	char *file_data = (char*)malloc(file_size + 1);
	fseek(json_file, 0L, SEEK_SET);
	fread(file_data, 1, file_size, json_file);
	fclose(json_file);
	
	// Get opcode.json
	json_object *opcode = sc3convert_load_opcode();

	// Decode json object
	json_object *file_object = json_tokener_parse(file_data);

	// New json object for scratch2
	json_object *result_object = sc3convert_new_stage();
	json_object *child_object = json_object_object_get(result_object, "children");

	// Convert each target
	json_object *target_object = json_object_object_get(file_object, "targets");
	size_t array_length = json_object_array_length(target_object);
	for (size_t i = 0; i < array_length; ++i) {
		json_object *current_object = json_object_array_get_idx(target_object, i);
		json_object *item_object = NULL;
		json_object *temp_object = NULL;
		json_object *goal_object = NULL;

		// If stage or sprite
		if (json_object_get_boolean(json_object_object_get(current_object, "isStage"))) {
			item_object = result_object;
		}
		else {
			// Set name
			item_object = sc3convert_new_sprite(json_object_get_string(json_object_object_get(current_object, "name")));
			json_object_array_add(child_object, item_object);
		}

		// Variables
		temp_object = json_object_object_get(current_object, "variables");
		goal_object = json_object_object_get(item_object, "variables");
		struct json_object_iterator iter_beg = json_object_iter_begin(temp_object);
		struct json_object_iterator iter_end = json_object_iter_end(temp_object);
		while (!json_object_iter_equal(&iter_beg, &iter_end)) {
			json_object_array_add(goal_object, sc3convert_new_variable(json_object_iter_peek_value(&iter_beg)));
			json_object_iter_next(&iter_beg);
		}

		// Lists
		temp_object = json_object_object_get(current_object, "lists");
		goal_object = json_object_object_get(item_object, "lists");
		iter_beg = json_object_iter_begin(temp_object);
		iter_end = json_object_iter_end(temp_object);
		while (!json_object_iter_equal(&iter_beg, &iter_end)) {
			json_object_array_add(goal_object, sc3convert_new_list(json_object_iter_peek_value(&iter_beg)));
			json_object_iter_next(&iter_beg);
		}

		// Scripts
		temp_object = json_object_object_get(current_object, "blocks");
		goal_object = json_object_object_get(item_object, "scripts");
		iter_beg = json_object_iter_begin(temp_object);
		iter_end = json_object_iter_end(temp_object);
		while (!json_object_iter_equal(&iter_beg, &iter_end)) {
			json_object *block_item = json_object_iter_peek_value(&iter_beg);
			if (json_object_get_boolean(json_object_object_get(block_item, "topLevel"))) {
				json_object *block_part = json_object_new_array();
				json_object_array_add(block_part, json_object_object_get(block_item, "x"));
				json_object_array_add(block_part, json_object_object_get(block_item, "y"));
				json_object_array_add(block_part, sc3convert_new_script(opcode, temp_object, block_item));
				json_object_array_add(goal_object, block_part);
			}
			json_object_iter_next(&iter_beg);
		}

		// Costumes
		temp_object = json_object_object_get(current_object, "costumes");
		goal_object = json_object_object_get(item_object, "costumes");
		size_t length = json_object_array_length(temp_object);
		for (size_t i = 0; i < length; ++i) {
			json_object *old_costume = json_object_array_get_idx(temp_object, i);
			json_object *new_costume = json_object_new_object();
			json_object_object_add(new_costume, "costumeName", json_object_object_get(old_costume, "name"));
			json_object_object_add(new_costume, "baseLayerMD5", json_object_object_get(old_costume, "md5ext"));
			json_object_object_add(new_costume, "rotationCenterX", json_object_object_get(old_costume, "rotationCenterX"));
			json_object_object_add(new_costume, "rotationCenterY", json_object_object_get(old_costume, "rotationCenterY"));
			json_object *temp = json_object_object_get(old_costume, "bitmapResolution");
			// No bitmap resolution
			if (temp == NULL) {
				json_object_object_add(new_costume, "bitmapResolution", json_object_new_int(1));
			}
			// Contains bitmap resolution
			else {
				json_object_object_add(new_costume, "bitmapResolution", temp);
			}
			int asset_id = single_list_string_contains(md5_image, json_object_get_string(json_object_object_get(old_costume, "assetId")));
			// Not include md5
			if (asset_id == -1) {
				single_list_string_append(md5_image, json_object_get_string(json_object_object_get(old_costume, "assetId")));
				json_object_object_add(new_costume, "baseLayerID", json_object_new_int((*md5_image).length));
				// Change file, set path
				char filepath[_MAX_PATH], srcpath[_MAX_PATH], buffer[11];
				strcpy(filepath, output_path);
				_itoa((*md5_image).length, buffer, 10);
				strcat(filepath, buffer);
				strcat(filepath, ".");
				strcat(filepath, json_object_get_string(json_object_object_get(old_costume, "dataFormat")));

				strcpy(srcpath, input_path);
				strcat(srcpath, json_object_get_string(json_object_object_get(old_costume, "md5ext")));
				// Copy file
				copyfile(srcpath, filepath);
			}
			// Included md5
			else {
				json_object_object_add(new_costume, "baseLayerID", json_object_new_int(asset_id));
			}
			json_object_array_add(goal_object, new_costume);
		}

		// Current costume
		json_object_object_add(item_object, "currentCostumeIndex", json_object_object_get(current_object, "currentCostume"));

		// Sounds
		temp_object = json_object_object_get(current_object, "sounds");
		goal_object = json_object_object_get(item_object, "sounds");
		length = json_object_array_length(temp_object);
		for (size_t i = 0; i < length; ++i) {
			json_object *old_costume = json_object_array_get_idx(temp_object, i);
			json_object *new_costume = json_object_new_object();
			json_object_object_add(new_costume, "soundName", json_object_object_get(old_costume, "name"));
			json_object_object_add(new_costume, "baseLayerMD5", json_object_object_get(old_costume, "md5ext"));
			json_object_object_add(new_costume, "sampleCount", json_object_object_get(old_costume, "sampleCount"));
			json_object_object_add(new_costume, "rate", json_object_object_get(old_costume, "rate"));
			json_object_object_add(new_costume, "format", json_object_object_get(old_costume, "format"));
			int asset_id = single_list_string_contains(md5_sound, json_object_get_string(json_object_object_get(old_costume, "assetId")));
			// Not include md5
			if (asset_id == -1) {
				json_object_object_add(new_costume, "soundID", json_object_new_int(md5_image->length));
				single_list_string_append(md5_image, json_object_get_string(json_object_object_get(old_costume, "assetId")));
				// Change file, set path
				char filepath[_MAX_PATH], srcpath[_MAX_PATH], buffer[11];
				strcpy(filepath, output_path);
				_itoa(md5_image->length - 1, buffer, 10);
				strcat(filepath, buffer);
				strcat(filepath, ".");
				strcat(filepath, json_object_get_string(json_object_object_get(old_costume, "dataFormat")));

				strcpy(srcpath, input_path);
				strcat(srcpath, json_object_get_string(json_object_object_get(old_costume, "md5ext")));
				// Copy file
				copyfile(srcpath, filepath);
			}
			// Included md5
			else {
				json_object_object_add(new_costume, "soundID", json_object_new_int(asset_id));
			}
			json_object_array_add(goal_object, new_costume);
		}
	}

	// Output json
	char out_json[MAX_FILENAME];
	strcpy(out_json, output_path);
	strcat(out_json, "project.json");
	json_object_to_file(out_json, result_object);

	// Pack *.sb2
	char out_file[MAX_FILENAME];
	strcpy(out_file, name);
	strcat(out_file, ".sb2");
	code = sc3convert_pack_file(output_path, out_file);
	if (code != SC3CONVERT_SUCCESS) {
		return code;
	}

	// Delete temp file
	removeall(input_path);

	return SC3CONVERT_SUCCESS;
}

json_object *sc3convert_new_script(json_object *opcode, json_object *blocks, json_object *block_item) {
	json_object *result = json_object_new_array();
	while (true) {
		json_object *block = json_object_new_array();
		const char *t = json_object_get_string(json_object_object_get(block_item, "opcode"));
		json_object *current_opcode = json_object_object_get(opcode, t);
		if (current_opcode != NULL) {
			// Get name in scratch2
			json_object_array_add(block, json_object_array_get_idx(current_opcode, 0));
			// Loop for each param of block (in the order of scratch2)
			json_object *param = json_object_array_get_idx(current_opcode, 1);
			json_object *inputs = json_object_object_get(block_item, "inputs");
			json_object *fields = json_object_object_get(block_item, "fields");
			size_t length = json_object_array_length(param);
			for (size_t i = 0; i < length; ++i) {
				const char *string = json_object_get_string(json_object_array_get_idx(param, i));
				// Value in inputs
				if (json_object_object_get(inputs, string) != NULL) {
					json_object *temp = json_object_object_get(inputs, string);
					int type = json_object_get_int(json_object_array_get_idx(temp, 0));
					// [1] immediate value or value in fields
					if (type == 1) {
						json_type type = json_object_get_type(json_object_array_get_idx(temp, 1));
						// [array] immediate value
						if (type == json_type_array) {
							json_object_array_add(block, json_object_array_get_idx(json_object_array_get_idx(temp, 1), 1));
						}
						// [string] value in fields
						else if (type == json_type_string) {
							temp = json_object_object_get(json_object_object_get(blocks, json_object_get_string(json_object_array_get_idx(temp, 1))), "fields");
							json_object_array_add(block, json_object_array_get_idx(json_object_object_get(temp, string), 0));
						}
					}
					// [2] C/E block
					else if (type == 2) {
						json_object_array_add(block, sc3convert_new_script(opcode, blocks, json_object_object_get(blocks, json_object_get_string(json_object_array_get_idx(temp, 1)))));
					}
					// [3] block-insert value
					else if (type == 3) {
						json_object *tmp = json_object_array_get_idx(temp, 1);
						json_type type = json_object_get_type(tmp);
						// [array] variable
						if (type == json_type_array) {
							json_object *tmp_block = json_object_new_array();
							json_object_array_add(tmp_block, json_object_new_string("readVariable"));
							json_object_array_add(tmp_block, json_object_array_get_idx(tmp, 1));
							json_object_array_add(block, tmp_block);
						}
						// [string] other insert
						else {
							json_object_array_add(block, json_object_array_get_idx(sc3convert_new_script(opcode, blocks, json_object_object_get(blocks, json_object_get_string(tmp))), 0));
						}
					}
				}
				// Value in fields
				else if (json_object_object_get(fields, string) != NULL) {
					json_object_array_add(block, json_object_array_get_idx(json_object_object_get(fields, string), 0));
				}
			}
			// Push block
			json_object_array_add(result, block);
		}
		// Next
		json_object *tmp = json_object_object_get(block_item, "next");
		if (json_object_get_type(tmp) == json_type_null) {
			break;
		}
		else {
			block_item = json_object_object_get(blocks, json_object_get_string(tmp));
		}
	}
	return result;
}

json_object *sc3convert_new_object(const char *name) {
	json_object *result_object = json_object_new_object();
	json_object_object_add(result_object, "objName", json_object_new_string(name));
	json_object_object_add(result_object, "variables", json_object_new_array());
	json_object_object_add(result_object, "lists", json_object_new_array());
	json_object_object_add(result_object, "scripts", json_object_new_array());
	json_object_object_add(result_object, "scriptComments", json_object_new_array());
	json_object_object_add(result_object, "costumes", json_object_new_array());
//	json_object_object_add(result_object, "currentCostumeIndex", json_object_new_int(0));
	json_object_object_add(result_object, "sounds", json_object_new_array());
	return result_object;
}

json_object *sc3convert_new_stage() {
	json_object *info = json_object_new_object();
//	json_object_object_add(info, "flashVersion", json_object_new_string("WIN 32,0,0,100"));
	json_object_object_add(info, "videoOn", json_object_new_boolean(false));
//	json_object_object_add(info, "swfVersion", json_object_new_string(""));
	json_object_object_add(info, "metaData", json_object_new_string("ad82caf04f3a3976353656cf87ace2ae"));
	json_object_object_add(info, "userAgent", json_object_new_string("Scratch 2.0 Offline Editor"));
	json_object_object_add(info, "comment", json_object_new_string("Scratch3Convert"));
	json_object *result_object = sc3convert_new_object("Stage");
	json_object_object_add(result_object, "children", json_object_new_array());
	json_object_object_add(result_object, "tempoBPM", json_object_new_double(.0));
	json_object_object_add(result_object, "videoAlpha", json_object_new_double(.0));
//	json_object_object_add(result_object, "penLayerID", json_object_new_int(0));
//	json_object_object_add(result_object, "penLayerMD5", json_object_new_string(""));
	json_object_object_add(result_object, "info", info);
	return result_object;
}

json_object *sc3convert_new_sprite(const char *name) {
	json_object *result_object = sc3convert_new_object(name);
	json_object_object_add(result_object, "scratchX", json_object_new_int(0));
	json_object_object_add(result_object, "scratchY", json_object_new_int(0));
	json_object_object_add(result_object, "scale", json_object_new_double(.0));
	json_object_object_add(result_object, "direction", json_object_new_int(0));
	json_object_object_add(result_object, "rotationStyle", json_object_new_string("normal"));
	json_object_object_add(result_object, "isDraggable", json_object_new_boolean(false));
	json_object_object_add(result_object, "indexInLibrary", json_object_new_int(0));
	json_object_object_add(result_object, "visible", json_object_new_boolean(false));
	json_object_object_add(result_object, "spriteInfo", json_object_new_array());
	return result_object;
}

json_object *sc3convert_new_variable(json_object *object) {
	json_object *result_object = json_object_new_object();
	json_object_object_add(result_object, "name", json_object_array_get_idx(object, 0));
	json_object_object_add(result_object, "value", json_object_array_get_idx(object, 1));
	json_object_object_add(result_object, "isPersistent", json_object_new_boolean(false));
	return result_object;
}

json_object *sc3convert_new_list(json_object *object) {
	json_object *result_object = json_object_new_object();
	json_object_object_add(result_object, "listName", json_object_array_get_idx(object, 0));
	json_object_object_add(result_object, "contents", json_object_array_get_idx(object, 1));
	json_object_object_add(result_object, "isPersistent", json_object_new_boolean(false));
	json_object_object_add(result_object, "x", json_object_new_int(0));
	json_object_object_add(result_object, "y", json_object_new_int(0));
	json_object_object_add(result_object, "width", json_object_new_int(0));
	json_object_object_add(result_object, "height", json_object_new_int(0));
	json_object_object_add(result_object, "visible", json_object_new_boolean(false));
	return result_object;
}

//test only
int main() {
	//char name[BUFFER_SIZE];
	//scanf("%s", &name);
	return sc3convert_convert_project("D:/Workspace/Program/Scratch/Alka/alka.sb3");
}

