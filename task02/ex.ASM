;Вариант 1
;Разработать программу, которая вводит одномерный массив A[N], формирует из элементов массива A
;новый массив B по правилам, указанным в таблице, и выводит его. Память под массивы может
;выделяться как статически, так и динамически по выбору разработчика.
;Разбить решение задачи на функции следующим образом:
;1. Ввод и вывод массивов оформит как подпрограммы.
;2. Выполнение задания по варианту оформить как процедуру
;3. Организовать вывод как исходного, так и сформированного массивов
;Массив B из положительных элементов A.

format PE console
entry start

include 'win32a.inc'

;--------------------------------------------------------------------------
section '.data' data readable writable

        strVecSize   db 'size of vector? ', 0
        strIncorSize db 'Incorrect size of vector', 10, 0
        strVecElemI  db '[%d]? ', 0
        strScanInt   db '%d', 0
        strVecElemOut  db '[%d] = %d', 10, 0
        strOldArr      db 'old array:', 10, 0
        strNewArr      db 'new array:', 10, 0

        vec_size     dd 0
        secVec_size  dd 0
        i            dd ?
        tmp          dd ?
        tmpStack     dd ?
        vec          rd 100
        secVec       rd 100

;--------------------------------------------------------------------------
section '.code' code readable executable
start:
; 1) vector input
        call VectorInput
; 2) get new vector
        call NewVector
; 3) test vector out
        mov ebx, vec
        push strOldArr
        call [printf]
        call VectorOut
        ;print new vector
        mov eax, [secVec_size]
        mov [vec_size], eax
        mov ebx, secVec
        push strNewArr
        call [printf]
        call VectorOut

finish:
        call [getch]

        push 0
        call [ExitProcess]

;--------------------------------------------------------------------------
VectorInput:
        push strVecSize
        call [printf]
        add esp, 4

        push vec_size
        push strScanInt
        call [scanf]
        add esp, 8

        mov eax, [vec_size]
        cmp eax, 0
        jg  getVector
; fail size
        push vec_size
        push strIncorSize
        call [printf]
        call [getch]
        push 0
        call [ExitProcess]
; else continue...
getVector:
        xor ecx, ecx            ; ecx = 0
        mov ebx, vec            ; ebx = &vec
getVecLoop:
        mov [tmp], ebx
        cmp ecx, [vec_size]
        jge endInputVector       ; to end of loop

        ; input element
        mov [i], ecx
        push ecx
        push strVecElemI
        call [printf]
        add esp, 8

        push ebx
        push strScanInt
        call [scanf]
        add esp, 8

        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp getVecLoop
endInputVector:
        ret
;--------------------------------------------------------------------------
NewVector:
        xor ecx, ecx            ; ecx = 0
        mov ebx, vec            ; ebx = &vec
        mov edx, secVec         ; edc = &SecVec
sumVecLoop:
        cmp ecx, [vec_size]
        je endSumVector      ; to end of loop

        mov eax, [ebx]
        cmp eax, 0
        jle contLoop            ; continue loop
        mov eax, [ebx]          ; eax = vec[i]
        mov [edx], eax          ; edx[j] = eax
        add edx, 4              ; inc j
        mov eax, [secVec_size]  ; eax = secVec_size
        add eax, 1
        mov [secVec_size], eax
contLoop:
        inc ecx
        add ebx, 4
        jmp sumVecLoop
endSumVector:
        ret
;--------------------------------------------------------------------------
VectorOut:
        mov [tmpStack], esp
        xor ecx, ecx            ; ecx = 0
putVecLoop:
        mov [tmp], ebx
        cmp ecx, [vec_size]
        je endOutputVector      ; to end of loop
        mov [i], ecx

        ; output element
        push dword [ebx]
        push ecx
        push strVecElemOut
        call [printf]

        mov ecx, [i]
        inc ecx
        mov ebx, [tmp]
        add ebx, 4
        jmp putVecLoop
endOutputVector:
        mov esp, [tmpStack]
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