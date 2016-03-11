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

#define BCM2835_AUX_BASE		0x3f215000

#define BCM2835_AUX_ENABLES_SPI2	BIT(0)
#define BCM2835_AUX_ENABLES_SPI1	BIT(0)
#define BCM2835_AUX_ENABLES_MINI_UART	BIT(0)

struct bcm2835_aux_regs {
	u32 irq;
	u32 enables;
};

#define BCM2835_GPIO_BASE		0x3f200000
#define BCM2835_GPIO_COUNT		54

#define BCM2835_GPIO_FSEL_MASK		0x7
#define BCM2835_GPIO_INPUT		0x0
#define BCM2835_GPIO_OUTPUT		0x1
#define BCM2835_GPIO_ALT0		0x4
#define BCM2835_GPIO_ALT1		0x5
#define BCM2835_GPIO_ALT2		0x6
#define BCM2835_GPIO_ALT3		0x7
#define BCM2835_GPIO_ALT4		0x3
#define BCM2835_GPIO_ALT5		0x2

#define BCM2835_GPIO_COMMON_BANK(gpio)	((gpio < 32) ? 0 : 1)
#define BCM2835_GPIO_COMMON_SHIFT(gpio)	(gpio & 0x1f)

#define BCM2835_GPIO_FSEL_BANK(gpio)	(gpio / 10)
#define BCM2835_GPIO_FSEL_SHIFT(gpio)	((gpio % 10) * 3)

struct bcm2835_gpio_regs {
	u32 gpfsel[6];
	u32 reserved1;
	u32 gpset[2];
	u32 reserved2;
	u32 gpclr[2];
	u32 reserved3;
	u32 gplev[2];
	u32 reserved4;
	u32 gpeds[2];
	u32 reserved5;
	u32 gpren[2];
	u32 reserved6;
	u32 gpfen[2];
	u32 reserved7;
	u32 gphen[2];
	u32 reserved8;
	u32 gplen[2];
	u32 reserved9;
	u32 gparen[2];
	u32 reserved10;
	u32 gppud;
	u32 gppudclk[2];
};

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

#define BCM283X_MU_LCR_DATA_SIZE_8	3

#define BCM283X_MU_LSR_TX_IDLE		BIT(6)
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

#if 0
static void toggle_led(void)
{
	struct bcm2835_gpio_regs *gpior = (struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	int gpio = 47;
	int bank = BCM2835_GPIO_COMMON_BANK(gpio);
	int shift = BCM2835_GPIO_COMMON_SHIFT(gpio);
	static int val;

	val = 1 - val;
	if (val)
		writel(BIT(shift), &gpior->gpset[bank]);
	else
		writel(BIT(shift), &gpior->gpclr[bank]);
}
#endif

static void bcm283x_mu_serial_setbrg(int baudrate)
{
	struct bcm283x_mu_regs *regs = (struct bcm283x_mu_regs *)BCM2835_MU_BASE;
	/* FIXME: Get this from plat data later */
	u32 clock_rate = 250000000;
	u32 divider;

	divider = clock_rate / (baudrate * 8);

	writel(BCM283X_MU_LCR_DATA_SIZE_8, &regs->lcr);
	writel(divider - 1, &regs->baud);
}

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
	struct bcm2835_gpio_regs *gpior = (struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	struct bcm2835_aux_regs *auxr = (struct bcm2835_aux_regs *)BCM2835_AUX_BASE;
	u32 val;

	/* Route mini UART to header UART pins */
	val = readl(&gpior->gpfsel[BCM2835_GPIO_FSEL_BANK(14)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(14));
	val |= (BCM2835_GPIO_ALT5 << BCM2835_GPIO_FSEL_SHIFT(14));
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(15));
	val |= (BCM2835_GPIO_ALT5 << BCM2835_GPIO_FSEL_SHIFT(15));
	writel(val, &gpior->gpfsel[BCM2835_GPIO_FSEL_BANK(14)]);

	/* Enable mini UART HW module */
	val = readl(&auxr->enables);
	val |= BCM2835_AUX_ENABLES_MINI_UART;
	writel(val, &auxr->enables);

	bcm283x_mu_serial_setbrg(115200);
	while (1)
		dbg_puts("Hello, world!\r\n");
}
