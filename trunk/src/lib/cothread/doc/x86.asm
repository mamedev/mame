; fastcall ABI:
; return = eax
; arguments = ecx,edx
; non-volatile registers = ebp,esi,edi,ebx

co_swap_fastcall(to = ecx, from = edx):
  mov [edx],esp
  mov esp,[ecx]
  pop eax

  mov [edx+ 4],ebp
  mov [edx+ 8],esi
  mov [edx+12],edi
  mov [edx+16],ebx

  mov ebp,[ecx+ 4]
  mov esi,[ecx+ 8]
  mov edi,[ecx+12]
  mov ebx,[ecx+16]

  jmp eax
