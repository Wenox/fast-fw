#include "benchmarks.h"
#include "graph.h"
#include <iostream>

constexpr float repetitions = 10.0;
constexpr int minSize = 4096;
constexpr int maxSize = 4096;
constexpr int step = 32;

std::vector<float> benchmarksAsm8(1 + (maxSize - minSize) / step);
std::vector<float> benchmarksAsm16(1 + (maxSize - minSize) / step);
std::vector<float> benchmarksAsm32(1 + (maxSize - minSize) / step);
std::vector<float> benchmarksCpp(1 + (maxSize - minSize) / step);

void benchmarkCase(int index, int graphSize) {
        auto  generatedGraph8 = generateGraph<int8_t>(graphSize);
        auto* copied16        = copy<int16_t>(generatedGraph8, graphSize);
        auto* copied32        = copy<int32_t>(generatedGraph8, graphSize);
        auto* copied32cpp     = copy<int32_t>(generatedGraph8, graphSize);

        int resultAsm8    = runBenchmark(Asm8,  generatedGraph8, graphSize);
        int resultAsm16   = runBenchmark(Asm16, copied16,        graphSize);
        int resultAsm32   = runBenchmark(Asm32, copied32,        graphSize);
        int resultCpp     = runBenchmark(Cpp,   copied32cpp,     graphSize);

        benchmarksAsm8[index]  += resultAsm8;
        benchmarksAsm16[index] += resultAsm16;
        benchmarksAsm32[index] += resultAsm32;
        benchmarksCpp[index]   += resultCpp;
}

void benchmark(int min, int max, int step, int repetitions) {
        auto n = (max - min) / step;
        for (int reps = 0; reps < repetitions; ++reps) {
                std::cout << "Repetition: " << reps << std::endl;
                for (int i = 0; i <= n; i += 1) {
                        auto currentGraphSize = min + i * step;
                        std::cout << "Test for: " << currentGraphSize << std::endl;
                        benchmarkCase(i, currentGraphSize);
                }
        }
        saveBenchmarks(min, step, repetitions);
}


void saveBenchmarks(int min, int step, float repetitions) {
        std::cout << "Saving benchmarks...\n";

        std::ofstream ost1{ "asm8_3.txt" };
        auto size = benchmarksAsm8.size();
        for (int i = 0; i < size; ++i) {
                ost1 << (step * (i)) + min << " " << benchmarksAsm8[i] / repetitions << std::endl;
        }

        std::ofstream ost2{ "asm16_3.txt" };
        size = benchmarksAsm16.size();
        for (int i = 0; i < benchmarksAsm16.size(); ++i) {
                ost2 << (step * (i)) + min << " " << benchmarksAsm16[i] / repetitions << std::endl;
        }

        std::ofstream ost3{ "asm32_3.txt" };
        size = benchmarksAsm8.size();
        for (int i = 0; i < benchmarksAsm32.size(); ++i) {
                ost3 << (step * (i)) + min << " " << benchmarksAsm32[i] / repetitions << std::endl;
        }

        std::ofstream ost4{ "cpp_3.txt" };
        size = benchmarksCpp.size();
        for (int i = 0; i < size; ++i) {
                ost4 << (step * (i)) + minSize << " " << benchmarksCpp[i] / repetitions << std::endl;
        }

        std::cout << "All saved successfully\n";
}