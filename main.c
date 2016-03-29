/*
 * Derived from code:
 *
 * Copyright (C) 2012 Vikram Narayananan <vikram186@gmail.com>
 * (C) Copyright 2012-2016 Stephen Warren
 * Copyright (C) 1996-2000 Russell King
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

typedef unsigned int u32;

#define BIT(x) (1 << (x))

#define BCM2835_MU_BASE			0x3f215040

struct bcm283x_mu_regs {
	u32 io;
	u32 iir;
	u32 ier;
	u32 lcr;
	u32 mcr;
	u32 lsr;
	u32 msr;
	u32 scratch;
	u32 cntl;
	u32 stat;
	u32 baud;
};

/* This actually means not full, but is named not empty in the docs */
#define BCM283X_MU_LSR_TX_EMPTY		BIT(5)
#define BCM283X_MU_LSR_RX_READY		BIT(0)

#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))

#define dmb()		__asm__ __volatile__ ("" : : : "memory")
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define readl(c)	({ u32 __v = __arch_getl(c); __iormb(); __v; })
#define writel(v,c)	({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })

static void bcm283x_mu_serial_putc(const char data)
{
	struct bcm283x_mu_regs *regs = (struct bcm283x_mu_regs *)BCM2835_MU_BASE;

	/* Wait until there is space in the FIFO */
	while (!(readl(&regs->lsr) & BCM283X_MU_LSR_TX_EMPTY))
		;

	/* Send the character */
	writel(data, &regs->io);
}

void dbg_puts(const char *s)
{
	while (*s)
		bcm283x_mu_serial_putc(*s++);
}

void main(void)
{
	while (1)
		dbg_puts("Hello, world!\r\n");
}
