_DATA SEGMENT

_DATA ENDS

_TEXT    SEGMENT

FUNC_PROLOGUE macro
    pushfq 
    push    rax
    mov     rax,rsp
    and     rsp,-10h
    sub     rsp,000002B0h
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

FUNC_EPILOGUE_NORAX macro
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
    add     rsp, 8
    popfq 
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

FUNC_EPILOGUE_LITE_NORAX macro
	mov		r15, [rsp + 20h]
	mov		r11, [rsp + 28h]
	mov		r10, [rsp + 30h]
	mov		r9, [rsp + 38h]
	mov		r8, [rsp + 40h]
	mov		rdx, [rsp + 48h]
	mov		rcx, [rsp + 50h]
	movaps	xmm5, [rsp + 60h]
	movaps	xmm4, [rsp + 70h]
	movaps	xmm3, [rsp + 80h]
	movaps	xmm2, [rsp + 90h]
	movaps	xmm1, [rsp + 0A0h]
	movaps	xmm0, [rsp + 0B0h]
	mov		rsp, r15
	pop		r15
endm


EXTERN AcceptP2PSessionWithUser_injection_helper: PROC
EXTERN AcceptP2PSessionWithUser_injection_return: qword

PUBLIC AcceptP2PSessionWithUser_injection
AcceptP2PSessionWithUser_injection PROC

FUNC_PROLOGUE_LITE
mov     rcx, qword ptr [rdx] ;steamid64
call    AcceptP2PSessionWithUser_injection_helper
FUNC_EPILOGUE_LITE_NORAX

;if we should decline this connection, abort function
cmp     al, 0
je      abort

;original code
PUSH    RDI
SUB     RSP, 40h
mov     qword ptr [rsp+20h], -2
jmp     AcceptP2PSessionWithUser_injection_return
abort:
ret
AcceptP2PSessionWithUser_injection ENDP


EXTERN IsP2PPacketAvailable_Replacement_injection_helper: PROC

EXTERN IsP2PPacketAvailable_1_Replacement_injection_return: qword
PUBLIC IsP2PPacketAvailable_1_Replacement_injection
IsP2PPacketAvailable_1_Replacement_injection PROC

FUNC_PROLOGUE
lea     rcx, [RAX+20h + 10h] ;pcubMsgSize
mov     rdx, 0 ;channel 0
call    IsP2PPacketAvailable_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     IsP2PPacketAvailable_1_Replacement_injection_return
IsP2PPacketAvailable_1_Replacement_injection ENDP

EXTERN IsP2PPacketAvailable_2_Replacement_injection_return: qword
PUBLIC IsP2PPacketAvailable_2_Replacement_injection
IsP2PPacketAvailable_2_Replacement_injection PROC

FUNC_PROLOGUE
lea     rcx, [RAX+20h + 10h] ;pcubMsgSize
mov     rdx, 0 ;channel 0
call    IsP2PPacketAvailable_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     IsP2PPacketAvailable_2_Replacement_injection_return
IsP2PPacketAvailable_2_Replacement_injection ENDP


EXTERN ReadP2PPacket_Replacement_injection_helper: PROC
EXTERN ReadP2PPacket_Replacement_injection_return: qword

PUBLIC ReadP2PPacket_Replacement_injection
ReadP2PPacket_Replacement_injection PROC

FUNC_PROLOGUE
mov     qword ptr [rsp + 20h], 0 ;nChannel = 0
mov     rcx, r15 ;pubDest
mov     edx, ebx ;cubDest
lea     r8, [rbp-38h] ;pcubMsgSize (DSR passes null)
lea     r9, [rbp-40h] ;psteamIDRemote (DSR passes null)
call    ReadP2PPacket_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     ReadP2PPacket_Replacement_injection_return
ReadP2PPacket_Replacement_injection ENDP


EXTERN SendP2PPacket_Replacement_injection_helper: PROC

