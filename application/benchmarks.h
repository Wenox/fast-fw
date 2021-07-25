#pragma once
#include <vector>
#include "modes.h"
#include "graph.h"
#include "windows.h"
#include "floyd_warshall.h"

void testcase(int index, int graphSize);

void benchmark(int min, int max, int step, int repetitions);

void saveBenchmarks(int min, int step, float repetitions);

template<typename T>
auto runBenchmark(LPCSTR procedure, T* graph, int graphSize) {
        LPCWSTR dllFile = procedure == Cpp ? CPP_DLL : ASM_DLL;
        HINSTANCE dll = LoadLibrary(dllFile);
        if (!dll) {
                std::cout << "Cannot open dll\n";
                return 0;
        }

        floyd_warshall<T> fw = (floyd_warshall<T>)GetProcAddress(dll, (LPCSTR)procedure);
        if (!fw) {
                std::cerr << "Cannot find procedure: " << procedure << std::endl;
        }
        else {
                auto started = std::chrono::high_resolution_clock::now();
                fw(graph, graphSize);
                auto done = std::chrono::high_resolution_clock::now();
                int ms = std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();
                return ms;
        }
        return -1;
}