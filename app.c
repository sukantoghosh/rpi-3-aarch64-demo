/*
 * Derived from code:
 *
 * Copyright (C) 2012 Vikram Narayananan <vikram186@gmail.com>
 * (C) Copyright 2012-2016 Stephen Warren
 * Copyright (C) 1996-2000 Russell King
 *
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <stdint.h>

#define BIT(x) (1 << (x))

#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))

#define dmb()		__asm__ __volatile__ ("" : : : "memory")
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define readl(c)	({ uint32_t __v = __arch_getl(c); __iormb(); __v; })
#define writel(v,c)	({ uint32_t __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define readb(c)	({ uint8_t __v = __arch_getb(c); __iormb(); __v; })
#define writeb(v,c)	({ uint8_t __v = v; __iowmb(); __arch_putb(__v,c); __v; })

#if 0
#define BCM2835_MU_BASE			0x3f215040

struct bcm283x_mu_regs {
	uint32_t io;
	uint32_t iir;
	uint32_t ier;
	uint32_t lcr;
	uint32_t mcr;
	uint32_t lsr;
	uint32_t msr;
	uint32_t scratch;
	uint32_t cntl;
	uint32_t stat;
	uint32_t baud;
};

/* This actually means not full, but is named not empty in the docs */
#define BCM283X_MU_LSR_TX_EMPTY		BIT(5)
#define BCM283X_MU_LSR_RX_READY		BIT(0)

static void bcm283x_mu_serial_putc(const char data)
{
	struct bcm283x_mu_regs *regs = (struct bcm283x_mu_regs *)BCM2835_MU_BASE;

	/* Wait until there is space in the FIFO */
	while (!(readl(&regs->lsr) & BCM283X_MU_LSR_TX_EMPTY))
		;

	/* Send the character */
	writel(data, &regs->io);
}

#define putc	bcm283x_mu_serial_putc
#else
#define PL01x_BASE		0x3f201000

struct pl01x_regs {
	uint32_t	dr;		/* 0x00 Data register */
	uint32_t	ecr;		/* 0x04 Error clear register (Write) */
	uint32_t	pl010_lcrh;	/* 0x08 Line control register, high byte */
	uint32_t	pl010_lcrm;	/* 0x0C Line control register, middle byte */
	uint32_t	pl010_lcrl;	/* 0x10 Line control register, low byte */
	uint32_t	pl010_cr;	/* 0x14 Control register */
	uint32_t	fr;		/* 0x18 Flag register (Read only) */
#ifdef CONFIG_PL011_SERIAL_RLCR
	uint32_t	pl011_rlcr;	/* 0x1c Receive line control register */
#else
	uint32_t	reserved;
#endif
	uint32_t	ilpr;		/* 0x20 IrDA low-power counter register */
	uint32_t	pl011_ibrd;	/* 0x24 Integer baud rate register */
	uint32_t	pl011_fbrd;	/* 0x28 Fractional baud rate register */
	uint32_t	pl011_lcrh;	/* 0x2C Line control register */
	uint32_t	pl011_cr;	/* 0x30 Control register */
};

#define UART_PL01x_FR_TXFF              0x20

static void bcm283x_pl01x_serial_putc(const char data)
{
	struct pl01x_regs *regs = (struct pl01x_regs *)PL01x_BASE;

	/* Wait until there is space in the FIFO */
	while (readl(&regs->fr) & UART_PL01x_FR_TXFF)
		;

	/* Send the character */
	writel(data, &regs->dr);
}

#define putc	bcm283x_pl01x_serial_putc
#endif

void dbg_puts(const char *s)
{
	while (*s)
		putc(*s++);
}

void dbg_puthex4(int val)
{
	int c;

	if (val < 10)
		c = val + '0';
	else
		c = val - 10 + 'A';

	putc(c);
}

void dbg_puthex32(uint32_t val)
{
	for (int i = 28; i >= 0; i -= 4)
		dbg_puthex4((val >> i) & 0xf);
}

void dbg_puthex64(uint64_t val)
{
	dbg_puthex32(val >> 32);
	dbg_puthex32(val & 0xffffffffU);
}

uint64_t read_mpidr(void)
{
	uint64_t v;
	__asm__ __volatile__("mrs %0, mpidr_el1" : "=r" (v) : : );
	return v;
}

uint64_t read_currentel(void)
{
	uint64_t v;
	__asm__ __volatile__("mrs %0, currentel" : "=r" (v) : : );
	return v;
}

uint64_t read_spsel(void)
{
	uint64_t v;
	__asm__ __volatile__("mrs %0, spsel" : "=r" (v) : : );
	return v;
}

void app(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
	static int this_cpuid = -1;
	uint64_t v;

	this_cpuid++;

	/* LF after line noise */
	if (!this_cpuid)
		dbg_puts("\r\n");

	dbg_puts("Hello, world!\r\n");

	dbg_puts("this_cpuid:");
	dbg_puthex4(this_cpuid);
	dbg_puts("\r\n");

	dbg_puts("MPIDR:");
	v = read_mpidr();
	dbg_puthex64(v);
	dbg_puts("\r\n");

	dbg_puts("r0:");
	dbg_puthex64(r0);
	dbg_puts("\r\n");

	dbg_puts("r1:");
	dbg_puthex64(r1);
	dbg_puts("\r\n");

	dbg_puts("r2:");
	dbg_puthex64(r2);
	dbg_puts("\r\n");

	dbg_puts("r3:");
	dbg_puthex64(r3);
	dbg_puts("\r\n");

	dbg_puts("CurrentEL:");
	v = read_currentel();
	dbg_puthex4(v);
	dbg_puts("\r\n");

	dbg_puts("SPSel:");
	v = read_spsel();
	dbg_puthex4(v);
	dbg_puts("\r\n");

	if (this_cpuid < 3) {
		unsigned long long *spin_table = (void *)0xd8;
		spin_table[this_cpuid + 1] = 0x8000;
		__asm__ __volatile__("sev");
	}

	while (1) {
	}
}
