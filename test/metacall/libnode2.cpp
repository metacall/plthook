#include <cassert>
#include <iostream>
#include <cstdlib>

// This is a test to ensure that if libnode is already loaded either
// in static or dynamic compilation, it should never execute the constructors.
// We use NORMAL_EXECUTABLE as exception because when we are using metacall.exe,
// this constructor should be called as usual.
class should_not_call_init {
public:
    should_not_call_init() {
        char * normal_executable = getenv("NORMAL_EXECUTABLE");
        if (normal_executable == NULL) {
            std::cout << "should_not_call_init constructor has been called and it should not" << std::endl;
            assert(0);
        }
    }
};

static should_not_call_init test;
