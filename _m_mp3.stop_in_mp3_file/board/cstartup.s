;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Part one of the system initialization code,
;; contains low-level
;; initialization.
;;
;; Copyright 2006 IAR Systems. All rights reserved.
;;
;; $Revision: 30870 $
;;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION IRQ_STACK:DATA:NOROOT(3)
        SECTION FIQ_STACK:DATA:NOROOT(3)
        SECTION SVC_STACK:DATA:NOROOT(3)
        SECTION ABT_STACK:DATA:NOROOT(3)
        SECTION UND_STACK:DATA:NOROOT(3)
        SECTION CSTACK:DATA:NOROOT(3)

;
; The module in this file are included in the libraries, and may be
; replaced by any user-defined modules that define the PUBLIC symbol
; __iar_program_start or a user defined start symbol.
;
; To override the cstartup defined in the library, simply add your
; modified version to the workbench project.

        SECTION .intvec:CODE:ROOT(2)

        PUBLIC  __vector
        PUBLIC  lpc32xx_reset_vector
        PUBLIC  __iar_program_start
        PUBLIC  lpc32xx_irq_handler, _tci_loop
        PUBLIC  vec_reset_handler, vec_undefined_handler, vec_swi_handler, vec_prefetch_handler, vec_abort_handler,  
        PUBLIC  vec_irq_handler, vec_fiq_handler


        EXTERN	undef_handler, swi_handler, prefetch_handler
        EXTERN	data_handler, irq_handler, fiq_handler, irq_func_ptrs
                      
        ARM	; Always ARM mode after reset	
