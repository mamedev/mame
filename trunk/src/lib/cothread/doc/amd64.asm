; SystemV ABI:
; return = rax
; arguments = rdi,rsi,rdx,rcx,r8,r9
; non-volatile registers = rbp,rbx,r12-r15

co_swap_systemV(to = rdi, from = rsi):
  mov [rsi],rsp
  mov rsp,[rdi]
  pop rax

  mov [rsi+ 8],rbp
  mov [rsi+16],rbx
  mov [rsi+24],r12
  mov [rsi+32],r13
  mov [rsi+40],r14
  mov [rsi+48],r15

  mov rbp,[rdi+ 8]
  mov rbx,[rdi+16]
  mov r12,[rdi+24]
  mov r13,[rdi+32]
  mov r14,[rdi+40]
  mov r15,[rdi+48]

  jmp rax

; Win64 ABI:
; return = rax
; arguments = rcx,rdx,r8,r9
; non-volatile registers = rbp,rsi,rdi,rbx,r12-r15
; non-volatile FPU registers = xmm6-xmm15

co_swap_win64(to = rcx, from = rdx):
  mov [rdx],rsp
  mov rsp,[rcx]
  pop rax

  mov [rdx+  8],rbp
  mov [rdx+ 16],rsi
  mov [rdx+ 24],rdi
  mov [rdx+ 32],rbx
  mov [rdx+ 40],r12
  mov [rdx+ 48],r13
  mov [rdx+ 56],r14
  mov [rdx+ 64],r15

  add rdx,128
  and rdx,~15

  movaps [rdx+  0],xmm6
  movaps [rdx+ 16],xmm7
  movaps [rdx+ 32],xmm8
  movaps [rdx+ 48],xmm9
  movaps [rdx+ 64],xmm10
  movaps [rdx+ 80],xmm11
  movaps [rdx+ 96],xmm12
  movaps [rdx+112],xmm13
  movaps [rdx+128],xmm14
  movaps [rdx+144],xmm15

  mov rbp,[rcx+  8]
  mov rsi,[rcx+ 16]
  mov rdi,[rcx+ 24]
  mov rbx,[rcx+ 32]
  mov r12,[rcx+ 40]
  mov r13,[rcx+ 48]
  mov r14,[rcx+ 56]
  mov r15,[rcx+ 64]

  add rcx,128
  and rcx,~15

  movaps [rcx+  0],xmm6
  movaps [rcx+ 16],xmm7
  movaps [rcx+ 32],xmm8
  movaps [rcx+ 48],xmm9
  movaps [rcx+ 64],xmm10
  movaps [rcx+ 80],xmm11
  movaps [rcx+ 96],xmm12
  movaps [rcx+112],xmm13
  movaps [rcx+128],xmm14
  movaps [rcx+144],xmm15

  jmp rax

