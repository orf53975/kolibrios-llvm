$Revision: 480 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; ����� ������������ �� ���� �����
MouseByteNumber_2 DB 0
; ������������ ��������� ������, ������������ �����
FirstByte_2       DB 0
SecondByte_2      DB 0
ThirdByte_2       DB 0
timer_ticks_ps2   dd 0

;**************************************
;* ���������� ���������� �� ���� PS/2 *
;**************************************
check_mouse_data_ps2:
        cmp     [ps2_mouse_detected],0
        je      @@EndMouseInterrupt_2
        call    Wait8042BufferEmpty ;������� ������
        in      AL,0x60              ;�������� ����-���
; �������� ���������� ����� ������������ �����
        cmp     [MouseByteNumber_2],0
        je      @@SaveFirstByte
        cmp     [MouseByteNumber_2],1
        je      @@SaveSecondByte
        cmp     [MouseByteNumber_2],2
        je      @@SaveThirdByte
        jmp     @@Error_2
; �������� ������ ���� �������
@@SaveFirstByte:
        test    AL,1000b  ;������ ���� �������?
        jz      @@Error_2     ;���� �������������
        mov     [FirstByte_2],AL
        inc     [MouseByteNumber_2]
        jmp     @@EndMouseInterrupt_2
; �������� ������ ���� �������
@@SaveSecondByte:
        mov     [SecondByte_2],AL
        inc     [MouseByteNumber_2]
        jmp     @@EndMouseInterrupt_2
; �������� ������ ���� �������
@@SaveThirdByte:
        mov     [ThirdByte_2],AL
        mov     [MouseByteNumber_2],0
; (����� ������ �� ���� ������ ���������)
; �������� ����� �������� ����� ��������� ������
        mov al,[FirstByte_2]   ;[0xfb01]
        and    eax,3
        mov    [BTN_DOWN],al
        mov    [mouse_active],1
; ��������� ����� X-���������� �������
        ; ������� � AX ����������� �� X
        mov     AH,0   ;��������� ���� �� ��� ������� AH
        mov     AL,[FirstByte_2]
        test    AL,10000b
        jz      @@M0
        mov     AH,0FFh
        ; ������� � AL ������� ����
@@M0:
         mov     AL,[SecondByte_2]
        call     mouse_acceleration_ps2
        ; ��������� ����� �������� ����������
        ; ������� �� X
        add     AX,[MOUSE_X]    ;[XCoordinate]
        jns     @@M1
        xor     eax, eax
        jmp     @@M2
@@M1:
        cmp     AX,[ScreenWidth]   ;ScreenLength
        jl      @@M2
        mov     AX,[ScreenWidth]   ;ScreenLength-1
@@M2:
        mov     [MOUSE_X],AX     ;[XCoordinate]

; ��������� ����� Y-���������� �������
        ; ������� � AX ����������� �� Y
        mov     AH,0   ;��������� ���� �� ��� ������� AH
        mov     AL,[FirstByte_2]
        test    AL,100000b
        jz      @@M3
        mov     AH,0FFh
        ; ������� � AL ������� ����
@@M3:
        mov     AL,[ThirdByte_2]
        call     mouse_acceleration_ps2
        ; ��������� ����� �������� ���������� �������
        ; �� Y (Y-���������� ���� PS/2 ����������
        ; �������������� ��������)
        neg     AX
        add     AX,[MOUSE_Y]   ;[YCoordinate]
        jns     @@M4
        xor     eax, eax
        jmp     @@M5
@@M4:
        cmp     AX,[ScreenHeight]  ;ScreenHeigth
        jl      @@M5
        mov     AX,[ScreenHeight] ;ScreenHeigth-1
@@M5:
        mov     [MOUSE_Y],AX     ;[YCoordinate]

; �������� ������ � ����� �������
        mov   eax,[timer_ticks]
        mov   [timer_ticks_ps2],eax
        jmp     @@EndMouseInterrupt_2

; ��������� ���� � ������� �������� ���������� �� ����
@@Error_2:
        mov   [MouseByteNumber_2],0
; ���������� ���������� ���������
@@EndMouseInterrupt_2:
        call  ready_for_next_irq_1
        ret

mouse_acceleration_ps2:
        push    eax
        mov   eax,[timer_ticks]
        sub   eax,[timer_ticks_ps2]
        cmp   eax,[mouse_delay]
        pop    eax
        ja     @f
        imul  ax,[mouse_speed_factor]
@@:
        ret
;***********************************************
;*   �������� ������� �������� ������ I8042    *
;* ��� ������ �� ���������:                    *
;* ���� ZF ���������� - ���������� ����������, *
;* ���� ZF ������� - ������ ����-����.         *
;***********************************************
Wait8042BufferEmpty:
;        push    CX
;        mov     CX,0FFFFh  ;������ ����� ������ �������
;@@kb:
;        in      AL,64h     ;�������� ������
;        test    AL,10b     ;����� i8042 ��������?
;        loopnz  @@kb       ;���� ���, �� ����
;        pop     CX
        push ecx
        xor ecx,ecx
      @@:
        in     al,64h
        test    al,00000010b
        loopnz @b
        pop    ecx
        ;���� ��� ������ �� ������������ �������
        ;���� ZF - ������
        ret                   ;������� � ������������

;***************************************
;* �������� ����������� ������ �� ���� *
;***************************************
WaitMouseData:
;        push    CX
;        mov     CX,0FFFFh  ;������ ����� ������ �������
;@@mouse:
;        in      AL,64h     ;�������� ������� �������
;        test    AL,100000b ;������ ���������?
;        loopz   @@mouse    ;���� ���, �� ����
;        pop     CX
        push ecx
        mov     ECX,0FFFFh
      @@:
        in     al,64h
        test    al,100000b
        loopz @b
        pop    ecx
        ;���� ��� ������ �� ������������ ����������
        ;���� ZF - ������
        ret
