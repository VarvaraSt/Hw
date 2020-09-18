format          PE console 4.0
include         '\win32ax.inc'
entry           start
 
;--- секция кода ---
 
section         '.code' code readable executable
 
  start:
        invoke  FreeConsole                             ; Освободить существующую консоль
        invoke  AllocConsole                            ; Назначить новую консоль
        invoke  GetStdHandle, STD_OUTPUT_HANDLE         ; Получить дескриптор стандартного потока вывода
        mov     [stdout], eax                           ; Сохранить его
        invoke  GetStdHandle, STD_INPUT_HANDLE          ; Получить дескриптор стандартного потока ввода
        mov     [stdin], eax                            ; Сохранить его
 
        invoke  GetConsoleScreenBufferInfo,[stdout],lpConsoleScreenBufferInfo
 
        mov     [ConsoleWindow.Left], 0
        mov     [ConsoleWindow.Right], 0
        mov     ax, word [lpConsoleScreenBufferInfo.srWindow.Right]
        shr     ax,1
        mov     word [ConsoleWindow.Right], ax
        mov     ax, word [lpConsoleScreenBufferInfo.srWindow.Bottom]
        shr     ax,0
        mov     word [ConsoleWindow.Bottom], ax
 
        invoke  SetConsoleWindowInfo,[stdout],TRUE,ConsoleWindow
 
        invoke  Sleep, 10000
        invoke  FreeConsole
 
  exit:
        invoke  ExitProcess, 0
 
;--- секция данных ---
 
section         '.data' data readable writeable
 
stdout          dd      ?
stdin           dd      ?
 
struct          COORD
  x             dw      ?
  y             dw      ?
ends
 
struct SMALL_RECT
  Left          dw      ?
  Top           dw      ?
  Right         dw      ?
  Bottom        dw      ?
ends
 
struct CONSOLE_SCREEN_BUFFER_INFO
  dwSize                COORD           ; размеры буфера в строках и столбцах символов
  dwCursorPosition      COORD           ; координаты (позиция) курсора в буфере
  wAttributes           dw      ?       ; атрибуты символов
  srWindow              SMALL_RECT      ; координаты верхнего левого и нижнего правого углов буфера
  dwMaximumWindowSize   COORD           ; максимальные размеры консольного окна
ends
 
lpConsoleScreenBufferInfo       CONSOLE_SCREEN_BUFFER_INFO
ConsoleWindow                   SMALL_RECT
dwSize                          COORD
 
;--- секция импорта ---
 
section         '.idata' import data readable writeable
 
library         kernel32,'KERNEL32.DLL'
 
import          kernel32,\
                AllocConsole,'AllocConsole',\
                GetConsoleScreenBufferInfo,'GetConsoleScreenBufferInfo',\
                SetConsoleWindowInfo,'SetConsoleWindowInfo',\
                FreeConsole,'FreeConsole',\
                GetStdHandle,'GetStdHandle',\
                Sleep,'Sleep',\
                ExitProcess,'ExitProcess'