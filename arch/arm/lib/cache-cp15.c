/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/system.h>
#include <asm/cache-cp15.h>

#if !(defined(CONFIG_SYS_NO_ICACHE) && defined(CONFIG_SYS_NO_DCACHE))
static void cp_delay (void)
{
	volatile int i;

	/* copro seems to need some delay between reading and writing */
	for (i = 0; i < 100; i++)
		nop();
}

/* cache_bit must be either CR_I or CR_C */
static void cache_enable(uint32_t cache_bit)
{
	uint32_t reg;

	reg = get_cr();	/* get control reg. */
	cp_delay();
	set_cr(reg | cache_bit);
}

/* cache_bit must be either CR_I or CR_C */
static void cache_disable(uint32_t cache_bit)
{
	uint32_t reg;

	reg = get_cr();
	cp_delay();
	set_cr(reg & ~cache_bit);
}

#endif
#ifndef CONFIG_SYS_NO_ICACHE


void cp15_icache_enable(void)
{
	cache_enable(CR_I);
}
void cp15_icache_disable(void)
{
	cache_disable(CR_I);
}

int cp15_icache_status(void)
{
	return (get_cr() & CR_I) != 0;
}
#endif

#ifndef CONFIG_SYS_NO_DCACHE
void cp15_dcache_enable(void)
{
	cache_enable(CR_C);
}

void cp15_dcache_disable(void)
{
	cache_disable(CR_C);
}

int cp15_dcache_status(void)
{
	return (get_cr() & CR_C) != 0;
}
#endif
