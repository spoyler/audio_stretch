;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; LPC313x vector table
;;
;;
;; Copyright 2009 IAR Systems. All rights reserved.
;;
;; $Revision: 30870 $
;;


        MODULE  ?vector
        
; This module provides low level interrupt vector table.
; This table shuold be copyed or linked at virtula address 0x0
;
        SECTION .text:CODE:NOROOT(2)
     
        PUBLIC  lpc313x_reset_vector
        PUBLIC  vec_fiq_handler
        PUBLIC  vec_undefined_handler
        PUBLIC  vec_prefetch_handler
        PUBLIC  vec_abort_handler
        PUBLIC  __vector
        PUBLIC  __vector_end
        EXTERN  __iar_program_start
        EXTERN g_irq_func_ptrs

        ARM ; 
lpc313x_reset_vector	
__vector
__reset
  ldr pc,[pc,#24] ; Absolute jump can reach 4 GByte
__undef_handler	
vec_undefined_handler
  ldr pc,[pc,#24] ; Branch to undef_handler
__swi_handler	
vec_swi_handler
  ldr pc,[pc,#24] ; Branch to swi_handler
__prefetch_handler
vec_prefetch_handler
  ldr pc,[pc,#24] ; Branch to prefetch_handler
__data_handler	
vec_abort_handler
  ldr pc,[pc,#24] ; Branch to data_handler
  dc32  0         ; RESERVED
__irq_handler
  ldr pc,[pc,#20] ; Branch to irq_handler
__fiq_handler	
  ldr pc,[pc,#20] ; Branch to fiq_handler
  ; Constant table entries (for ldr pc)
  dc32  __iar_program_start
  dc32 __iar_program_start ;__undef_handler
  dc32 __iar_program_start ;__swi_handler
  dc32 __iar_program_start ;__prefetch_handler
  dc32 __iar_program_start ;__data_handler
  dc32 lpc313x_irq_handler
__vector_end
vec_fiq_handler
  dc32 __fiq_handler

lpc313x_irq_handler
    SUB lr, lr, #4                 ; Get return address 
    STMFD sp!, {r0-r12, lr}        ; Save registers 

    ; Read the IRQ vector status registers 
    LDR    r2, =INTC_BASE_ADDR
    LDR    r3, [r2, #IRQ_VEC_OFF]
    
    ; Assuming TABLE_ADDR in Interrupt vector_0 is set 0 during init. 
    ; If not then add masking instruction below. uncomment the following.
	; AND r3, r3, #0x7FC
	
    LDR    r0, =g_irq_func_ptrs    ; Get address of jump table 
    LDR    r0, [r0, r3, LSR #1]  ; Get handler address. Add by interrupt offset 
    CMP    r0, #0                ; Is handler address NULL? 
    BEQ    int_exit              ; If null, the exit 
    MOV    lr, pc                ; Will return to int_exit 
    BX     r0                    ; Jump to handler 

int_exit
    LDMFD  sp!, {r0-r12, pc}^    ; Restore registers and exit 


INTC_BASE_ADDR  EQU 0x60000000 ; Base address of interrupt controller
IRQ_VEC_OFF     EQU 0x100      ; Offset to IRQ vectored status  
FIQ_VEC_OFF     EQU 0x104      ; Offset to FIQ vectored status  

        END
  
  
