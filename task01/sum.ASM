format PE console
entry main

include 'macro/import32.inc'
INCLUDE 'win32ax.inc'
INCLUDE '\encoding\win1251.inc'
INCLUDE '\api\user32.inc'
INCLUDE '\api\kernel32.inc'

section '.data' data readable writeable
msg db "Another string",0
p db "pause>nul",0
formats db '%d',0
result db 256 dup(?)

section '.code' code readable executable
main:
push ebp
mov ebp,esp
sub ebp,4
mov eax,10 ;?????????? ? eax ????? 1
mov edx,115 ;?????????? ? edx ????? 2
add eax,edx ;?????????? ?????????? eax ? edx (1+2). ??????? ????????? ? eax.
invoke wsprintf,result,formats,eax
mov dword [esp],result
call [printf]
mov dword [esp],p
call [system]
mov dword [esp],0
call [exit]
invoke ExitProcess,0

section '.idata' import data readable
library msvcrt,'msvcrt.dll', kernel32,'KERNEL32.DLL',user32,'USER32.DLL'
import msvcrt,\
printf,'printf',\
system,'system',\
exit,'exit'