__vector:
lpc32xx_reset_vector:
vec_reset_handler:
		ldr	pc,[pc,#24]	; Absolute jump can reach 4 GByte
;		ldr	b,?cstartup	; Relative branch allows remap, limited to 32 MByte
__undef_handler:
vec_undefined_handler:
                ldr	pc,[pc,#24]	; Branch to undef_handler
__swi_handler:
vec_swi_handler:
		ldr	pc,[pc,#24]	; Branch to swi_handler
__prefetch_handler:
vec_prefetch_handler:
		ldr	pc,[pc,#24]	; Branch to prefetch_handler
__data_handler:
vec_abort_handler:
		ldr	pc,[pc,#24]	; Branch to data_handler
		nop
                  
__irq_handler:
vec_irq_handler:
		ldr	pc,[pc,#20]	; Branch to irq_handler
__fiq_handler:
vec_fiq_handler:
		ldr	pc,[pc,#20]	; Branch to fiq_handler

		; Constant table entries (for ldr pc) will be placed at 0x20
      dc32	__iar_program_start
      dc32	__undef_handler
      dc32	__swi_handler
      dc32	__prefetch_handler
      dc32	__data_handler
      dc32	lpc32xx_irq_handler
      dc32	lpc32xx_fiq_handler


;        PUBLIC  ?cstartup
        EXTERN  ?main
        EXTERN  bootloader
        REQUIRE __vector
#ifdef BOOT_LEVEL_2
        SECTION .bootloader:CODE:NOROOT(2)
#else
        SECTION .text:CODE:NOROOT(2)
#endif
        ARM

__iar_program_start:
?cstartup:

MIC_BASE_ADDR DEFINE 0x40008000    /* Base address of MIC */
SIC1_BASE_ADDR DEFINE 0x4000C000   /* Base address of SIC1 */
SIC2_BASE_ADDR DEFINE 0x40010000   /* Base address of SIC2 */
IRQ_STATUS_OFF DEFINE 0x08         /* Masked IRQ status offset */


;
; Add initialization needed before setup of stackpointers here.
;
CP_DIS_MASK DEFINE  0xFFFFEFFA
// Disable Addr translation, D cache and enable I cache
                mrc     p15,0,R1,C1,C0,0
                ldr     R0,=CP_DIS_MASK     ; 0xFFFFEFFA
                and     R1,R1,R0
                orr     R1,R1,#(1<<12)
                //mcr     p15,0,R1,C1,C0,0
; Disable WDT
                ldr     r0,=0x400040BC      ; TIMCLK_CTRL
                ldr     r1,[r0]
                bic     r1,r1,#1            ; clear WDT_CLK_ENA
                str     r1,[r0]

#ifdef BOOT_LEVEL_2
        mrs         r0,cpsr                             ; Original PSR value
        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#SVC_MODE                     ; Set Supervisor mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=0x08000FF8                      ; End of bootloader stack ()

        LDR     r0, =bootloader
        blx     r0
#endif

;
; Initialize the stack pointers.
; The pattern below can be used for any of the exception stacks:
; FIQ, IRQ, SVC, ABT, UND, SYS.
; The USR mode uses the same stack as SYS.
; The stack segments must be defined in the linker command file,
; and be declared above.
;
; --------------------
; Mode, correspords to bits 0-5 in CPSR

MODE_MSK DEFINE 0x1F            ; Bit mask for mode bits in CPSR
USR_MODE DEFINE 0x10            ; User mode
FIQ_MODE DEFINE 0x11            ; Fast Interrupt Request mode
IRQ_MODE DEFINE 0x12            ; Interrupt Request mode
SVC_MODE DEFINE 0x13            ; Supervisor mode
ABT_MODE DEFINE 0x17            ; Abort mode
UND_MODE DEFINE 0x1B            ; Undefined Instruction mode
SYS_MODE DEFINE 0x1F            ; System mode

        mrs         r0,cpsr                             ; Original PSR value
        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#SVC_MODE                     ; Set Supervisor mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(SVC_STACK)                  ; End of SVC_STACK

        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#UND_MODE                     ; Set Undefined mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(UND_STACK)                  ; End of UND_MODE

        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#ABT_MODE                     ; Set Data abort mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(ABT_STACK)                  ; End of ABT_STACK

        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#FIQ_MODE                     ; Set FIR mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(FIQ_STACK)                  ; End of FIQ_STACK

        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#IRQ_MODE                     ; Set IRQ mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(IRQ_STACK)                  ; End of IRQ_STACK

        bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
        orr         r0,r0,#SYS_MODE                     ; Set System mode bits
        msr         cpsr_c,r0                           ; Change the mode
        ldr         sp,=SFE(CSTACK)                     ; End of CSTACK

#ifdef __ARMVFP__
        ;; Enable the VFP coprocessor.

        MOV     r0, #0x40000000         ; Set EN bit in VFP
        FMXR    fpexc, r0               ; FPEXC, clear others.

;
; Disable underflow exceptions by setting flush to zero mode.
; For full IEEE 754 underflow compliance this code should be removed
; and the appropriate exception handler installed.
;

        MOV     r0, #0x01000000         ; Set FZ bit in VFP
        FMXR    fpscr, r0               ; FPSCR, clear others.
#endif

; Continue to ?main for C-level initialization.

        LDR     r0, =?main
        BX      r0
        
//процедура чистки Dcash
_tci_loop: 
        MRC p15, 0, r15, c7, c10, 3 ; test, clean, invalid
        BNE _tci_loop
        
        MCR p15, 0, r0, c7, c10, 4 ; flash write buff
        bx r14
        
        
        
/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;
; Function: lpc32xx_irq_handler
;
; Purpose:
;   Handle the IRQ interrupt
; 
; Processing:
;   Loop forever
;
; Parameters: None
;
; Outputs:  None
;
; Returns: Nothing
;
; Notes: None
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
lpc32xx_irq_handler:
    SUB    lr, lr, #4                 /* Get return address */
    STMFD  sp!, {r0-r12, lr}          /* Save registers */

    /* Read the MIC interrupt status registers */
    LDR    r2, =MIC_BASE_ADDR
    LDR    r3, [r2, #IRQ_STATUS_OFF]
    AND    r3, r3, #0xFFFFFFFC /* Mask off subIRQ bits */
    MOV    r4, #0

    /* If there the MIC IRQ status is 0, then there are no MIC
       interrupts pending. That means, go service SIC1 interrupts
       instead. */
service_mic:
    CMP    r3, #0                  
    BNE    int_found 
    /* The interrupt was not from MIC */
service_sic1:
    /* Read the SIC1 interrupt status registers */
    LDR    r2, =SIC1_BASE_ADDR     
    LDR    r3, [r2, #IRQ_STATUS_OFF]
    MOV    r4, #32

    /* If there the SIC1 IRQ status is 0, then there are no SIC1
       interrupts pending. That means, go service SIC2 interrupts
       instead. */
    CMP    r3, #0                  
    BNE    int_found 
    /* The interrupt was not from SIC1 */

service_sic2:
    /* Read the SIC2 interrupt status registers  */
    LDR    r2, =SIC2_BASE_ADDR     
    LDR    r3, [r2, #IRQ_STATUS_OFF]
    MOV    r4, #64
    CMP    r3, #0                  
    BEQ    int_exit 
    /* The interrupt was not from SIC2 */

int_found:
    CLZ    r1, r3
    RSB    r1, r1, #31
    ADD    r1, r1, r4
    LDR    r0, =irq_func_ptrs    /* Get address of jump table */
    ADD    r0, r0, r1, LSL #2    /* Add by interrupt offset */
    LDR    r0, [r0]              /* Get handler address */
    CMP    r0, #0                /* Is handler address NULL? */
    BEQ    int_exit              /* If null, the exit */
    MOV    lr, pc                /* Will return to int_exit */
    BX     r0                    /* Jump to handler */

int_exit:
    LDMFD  sp!, {r0-r12, pc}^    /* Restore registers and exit */

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;
; Function: lpc32xx_fiq_handler
;
; Purpose:
;   Handle the FIQ interrupt
; 
; Processing:
;   Loop forever
;
; Parameters: None
;
; Outputs:  None
;
; Returns: Nothing
;
; Notes: None
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
lpc32xx_fiq_handler:
    B    lpc32xx_fiq_handler
        

    END
