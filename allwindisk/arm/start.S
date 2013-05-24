/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

.globl _start
_start:
    b init
    b send_resp_msg  /* 0x4A000004, printf handler for "allwindisk mx" */
init:
    movw sp, 0x8000
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr,r0

    /* clear bss */
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0

clbss_l:
    str	r2, [r0]
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l
	bl	main_loop
.globl reset_board
reset_board:
    ldr r1, =0x01C20C94
    mov r3, #0x3
    str r3, [r1]
1:  b 1b
