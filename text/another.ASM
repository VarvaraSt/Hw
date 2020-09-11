format PE GUI
include 'win32ax.inc'
; import, code, data in the same section
 xor ebx,ebx
 mov edi,wTitle
 mov esi,400000h
 ; +------------------------------+
 ; | registering the window class |
 ; +------------------------------+
 invoke RegisterClass,esp,CS_HREDRAW or CS_VREDRAW,WndProc,ebx,\
 ebx,esi,ebx,10011h,COLOR_WINDOW+1,ebx,edi
 ; +--------------------------+
 ; | creating the main window |
 ; +--------------------------+
 push ebx esi ebx ebx
 shl esi,9
 invoke CreateWindowEx,ebx,edi,edi,WS_OVERLAPPEDWINDOW+WS_VISIBLE,\
 esi,esi,esi,esi
 mov ebp,esp
 ; +---------------------------+
 ; | entering the message loop |
 ; +---------------------------+
message_loop: invoke GetMessage,ebp,ebx,ebx,ebx
 invoke DispatchMessage,ebp
 jmp message_loop
 ; +----------------------+
 ; | the window procedure |
 ; +----------------------+
proc WndProc hWnd, uMsg, wParam, lParam
local ps:PAINTSTRUCT
local expRect:RECT
mov eax,[uMsg]
mov edi,[hWnd]
dec eax; cmp uMsg,WM_DESTROY
dec eax
je wmDESTROY
sub eax,WM_PAINT-WM_DESTROY; cmp uMsg,WM_PAINT
je   wmPAINT
leave
jmp dword [DefWindowProc]
wmPAINT: invoke BeginPaint,edi,esp
lea esi,[expRect]
invoke GetClientRect,edi,esi,eax
invoke DrawText,dword[esp+16],expTxt,-1,esi,DT_SINGLELINE or DT_CENTER or DT_VCENTER
invoke EndPaint,edi,esp
ret
wmDESTROY: invoke ExitProcess,eax
endp
      wTitle db   'Iczelion Tutorial #4:Painting with Text',0 ;name of our window
      expTxt db   'I tried',0
data import
 library KERNEL32, 'KERNEL32.DLL',\
user32,   'USER32.DLL'
 import KERNEL32,\
      ExitProcess,   'ExitProcess'
 import user32,\
RegisterClass,    'RegisterClassA',\
CreateWindowEx,     'CreateWindowExA',\
DefWindowProc,    'DefWindowProcA',\
GetMessage,    'GetMessageA',\
BeginPaint,    'BeginPaint',\
GetClientRect,    'GetClientRect',\
DrawText,    'DrawTextA',\
EndPaint,    'EndPaint',\
DispatchMessage,    'DispatchMessageA'
end data