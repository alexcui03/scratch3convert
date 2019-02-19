#define _HAS_CXX17 1

#include "cppfunc.h"

#include <filesystem>

void copyfile(const char *src, const char *name) {
	std::filesystem::copy_file(src, name, std::filesystem::copy_options::overwrite_existing);
}

void foreachdir(void *obj, const char *path, file_iter_func func) {
	for (auto& p : std::filesystem::directory_iterator(path)) {
		func(obj, path, p.path().filename().generic_string().c_str());
	}
}

void removeall(const char *path) {
	std::filesystem::remove_all(path);
}

