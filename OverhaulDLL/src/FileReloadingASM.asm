_DATA SEGMENT

sub_1405299b0   dq  1405299b0h
label_1402dfbae dq  1402dfbaeh

_DATA ENDS

_TEXT    SEGMENT

FUNC_PROLOGUE macro
    pushfq 
    push    rax
    mov     rax,rsp
    and     rsp,-10h
    sub     rsp,000002A0h
    fxsave  [rsp+20h]
    mov     [rsp+00000220h],rbx
    mov     [rsp+00000228h],rcx
    mov     [rsp+00000230h],rdx
    mov     [rsp+00000238h],rsi
    mov     [rsp+00000240h],rdi
    mov     [rsp+00000248h],rax
    mov     [rsp+00000250h],rbp
    mov     [rsp+00000258h],r8
    mov     [rsp+00000260h],r9
    mov     [rsp+00000268h],r10
    mov     [rsp+00000270h],r11
    mov     [rsp+00000278h],r12
    mov     [rsp+00000280h],r13
    mov     [rsp+00000288h],r14
    mov     [rsp+00000290h],r15
endm

FUNC_EPILOGUE macro
    mov     r15,[rsp+00000290h]
    mov     r14,[rsp+00000288h]
    mov     r13,[rsp+00000280h]
    mov     r12,[rsp+00000278h]
    mov     r11,[rsp+00000270h]
    mov     r10,[rsp+00000268h]
    mov     r9, [rsp+00000260h]
    mov     r8, [rsp+00000258h]
    mov     rbp,[rsp+00000250h]
    mov     rdi,[rsp+00000240h]
    mov     rsi,[rsp+00000238h]
    mov     rdx,[rsp+00000230h]
    mov     rcx,[rsp+00000228h]
    mov     rbx,[rsp+00000220h]
    fxrstor [rsp+20h]
    mov     rsp,[rsp+00000248h]
    pop     rax
    popfq 
endm

FUNC_PROLOGUE_LITE macro
	push	r15
	mov		r15, rsp
	and		rsp, -10h
	sub		rsp, 0C0h
	movaps	[rsp + 0B0h], xmm0
	movaps	[rsp + 0A0h], xmm1
	movaps	[rsp + 90h], xmm2
	movaps	[rsp + 80h], xmm3
	movaps	[rsp + 70h], xmm4
	movaps	[rsp + 60h], xmm5
	mov		[rsp + 58h], rax
	mov		[rsp + 50h], rcx
	mov		[rsp + 48h], rdx
	mov		[rsp + 40h], r8
	mov		[rsp + 38h], r9
	mov		[rsp + 30h], r10
	mov		[rsp + 28h], r11
	mov		[rsp + 20h], r15
endm

FUNC_EPILOGUE_LITE macro
	mov		r15, [rsp + 20h]
	mov		r11, [rsp + 28h]
	mov		r10, [rsp + 30h]
	mov		r9, [rsp + 38h]
	mov		r8, [rsp + 40h]
	mov		rdx, [rsp + 48h]
	mov		rcx, [rsp + 50h]
	mov		rax, [rsp + 58h]
	movaps	xmm5, [rsp + 60h]
	movaps	xmm4, [rsp + 70h]
	movaps	xmm3, [rsp + 80h]
	movaps	xmm2, [rsp + 90h]
	movaps	xmm1, [rsp + 0A0h]
	movaps	xmm0, [rsp + 0B0h]
	mov		rsp, r15
	pop		r15
endm

EXTERN CalcCorrectGraph_injection_return: qword

PUBLIC CalcCorrectGraph_injection
CalcCorrectGraph_injection PROC
;original code
mov     [rsp+28h], rbx
movaps  xmm6, xmm0
call    qword ptr [sub_1405299b0]
mov     rdx, [rsp+28h]

;check the returned result isn't null
test    rdx, rdx
jz      funcAbort
jmp     CalcCorrectGraph_injection_return
funcAbort:
jmp     qword ptr [label_1402dfbae]
CalcCorrectGraph_injection ENDP


EXTERN get_ParamResCap_from_ParamMan_injection_return: qword
EXTERN SoloParamRes_curindex: dword

PUBLIC get_ParamResCap_from_ParamMan_injection
get_ParamResCap_from_ParamMan_injection PROC
;original code
cmp     ecx, 27h
jge     abort
test    edx, edx
js      abort
mov     r8, 141c7e000h
mov     r8, qword ptr [r8]
;custom code to change which SoloParamRes we're accessing
mov     edx, SoloParamRes_curindex
jmp     get_ParamResCap_from_ParamMan_injection_return
abort:
xor     eax, eax
ret
get_ParamResCap_from_ParamMan_injection ENDP


EXTERN call_SetHostPlayerIns_offset_injection_return: qword
extern call_SetHostPlayerIns_offset_helper: proc

PUBLIC call_SetHostPlayerIns_offset_injection
call_SetHostPlayerIns_offset_injection PROC

FUNC_PROLOGUE
mov     rcx, 1 ;lock
call    call_SetHostPlayerIns_offset_helper
FUNC_EPILOGUE

;original code
mov     rax, [rsi]
mov     rcx, rsi
call    qword ptr [rax+150h]
movaps  xmm7, xmmword ptr [rsp+0D0h]

FUNC_PROLOGUE
mov     rcx, 0 ;unlock
call    call_SetHostPlayerIns_offset_helper
FUNC_EPILOGUE

jmp     call_SetHostPlayerIns_offset_injection_return
call_SetHostPlayerIns_offset_injection ENDP


extern reload_chrctrl_injection_helper: proc

PUBLIC reload_chrctrl_location1_injection
reload_chrctrl_location1_injection PROC

;original code
mov     rbx, [rsp+70h]
add     rsp, 40h
pop     rdi
pop     rsi
pop     rbp

FUNC_PROLOGUE_LITE
call    reload_chrctrl_injection_helper
FUNC_EPILOGUE_LITE

ret
reload_chrctrl_location1_injection ENDP

PUBLIC reload_chrctrl_location2_injection
reload_chrctrl_location2_injection PROC

;original code
add     rsp, 50h
pop     rdi

FUNC_PROLOGUE_LITE
call    reload_chrctrl_injection_helper
FUNC_EPILOGUE_LITE

ret
reload_chrctrl_location2_injection ENDP

_TEXT    ENDS

END
