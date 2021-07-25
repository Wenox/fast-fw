.data
n  qword ? ; <- graph size |V| loaded from RDX
n2 qword ? ; <- |V|^2

.code

; Three different procedures are supported (see below):
; - FloydWarshallAsm8
; - FloydWarshallAsm16
; - FloydWarshallAsm32
; 
; For example, 8-bit version is using 8-bit types as graph's weights,
; and also 8-bit SSE instructions accordingly.
;
; I do not want to write excessive comments, I assume reader is familiar with ASM.


; Stores array's data element offset into REG_OUT.
; 1d array is used everywhere.
; But the algorithm conceptually uses 2d array.
; Therefore 1d array is treated as 2d array.
;
; This macro is used for int8_t procedure.
store_byte_offset_into macro    REG_OUT, ROW, COL
        mov  REG_OUT, COL       ; for example: rdi = j
        imul REG_OUT, n         ; for example: rdi = n * j
        add  REG_OUT, ROW       ; for example: rdi = n * j + i
endm

; This macro is used for int16_t procedure.
store_word_offset_into macro    REG_OUT, ROW, COL
        store_byte_offset_into  REG_OUT, ROW, COL
        shl REG_OUT, 1          ; for example: rdi = 2 * (n * j + i) ;   <-- final data offset
endm

; This macro is used for int32_t procedure.
store_dword_offset_into macro   REG_OUT, ROW, COL
        store_byte_offset_into  REG_OUT, ROW, COL
        shl REG_OUT, 2          ; for example: rdi = 4 * (n * j + i) ;   <-- final data offset
endm

init macro
        push r13
        push r12
        push r10
        push rdi

        xor  r8,  r8            ; k loop (outer-most loop)
        xor  r9,  r9            ; j loop (middle loop)
        xor  r10, r10           ; i loop (inner-most loop)
        
        mov  n,   rdx           ; n = |V| - graph size
        imul rdx, rdx           ; calculate |V|^2
        mov  n2,  rdx           ; n2 = |V|^2
endm

clean_up macro
        pop  rdi
        pop  r10
        pop  r12
        pop  r13
endm

; The very core of the algorithm basically does two things:
; - 1. Reads some data (stores it into YMM registers)
; - 2. Does algebraic operations and evaluates expressions on that data.
; This macro is used for int8_t procedure.
handle_byte_weight macro
        store_byte
        operations_byte
endm

; This macro is used for int16_t procedure.
handle_word_weight macro
        store_word
        operations_word
endm

; This macro is used for int32_t procedure.
handle_dword_weight macro
        store_dword
        operations_dword
endm

; int8_t weights version macro
; YMM10 - stores [i][j] data
; YMM11 - stores [i][k] data
; XMM12 - stores [k][j] data, that is ready to be broadcasted onto YMM12
store_byte macro
        store_byte_offset_into RDI, R10, R9          ; RDI stores final packed data offset, used for R/W operations
        vmovdqu ymm10, ymmword ptr [rcx + rdi]       ; ymm10 is [i][j]: = { [rcx], [rcx + 1], ... , [rcx + 31] }

        store_byte_offset_into RAX, R10, R8          ; RAX stores offset to [i][k] 
        vmovdqu ymm11, ymmword ptr [rcx + rax]       ; ymm11 is [i][k]

        store_byte_offset_into RAX, R8, R9           ; RAX stores offset to [k][j]
        ; !! this would do the job, but is legal only on the bleedig-edge newest CPUs (2020):
        ; !! vpbroadcastd ymm12, r13 (illegal AVX-256, legal AVX-512)

        mov  r13,  [rcx + rax]                       ; therefore I must do this workaround (gcc does the same)
        movq xmm12, r13
endm

; int16_t version macro
store_word macro
        store_word_offset_into RDI, R10, R9          ; RDI stores final packed data offset, used for R/W operations
        vmovdqu ymm10, ymmword ptr [rcx + rdi]       ; ymm10 is [i][j]: = { [rcx], [rcx + 2], ... , [rcx + 30] }

        store_word_offset_into RAX, R10, R8          ; RAX stores offset to [i][k] 
        vmovdqu ymm11, ymmword ptr [rcx + rax]       ; ymm11 is [i][k]

        store_word_offset_into RAX, R8, R9           ; RAX stores offset to [k][j]

        mov  r13,  [rcx + rax]                       ; therefore I must do this workaround (gcc does the same)
        movq xmm12, r13
