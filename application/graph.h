#pragma once
#include "modes.h"
#include "floyd_warshall.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <fstream>


/** 
  * Verifies whether two graphs are equal.
  * Used to verify an equality of C++ and ASM results.
  */
template <typename T1, typename T2 = int32_t>
bool assertEquals(T1* graphAsm, T2* graphCpp, int size, const std::string& asmDescription) {
	auto sizeSquared = size * size;
	for (int i = 0; i < size; ++i) {
		if ((int)graphAsm[i] != (int)graphCpp[i]) {
			std::cout << "\n\n=====================================\n";
			std::cout << "TEST FAILED: The CPP and ASM " << asmDescription << " results are NOT equal\n";
			return false;
		}
	}
	std::cout << "\n\n=====================================\n";
	std::cout << "TEST SUCCESS: The CPP and ASM " << asmDescription << " results are equal\n";
	return true;
}

/** 
  * Vizualizes the graph of a given type and size.
  */
template <typename T>
void print(T* graph, int size) {
	std::cout << "Printing graph of size |" << size << "|:\n";
	for (int j = 0; j < size; ++j) {
		for (int i = 0; i < size; ++i) {
			if (graph[i + j * size] != INF) {
				std::cout << std::setw(4) << (int)graph[i + j * size] << " ";
			}
			else {
				std::cout << std::setw(4) << " " << " ";
			}
		} std::cout << "\n";
	} std::cout << "\n";
}

/** 
  * Runs Floyd-Warshall algorithm for the passed graph.
  * This is a function template that is a facade to the main program's functionality.
  * 
  * The result is computed based on one of three available ASM functions, or C++ function.
  * Appropariate function is loaded dynamically from the DLL.
  * Fails fast when the DLL or the procedure cannot be loaded.
  * Side effect: prints the time it took to calculate shortest distances.
  * 
  * @param procedure   - the name of the procedure: FloydWarshallAsm[8|16|32] or FloydWarshallCpp
  * @param graph       - pointer to a graph whose weights are of type T
  * @param graphSize   - |V|
  * @param printGraph  - specifies whether or not the graph should be printed to the screen
  * @return            - pointer to the same graph whose weights are of type T as @graph,
  *                      however the data is modified and now contains shortest distances
  *                      between all pairs, if available.
  */
template<typename T>
T* run(LPCSTR procedure, T* graph, int graphSize, boolean printGraph = true) {
	std::cout << "All-pairs shortest path: |V| = " << graphSize << "\n\n";
	if (procedure == Asm8) {
		std::cout << "Computing with ASM 8-bit weights procedure... \n";
	}
	else if (procedure == Asm16) {
		std::cout << "Computing with ASM 16-bit weights procedure... \n";
	}
	else if (procedure == Asm32) {
		std::cout << "Computing with ASM 32-bit weights procedure... \n";
	}
	else if (procedure == Cpp) {
		std::cout << "Computing with C++ procedure... \n";
	}
	else {
		throw std::invalid_argument{ "ERROR: No such algorithm procedure type available!\n" };
	}

	LPCWSTR dllFile = procedure == Cpp ? CPP_DLL : ASM_DLL;
	HINSTANCE dll = LoadLibrary(dllFile);
	if (!dll) {
		std::cerr << "Cannot open dll\n";
		return 0;
	}

	floyd_warshall<T> fw = (floyd_warshall<T>)GetProcAddress(dll, (LPCSTR)procedure);
	if (!fw) {
		std::cerr << "Cannot find procedure: " << procedure << std::endl;
	}
	else {
		if (printGraph) {
			print(graph, graphSize);
		}

		auto started = std::chrono::high_resolution_clock::now();
		fw(graph, graphSize);
		auto done = std::chrono::high_resolution_clock::now();

		if (printGraph) {
			std::cout << "\n==========================================================================\nSOLUTION\n==========================================================================\n";
			print(graph, graphSize);
		}

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();
		if (procedure == Asm8) {
			std::cout << "Execution time - ASM 8-bit:\t" << ms << " ms " << std::endl;
		}
		else if (procedure == Asm16) {
			std::cout << "Execution time - ASM 16-bit:\t" << ms << " ms " << std::endl;
		}
		else if (procedure == Asm32) {
			std::cout << "Execution time - ASM 32-bit:\t" << ms << " ms " << std::endl;
		}
		else if (procedure == Cpp) {
			std::cout << "Execution time - C++:\t" << ms << " ms " << std::endl;
		}
		std::cout << "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n";

	}
	return graph;
}

/**
  * Generates a new graph in the form of an adjacency matrix.
  * 
  * @param size
  * @param probability - probability for given edge (i, j) to be genearted. 
  * @return            - pointer to a new graph whose weights are of type T.
  */
template <typename T = uint32_t>
T* generateGraph(int size, int probability = 12) {
	T* graph = new T[size * size];

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> rand_edge(MIN, MAX);
	std::uniform_int_distribution<std::mt19937::result_type> rand_prob(1, 100);

	for (int j = 0; j < size; ++j) {
		for (int i = 0; i < size; ++i) {
			bool shouldGenerateEdge = rand_prob(rng) <= probability;
			if (shouldGenerateEdge) {
				graph[i + j * size] = rand_edge(rng);
			}
			else {
				graph[i + j * size] = INF;
			}
		}
	}

	for (int i = 0; i < size; ++i) {
		graph[i + i * size] = 0;
	}

	return graph;
}

/** 
  * Serializes the graph into a file.
  * 
  * @param graph - pointer to a graph whose weights are of type T
  * @param size
  * @param file
  */
template <typename T>
void write(T* graph, int size, const std::string& file = "graph.txt") {
	std::ofstream ost{ file };
	ost << size << std::endl;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			if (graph[i + j * size] != 0) {
				// <from>  <to>  <weight>
				ost << i << " " << j << " " << (int)graph[i + j * size] << std::endl;
			}
		}
	}
}

/** 
  * Reads the graph from a file. 
  * Currently unused, but left here to complete the API for the program's user.
  *  
  * @param file
  * @return pointer to a graph whose weights are of type T
  */
template <typename T>
T* read(const std::string& file = "graph.txt") {
	std::ifstream ist{ file };
	if (!ist) {
		throw std::runtime_error{ "Could not load graph from: " + file };
	}

	int size = 0;
	ist >> size;
	if (size <= 0) {
		throw std::runtime_error{ "Graph size must be positive, but it was " + size };
	}

	T* graph = new T[size * size];
	for (int j = 0; j < size; ++j) {
		for (int i = 0; i < size; ++i) {
			graph[i + j * size] = (int) INF;
		}
	}

	int from = 0;
	int to = 0;
	char weight = 0;
	while (ist >> from >> to >> weight) {
		graph[from + to * size] = weight;
	}

	for (int i = 0; i < size; ++i) {
		graph[i + i * size] = 0;
	}

	return graph;
}


/** 
  * Allocates new graph copy.
  * Used to copy graph into a different graph's type,
  *   for example: int32_t --> int8_t
  * 
  * @param src - pointer to a source graph whose weights are of type T2
  * @param size
  * @return pointer to a newly allocated graph of type T2
  */
template <typename T1, typename T2>
T1* copy(T2* src, int size) {
	T1* dest = new T1[size * size];
	for (int i = 0; i < size * size; ++i) {
		dest[i] = (int) src[i];
	}
	return dest;
}