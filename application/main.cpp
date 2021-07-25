#include <iostream>
#include "graph.h"
#include "windows.h"
#include "benchmarks.h"
#include <cstdint>

/** 
  * Runs all Floyd-Warshall algorithm variants:
  * - C++
  * - ASM 8-bit
  * - ASM 16-bit
  * - ASM 32-bit
  * 
  * Generates graph, runs all algorithms and tests results correctness.
  * 
  * See "modes.h" for additional MIN/MAX configuration.
  * Set the INF to significantly larger value than MAX.
  */
void runAllVariants(int graphSize) {
        // Given:  Generate graph and create deep copies of it.
        auto generatedGraph8 = generateGraph<int8_t>(graphSize);
        auto copied16        = copy<int16_t>(generatedGraph8, graphSize);
        auto copied32        = copy<int32_t>(generatedGraph8, graphSize);
        auto copiedCpp       = copy<int32_t>(generatedGraph8, graphSize);

        // When:  Run the Floyd-Warshall algorithm on each graph.
        auto graphAsm8       = run(Asm8,  generatedGraph8, graphSize);
        auto graphAsm16      = run(Asm16, copied16,        graphSize);
        auto graphAsm32      = run(Asm32, copied32,        graphSize);
        auto graphCpp        = run(Cpp,   copiedCpp,       graphSize);

        // Then:  ASM and C++ resuls should be equal.
        assertEquals<int8_t> (graphAsm8,  graphCpp, graphSize, "8 bit");
        assertEquals<int16_t>(graphAsm16, graphCpp, graphSize, "16 bit");
        assertEquals<int32_t>(graphAsm32, graphCpp, graphSize, "32 bit");
}

/** 
  * Runs only the Asm32 and C++ variants.
  * 
  * See "modes.h" for additional MIN/MAX configuration.
  * Remember to set the INF to significantly larger value than MAX.
*/
void runSimplified() {
        constexpr auto graphSize = 32; ///< MUST BE multiplication of 16 when using 16-bit weights mode.
        auto generatedGraph16 = generateGraph<int32_t>(graphSize);
        auto copiedCpp = copy<int32_t>(generatedGraph16, graphSize);

        run(Asm32, generatedGraph16, graphSize);
        run(Cpp,  copiedCpp,        graphSize);

        assertEquals<int32_t>(generatedGraph16, copiedCpp, graphSize, "32 bit");
}

int main(int argc, char** argv) {
        runAllVariants(32);

        // min |V| = 512, max |V| = 1280, step = 32, repetitions = 100
        // benchmark(512, 1280, 32, 100); 

        return 0;
}


