/*
 * Copyright (c) 2014, Texas Instruments, Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. */

/* This program waits for sysevent 16 from ARM. It sends and interrupt to ARM
   using sysevent 32 which is generated by writing to its own R31 register.
   Sysevent 16 is then cleared by this code. The ARM remoteproc driver is
   responsible for clearing sysevent 32 */

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include "resource_table_1.h"
volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* Mapping Constant table register to variable */
volatile pruCfg CT_CFG __attribute__((cregister("PRU_CFG", near), peripheral));
volatile far pruIntc CT_INTC __attribute__((cregister("PRU_INTC", far), peripheral));
volatile far uint32_t CT_DRAM __attribute__((cregister("PRU_DMEM_0_1", near), peripheral));

/* Defines */
#define PRU1
#define HOST1_MASK		(0x80000000)
#define PRU0_PRU1_EVT	(16)

#define TOGGLE_BLUE (__R30 ^= (1 << 7))
void main(){
	/* Configure GPI and GPO as Mode 0 (Direct Connect) */
	CT_CFG.GPCFG0 = 0x0000;

	uint32_t *dram = (uint32_t *) &CT_DRAM;

	/* Clear GPO pins */
	__R30 &= 0x00000000;
	/* Spin in loop until interrupt on HOST 1 is detected */
	while(1){
		/* Wait for sysevent 16 from ARM which mapped to HOST1 */
		if (__R31 & HOST1_MASK){
			TOGGLE_BLUE;

			/* Add values at first two DRAM addresses and store result in 3rd address */
			dram[2] = dram[0] + dram[1];
			
			/* Interrupt ARM using sys evt 32 */
			__R31 |= 0x00000021;
			
			/* Clear interrupt event */
			CT_INTC.SICR = 16;
			__delay_cycles(5);
		}
	}

	/* Halt the PRU core - shouldn't get here */
	//__halt();
}

