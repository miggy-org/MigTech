#pragma once

#include <stdint.h>
#include <time.h>

#include <string>
#include <stdexcept>
#include <map>
#include <list>
#include <vector>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// included in Windows headers but not defined by Android
#define uint64 uint64_t
#define nullptr NULL
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])
#define byte unsigned char
