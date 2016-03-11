/*
 * Derived from code:
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * (C) Copyright 2013 David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

/*
 * Branch if current processor is a master,
 * choose processor with all zero affinity value as the master.
 */
.macro	branch_if_master, xreg1, xreg2, master_label
	/* NOTE: MPIDR handling will be erroneous on multi-cluster machines */
	mrs	\xreg1, mpidr_el1
	lsr	\xreg2, \xreg1, #32
	lsl	\xreg1, \xreg1, #40
	lsr	\xreg1, \xreg1, #40
	orr	\xreg1, \xreg1, \xreg2
	cbz	\xreg1, \master_label
.endm

.globl _start
_start:
	branch_if_master x0, x1, master_cpu
	/* Slave CPUs */
slave_cpu:
	//wfe
	b slave_cpu
master_cpu:
	/* On the master CPU */
	ldr x0, =0x00100000
	mov sp, x0
	bl main
hang:
	b hang

.ltorg

.org 0x100
.globl _atags
atags:

.org 0x8000