endm

; int32_t version macro
store_dword macro
        store_dword_offset_into RDI, R10, R9         ; RDI stores final packed data offset, used for R/W operations
        vmovdqu ymm10, ymmword ptr [rcx + rdi]       ; ymm10 is [i][j]: = { [rcx], [rcx + 4], ... , [rcx + 28] }

        store_dword_offset_into RAX, R10, R8         ; RAX stores offset to [i][k] 
        vmovdqu ymm11, ymmword ptr [rcx + rax]       ; ymm11 is [i][k]

        store_dword_offset_into RAX, R8, R9          ; RAX stores offset to [k][j]
        mov  r13,  [rcx + rax]                       ; therefore I must do this workaround (gcc does the same)
        movq xmm12, r13
endm

; Version for 8-bit weights
; The macro first broadcasts XMM12 onto YMM12.
; Then adds [i][k] + [k][j]
; And finally minimum of minimum of ([i][j], [i][k] + [k][j]) is stored
operations_byte macro
        vpbroadcastb  ymm12,  xmm12                  ; ymm12 is [k][j] --- all values are the same
        vpaddb        ymm13,  ymm12,  ymm11          ; sum pairs: [i][k] + [k][j]
        vpminub       ymm14,  ymm13,  ymm10          ; store pairs: min( [i][j], [i][k] + [k][j] ) into ymm14
endm

; Version for 16-bit weights
operations_word macro
        vpbroadcastw  ymm12,  xmm12                  ; ymm12 is [k][j] --- all values are the same
        vpaddw        ymm13,  ymm12,  ymm11          ; sum pairs: [i][k] + [k][j]
        vpminuw       ymm14,  ymm13,  ymm10          ; store pairs: min( [i][j], [i][k] + [k][j] ) into ymm14
endm

; Version for 32-bit weights 
operations_dword macro
        vpbroadcastd  ymm12,  xmm12                  ; ymm12 is [k][j] --- all values are the same
        vpaddd        ymm13,  ymm12,  ymm11          ; sum pairs: [i][k] + [k][j]
        vpminud       ymm14,  ymm13,  ymm10          ; store pairs: min( [i][j], [i][k] + [k][j] ) into ymm14
endm


; The Floyd Warshall algorithm runner. 
; Does initialization, executes the core inside the O(|V|^3) triple loop and cleans up.
;
; @param fw_core - the macro that is a core of the algorithm. Either 8-bit, 16-bit or 32-bit version.
; @param parallelization_number - by how much should the inner-most loop be incremented: either 32, 16 or 8.
floyd_warshall macro fw_core, parallelization_number
        init
k_loop:
        cmp r8, n                                    ; k == n is an exit condition
        je done

j_loop:
        cmp r9, n
        je k_loop_start
         
i_loop:
        cmp r10, n
        je j_loop_start

        ; <<< i loop body --- inner most loop >>>
        fw_core
        lea r12, [rcx + rdi]                         ; r12 is an address data is written to       
        vmovdqu ymmword ptr [r12], ymm14             ; write the minimum pairs into [i][j]

noop_iteration:
    
        add r10d, parallelization_number            ; add 8, 16 or 32 depending on weight type
        jmp i_loop
       
j_loop_start:
        xor r10d, r10d

        ; <<< j loop body --- middle loop >>>
        inc r9d
        jmp j_loop

k_loop_start:
        xor r9d, r9d

        ; <<< k loop body --- outer-most loop >>>
        inc r8d
        jmp k_loop
   
done:
        clean_up
        ret
endm

; The procedure that handles 8-bit graph's edges weights.
FloydWarshallAsm8 proc  EXPORT
        floyd_warshall          handle_byte_weight,             32
FloydWarshallAsm8 endp

; The procedure that handles 16-bit graph's edges weights.
FloydWarshallAsm16 proc  EXPORT
        floyd_warshall          handle_word_weight,             16
FloydWarshallAsm16 endp

; The procedure that handles 32-bit graph's edges weights.
FloydWarshallAsm32 proc  EXPORT
        floyd_warshall          handle_dword_weight,            8
FloydWarshallAsm32 endp

end