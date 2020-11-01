format PE console
entry start

include 'win32a.inc'

;--------------------------------------------------------------------------
section '.data' data readable writable

        strDWord       db 'double word? ', 0
        strScanInt     db '%d', 0
        strInt         db '%d', 10, 0
        strOutput      db 'Number of bit sequences 110:', 10, 0

        double_word  dd 0
        tmp_word     dd 0

;--------------------------------------------------------------------------
section '.code' code readable executable
start:
; 1) input
        call Input
; 2) get number of sequences
        call NumberOf110
; 3) output
        push edx         ;edx = number of '110'
        push strOutput
        call [printf]
        add esp,4
        pop edx

        push edx
        push strInt
        call [printf]
        add esp,8

finish:
        call [getch]

        push 0
        call [ExitProcess]

;--------------------------------------------------------------------------
Input:
        push strDWord
        call [printf]
        add esp, 4

        push double_word
        push strScanInt
        call [scanf]
        add esp, 8
        ret
;--------------------------------------------------------------------------
NumberOf110:
        mov ecx, 30                ;ecx = number of bits in dd - length of sequence + 1 = 32 - 3 + 1
        xor edx, edx               ;edx = 0
        mov eax, [double_word]
        mov [tmp_word], eax        ;tmp_word = double_word
bitLoop:
        mov ebx, [tmp_word]        ;ebx = tmp_word
        or ebx, 6                  ;set 1 in the second and third bits from the end (6 = 000..0110)
        and ebx, -2                ;set 0 in the last bit (-2 = 1111...110)
        xor ebx, [tmp_word]        ;ebx ^ tmp_word (tmp_word = ......, ebx = ...110)
        mov eax, [tmp_word]
        shr eax, 1
        mov [tmp_word], eax        ; tmp_word >> 1
        cmp ebx, 0                 ; ebx == 0 if tmp_word = ...110
        jne continLoop             ; ebx != 0
        inc edx                    ; edx++
continLoop:
        loop bitLoop
endNumberOf110:
        ret
;-------------------------------third act - including HeapApi--------------------------
                                                 
section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll',\
            user32,'USER32.DLL'

include 'api\user32.inc'
include 'api\kernel32.inc'
    import kernel,\
           ExitProcess, 'ExitProcess',\
           HeapCreate,'HeapCreate',\
           HeapAlloc,'HeapAlloc'
  include 'api\kernel32.inc'
    import msvcrt,\
           printf, 'printf',\
           scanf, 'scanf',\
           getch, '_getch'