#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <stdint.h>

extern "C"
{
        __declspec(dllexport) void FloydWarshallCpp(int32_t* graph, int n);
}