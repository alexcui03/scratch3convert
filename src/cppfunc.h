#ifndef CPPFUNC_H
#define CPPFUNC_H

#ifdef __cplusplus
extern "C" {
#endif
	typedef void(*file_iter_func)(void *, const char *, const char *);

	void copyfile(const char *src, const char *name);
	void foreachdir(void *obj, const char *path, file_iter_func func);
	void removeall(const char *path);

#ifdef __cplusplus
}
#endif

#endif
