format PE console
include '\win32axp.inc'
include '\encoding\win1251.inc'
.data
ns   dd ?
hout dd ?
hin  dd ?
buffer db 100h dup ?
begin:
invoke GetStdHandle,STD_OUTPUT_HANDLE
mov [hout],eax
invoke GetStdHandle,STD_INPUT_HANDLE
mov [hin],eax
invoke ReadConsole,[hin],buffer,100h,ns,NULL
invoke WriteConsole,[hout],buffer,[ns],ns,NULL
invoke ReadConsole,[hin],buffer,100h,ns,NULL
invoke WriteConsole,[hout],buffer,[ns],ns,NULL
invoke ReadConsole,[hin],buffer,100h,ns,NULL
invoke WriteConsole,[hout],buffer,[ns],ns,NULL
invoke Sleep, 10000
invoke ExitProcess,0
.end begin
