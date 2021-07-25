#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdint.h>

extern "C"
{
    __declspec(dllexport) void FloydWarshallAsm8(int8_t* graph, int n);
    __declspec(dllexport) void FloydWarshallAsm16(int16_t* graph, int n);
    __declspec(dllexport) void FloydWarshallAsm32(int32_t* graph, int n);
}
