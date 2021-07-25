#include "pch.h"
#include <iostream>

/** 
  * Solves All-Pairs shortest path problem using Floyd Warshall algorithm.
  * 
  * @param1 graph   pointer to the graph that is to be mutated
  * @param2 n       graph size |V|
  * 
  * The CPP version works with 32-bit graph weights. 
  * For the C++ procedure it does not matter whether 32-bit, 16-bit or 8-bit weights are used.
  */
void FloydWarshallCpp(int32_t* graph, int n) {
	for (int k = 0; k < n; ++k) {
		for (int j = 0; j < n; ++j) {
			for (int i = 0; i < n; ++i) {
				auto jn = j * n;
				auto newWeight = graph[i + k * n] + graph[k + jn];
				auto index = i + jn;
				if (graph[index] > newWeight) {
					graph[index] = newWeight;
				}
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

