
/* Example program which may be executed using allwindisk.
 * To execute on arm, invoke:
 *      allwindisk mx 0 asmhello.bin
 */

.globl _start
_start:
    push {lr}
    adrl r0, .hello
    ldr r2, =0x4A000004     /* printf() address */
    blx r2
    pop {pc}
.hello:
    .ascii  "Hello from asm !!!\n\0"
