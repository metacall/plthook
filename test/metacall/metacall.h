#ifndef LIBMETACALL
#define LIBMETACALL 1

#if defined(WIN32) || defined(_WIN32)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int load_normal_executable(void);
EXPORT int load_node_dynamic(void);
EXPORT int load_node_static(char *(*string_function)(void));

#endif
