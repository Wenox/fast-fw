#pragma once

/** Template is used because to support multiple assembler procedures. */
template <typename T>
using floyd_warshall = void (__fastcall *) (T* graph, int size);
