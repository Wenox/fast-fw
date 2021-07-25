#include "floyd_warshall.h"

void floyd_warshall_cpp2(int8_t* graph, int n) {
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