EXTERN SendP2PPacket_voice_Replacement_injection_return: qword
PUBLIC SendP2PPacket_voice_Replacement_injection
SendP2PPacket_voice_Replacement_injection PROC

FUNC_PROLOGUE
mov     qword ptr [rsp + 20h], 0 ;nChannel = 0
mov     rcx, [rbx] ;steamIDRemote
lea     rdx, [RAX+70h + 10h] ;pubData
mov     r8d, [RAX+50h + 10h] ;cubData
inc     r8d ;i dunno, DSR does it
mov     r9d, 1 ;eP2PSendType::k_EP2PSendUnreliableNoDelay (voice specific)
call    SendP2PPacket_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     SendP2PPacket_voice_Replacement_injection_return
SendP2PPacket_voice_Replacement_injection ENDP

EXTERN SendP2PPacket_Replacement_injection_return: qword
PUBLIC SendP2PPacket_Replacement_injection
SendP2PPacket_Replacement_injection PROC

FUNC_PROLOGUE
mov     qword ptr [rsp + 20h], 0 ;nChannel = 0
mov     rcx, [rbp+0C8h] ;steamIDRemote
mov     rdx, r14 ;pubData
mov     r8d, r15d ;cubData
mov     r9d, [RAX+40h + 10h] ;eP2PSendType
call    SendP2PPacket_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     SendP2PPacket_Replacement_injection_return
SendP2PPacket_Replacement_injection ENDP


EXTERN CloseP2PSessionWithUser_Replacement_injection_helper: PROC
EXTERN CloseP2PSessionWithUser_Replacement_injection_return: qword

PUBLIC CloseP2PSessionWithUser_Replacement_injection
CloseP2PSessionWithUser_Replacement_injection PROC

FUNC_PROLOGUE
mov     rcx, [rbx+0C8h]
call    CloseP2PSessionWithUser_Replacement_injection_helper
FUNC_EPILOGUE_NORAX

jmp     CloseP2PSessionWithUser_Replacement_injection_return
CloseP2PSessionWithUser_Replacement_injection ENDP


EXTERN Start_SessionDisconnect_Task_injection_helper: PROC
EXTERN Start_SessionDisconnect_Task_injection_return: qword

PUBLIC Start_SessionDisconnect_Task_injection
Start_SessionDisconnect_Task_injection PROC

FUNC_PROLOGUE_LITE
mov     rcx, rdx
call    Start_SessionDisconnect_Task_injection_helper
FUNC_EPILOGUE_LITE

;original code
PUSH    RDI
SUB     RSP, 40h
mov     qword ptr [rsp+20h], -2
jmp     Start_SessionDisconnect_Task_injection_return
Start_SessionDisconnect_Task_injection ENDP


EXTERN Steam_LeaveLobby_injection_helper: PROC
EXTERN Steam_LeaveLobby_injection_return: qword

PUBLIC Steam_LeaveLobby_injection
Steam_LeaveLobby_injection PROC

FUNC_PROLOGUE_LITE
mov     rcx, qword ptr [rcx + 3f0h]
call    Steam_LeaveLobby_injection_helper
FUNC_EPILOGUE_LITE

;original code
PUSH    RDI
SUB     RSP, 30h
MOV     qword ptr [RSP + 20h], -2
jmp     Steam_LeaveLobby_injection_return
Steam_LeaveLobby_injection ENDP


EXTERN Steam_CreateLobby_injection_helper: PROC
EXTERN Steam_CreateLobby_injection_return: qword

PUBLIC Steam_CreateLobby_injection
Steam_CreateLobby_injection PROC

FUNC_PROLOGUE_LITE
call    Steam_CreateLobby_injection_helper
FUNC_EPILOGUE_LITE

;don't keep original code
;just return true, since we are replacing this function all together
mov     al, 1
ret
Steam_CreateLobby_injection ENDP

_TEXT    ENDS

END
