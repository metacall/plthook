#ifndef LIBNODE
#define LIBNODE 1

#if defined(WIN32) || defined(_WIN32)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT char *string_function(void);

#endif
