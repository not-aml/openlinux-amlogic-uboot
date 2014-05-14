/*
 * drivers/mtd/nand/nand_util.c
 *
 * Copyright (C) 2006 by Weiss-Electronic GmbH.
 * All rights reserved.
 *
 * @author:	Guido Classen <clagix@gmail.com>
 * @descr:	NAND Flash support
 * @references: borrowed heavily from Linux mtd-utils code:
 *		flash_eraseall.c by Arcom Control System Ltd
 *		nandwrite.c by Steven J. Hill (sjhill@realitydiluted.com)
 *			       and Thomas Gleixner (tglx@linutronix.de)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
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
 *
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <div64.h>

#include <asm/errno.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <jffs2/jffs2.h>

typedef struct erase_info erase_info_t;
typedef struct mtd_info	  mtd_info_t;

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

/*****************************************************************************/
static int nand_block_bad_scrub(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	return 0;
}

/**
 * nand_erase_opts: - erase NAND flash with support for various options
 *		      (jffs2 formating)
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	erase_info_t erase;
	loff_t erase_length;
	
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	int (*nand_block_bad_old)(struct mtd_info *, loff_t, int) = NULL;
	const char *mtd_device = meminfo->name;
	struct mtd_oob_ops oob_opts;
	struct nand_chip *chip = meminfo->priv;

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = opts->length;

	cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen = cpu_to_je32(8);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		struct nand_chip *priv_nand = meminfo->priv;

		nand_block_bad_old = priv_nand->block_bad;
		priv_nand->block_bad = nand_block_bad_scrub;
		/* we don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (priv_nand->bbt) {
			kfree(priv_nand->bbt);
		}
		priv_nand->bbt = NULL;
	}

	if (erase_length < meminfo->erasesize) {
		printf("Warning: Erase size 0x%09qx smaller than one "	\
		       "erase block 0x%08x\n",erase_length, meminfo->erasesize);
		printf("         Erasing 0x%08x instead\n", meminfo->erasesize);
		erase_length = meminfo->erasesize;
	}
    for (;
	     erase.addr < opts->offset + erase_length;
	     erase.addr += meminfo->erasesize) {
        
	
		WATCHDOG_RESET ();

		if (!opts->scrub && bbtest) {
			int ret = meminfo->block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					printf("\rSkipping bad block at  "
					       "0x%08llx                 "
					       "                         \n",
					       erase.addr);
				continue;

			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		result = meminfo->erase(meminfo, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d %llx\n",
			       mtd_device, result,erase.addr);
			continue;
		}

		/* format for JFFS2 ? */
		if (opts->jffs2 && chip->ecc.layout->oobavail >= 8) {
			chip->ops.ooblen = 8;
			chip->ops.datbuf = NULL;
			chip->ops.oobbuf = (uint8_t *)&cleanmarker;
			chip->ops.ooboffs = 0;
			chip->ops.mode = MTD_OOB_AUTO;

			result = meminfo->write_oob(meminfo,
			                            erase.addr,
			                            &chip->ops);
			if (result != 0) {
				printf("\n%s: MTD writeoob failure: %d\n",
				       mtd_device, result);
				continue;
			}
		}

		if (!opts->quiet) {
			unsigned long long n =(unsigned long long)
				(erase.addr + meminfo->erasesize - opts->offset)
				* 100;
			int percent=0;
            n/=erase_length;
//			do_div(n, erase_length);
			percent = (int)n;

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			 
			if (percent != percent_complete) {
				percent_complete = percent;

			    printf("\rErasing at 0x%llx -- %3d%% complete.",
				       erase.addr, percent);
	
				if (opts->jffs2 && result == 0)
					printf(" Cleanmarker written at 0x%llx.",
					       erase.addr);
			}
		}
	}
	if (!opts->quiet)
		printf("\n");

	if (nand_block_bad_old) {
		struct nand_chip *priv_nand = meminfo->priv;

		priv_nand->block_bad = nand_block_bad_old;
		priv_nand->scan_bbt(meminfo);
	}

	return 0;
}

/* XXX U-BOOT XXX */
#if 0

#define MAX_PAGE_SIZE	2048
#define MAX_OOB_SIZE	64

/*
 * buffer array used for writing data
 */
static unsigned char data_buf[MAX_PAGE_SIZE];
static unsigned char oob_buf[MAX_OOB_SIZE];

/* OOB layouts to pass into the kernel as default */
static struct nand_ecclayout none_ecclayout = {
	.useecc = MTD_NANDECC_OFF,
};

static struct nand_ecclayout jffs2_ecclayout = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 0, 1, 2, 3, 6, 7 }
};

static struct nand_ecclayout yaffs_ecclayout = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = { 8, 9, 10, 13, 14, 15}
};

static struct nand_ecclayout autoplace_ecclayout = {
	.useecc = MTD_NANDECC_AUTOPLACE
};
#endif

/* XXX U-BOOT XXX */
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK

/******************************************************************************
 * Support for locking / unlocking operations of some NAND devices
 *****************************************************************************/

#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_LOCK_TIGHT	0x2c
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24
#define NAND_CMD_LOCK_STATUS	0x7a

/**
 * nand_lock: Set all pages of NAND flash chip to the LOCK or LOCK-TIGHT
 *	      state
 *
 * @param mtd		nand mtd instance
 * @param tight		bring device in lock tight mode
 *
 * @return		0 on success, -1 in case of error
 *
 * The lock / lock-tight command only applies to the whole chip. To get some
 * parts of the chip lock and others unlocked use the following sequence:
 *
 * - Lock all pages of the chip using nand_lock(mtd, 0) (or the lockpre pin)
 * - Call nand_unlock() once for each consecutive area to be unlocked
 * - If desired: Bring the chip to the lock-tight state using nand_lock(mtd, 1)
 *
 *   If the device is in lock-tight state software can't change the
 *   current active lock/unlock state of all pages. nand_lock() / nand_unlock()
 *   calls will fail. It is only posible to leave lock-tight state by
 *   an hardware signal (low pulse on _WP pin) or by power down.
 */
int nand_lock(struct mtd_info *mtd, int tight)
{
	int ret = 0;
	int status;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd,
		      (tight ? NAND_CMD_LOCK_TIGHT : NAND_CMD_LOCK),
		      -1, -1);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);

	/* see if device thinks it succeeded */
	if (status & 0x01) {
		ret = -1;
	}

	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_get_lock_status: - query current lock state from one page of NAND
 *			   flash
 *
 * @param mtd		nand mtd instance
 * @param offset	page address to query (muss be page aligned!)
 *
 * @return		-1 in case of error
 *			>0 lock status:
 *			  bitfield with the following combinations:
 *			  NAND_LOCK_STATUS_TIGHT: page in tight state
 *			  NAND_LOCK_STATUS_LOCK:  page locked
 *			  NAND_LOCK_STATUS_UNLOCK: page unlocked
 *
 */
int nand_get_lock_status(struct mtd_info *mtd, loff_t offset)
{
	int ret = 0;
	int chipnr;
	int page;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chipnr = (int)(offset >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);


	if ((offset & (mtd->writesize - 1)) != 0) {
		printf ("nand_get_lock_status: "
			"Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Status */
	page = (int)(offset >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, page & chip->pagemask);

	ret = chip->read_byte(mtd) & (NAND_LOCK_STATUS_TIGHT
					  | NAND_LOCK_STATUS_LOCK
					  | NAND_LOCK_STATUS_UNLOCK);

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_unlock: - Unlock area of NAND pages
 *		  only one consecutive area can be unlocked at one time!
 *
 * @param mtd		nand mtd instance
 * @param start		start byte address
 * @param length	number of bytes to unlock (must be a multiple of
 *			page size nand->writesize)
 *
 * @return		0 on success, -1 in case of error
 */
int nand_unlock(struct mtd_info *mtd, ulong start, ulong length)
{
	int ret = 0;
	int chipnr;
	int status;
	int page;
	struct nand_chip *chip = mtd->priv;
	printf ("nand_unlock: start: %08x, length: %d!\n",
		(int)start, (int)length);

	/* select the NAND device */
	chipnr = (int)(start >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	/* check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	if (!(chip->read_byte(mtd) & NAND_STATUS_WP)) {
		printf ("nand_unlock: Device is write protected!\n");
		ret = -1;
		goto out;
	}

	if ((start & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Start address must be beginning of "
			"nand block!\n");
		ret = -1;
		goto out;
	}

	if (length == 0 || (length & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Length must be a multiple of nand block "
			"size %08x!\n", mtd->erasesize);
		ret = -1;
		goto out;
	}

	/*
	 * Set length so that the last address is set to the
	 * starting address of the last block
	 */
	length -= mtd->erasesize;

	/* submit address of first page to unlock */
	page = (int)(start >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK1, -1, page & chip->pagemask);

	/* submit ADDRESS of LAST page to unlock */
	page += (int)(length >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK2, -1, page & chip->pagemask);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);
	/* see if device thinks it succeeded */
	if (status & 0x01) {
		/* there was an error */
		ret = -1;
		goto out;
	}

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}
#endif

/**
 * get_len_incl_bad
 *
 * Check if length including bad blocks fits into device.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @return image length including bad blocks
 */
static size_t get_len_incl_bad (nand_info_t *nand, loff_t offset,
				const size_t length)
{
	size_t len_incl_bad = 0;
	size_t len_excl_bad = 0;
	size_t block_len;

	while (len_excl_bad < length) {
		block_len = nand->erasesize - (offset & (nand->erasesize - 1));

		if (!nand_block_isbad (nand, offset & ~(nand->erasesize - 1)))
			len_excl_bad += block_len;

		len_incl_bad += block_len;
		offset       += block_len;

		if (offset >= nand->size)
			break;
	}

	return len_incl_bad;
}

/**
 * nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param buf           buffer to read from
 * @return		0 in case of success
 */
int nand_write_skip_bad(nand_info_t *nand, loff_t offset, loff_t *length,
			u_char *buffer)
{
	int rval;
	size_t left_to_write = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
	    (*length & (nand->writesize - 1)) != 0) {
		printf ("Attempt to write non page aligned data\n");
		return -EINVAL;
	}

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) > nand->size) {
		printf ("Attempt to write outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = nand_write (nand, offset, length, buffer);
		if (rval != 0)
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);

		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (nand->erasesize - block_offset))
			write_size = left_to_write;
		else
			write_size = nand->erasesize - block_offset;

			rval = nand_write (nand, offset, &write_size, p_buffer);
	
		if (rval != 0) {
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
		offset        += write_size;
		p_buffer      += write_size;
	}

	return 0;
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is readen
 * instead as long as the image is short enough to fit even after skipping the
 * bad blocks.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds remaining bytes to read
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(nand_info_t *nand, loff_t offset, loff_t *length,
		       u_char *buffer)
{
	int rval;
	size_t left_to_read = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) > nand->size) {
		printf ("Attempt to read outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = nand_read (nand, offset, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;
		printf ("NAND read from offset %llx failed %d\n",
			offset, rval);
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skipping bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = nand_read (nand, offset, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			printf ("NAND read from offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

#define default_boot_name  "nandboot"
int romboot_nand_write(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf, int protect_flag)
{
	unsigned int area_size,i,cur;
	unsigned ret=0,err_flag=0;
	unsigned int w_size,total_size;
	size_t len=*plen;
	u_char * p=(u_char*)buf;
	nand_erase_options_t opts;

	struct mtd_oob_ops ops=	{.retlen=0 ,
							.mode=MTD_OOB_PLACE};
	u16    oob[]={0x55aa,0x55aa,0x55aa};
	struct mtd_info * mtd=get_mtd_device_nm(default_boot_name);

	if (IS_ERR(mtd)) {
		printk("NO nand romboot device , pls config it\n");
		printk("ABORT \n");
		return  1;	
	}	

	if(offset!=0) 
	{
		printk("Wrong addr begin\n");
		printk("ABORT \n");
		return  1;
	}

	if(mtd!=nand)
	{	
		printk("AUTO SWITCH TO BOOT DEVICE\n");
	}
	
	if(mtd->writesize<=512)
	{
		area_size=mtd->erasesize;
		w_size=512;
		ops.ooblen=2;
		total_size=512*1024;
	}else
	{
		area_size=mtd->writesize*256;
		w_size=3*512;
		total_size=mtd->writesize*1024;
		ops.ooblen=6;
		if(len>w_size*256)
		{
			printk("Wrong Length bigger\n");
			printk("ABORT \n");
			return  1;
		}
		if(len%w_size)
		{
		
			printk("Wrong Length NOt 3*512 \n");
			printk("ABORT \n");
			return  1;	
		}
	}

	if(protect_flag)
	{
		printk("^^^^^^^MUST ERASE THIS BOOT DEVICE FIRST^^^^^^^^^\n");
		printk("INPUT y for sure this \n");
		if (getc() != 'y') {
			printk("ABORT \n");
			return 1 ;
		}
	}

	opts.offset = 0;
	opts.length =total_size;
	opts.jffs2=0;
	opts.quiet  = 0;
	ret = nand_erase_opts(nand, &opts);
	printf(" erase %s\n", ret ? "ERROR" : "OK");
	if(ret)
		return 1;	
	
	ops.retlen=0;
	ops.oobbuf=(uint8_t*)&oob[0];
	ops.len=w_size;

	
	for(cur=0,i=0;cur<len;i++)
	{
			ops.datbuf=(uint8_t*)&p[cur];
			ret=mtd->write_oob(mtd,offset+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 0Aoff %lld \n",offset+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 1Aoff %lld \n",offset+area_size+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+2*area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 2Aoff %lld \n",offset+2*area_size+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+3*area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 3Aoff %lld \n",offset+3*area_size+i*(mtd->writesize));	
			}	

			cur	+=w_size;
	}

	if(err_flag)
	{
		printk("romboot_nand_write  err happen , for safe you can use cmd : nand device 1 ; nand erase ; nand device 0  \n");
		return 1;
	}	

	return 0;
}

/*int romboot_nand_read(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf)
{
	int area_size,i,cur,temp,j,k,ret=0;
	int w_size,total_size;
	size_t len=*plen;
	u_char * p=(u_char*)buf;
	struct mtd_info * mtd=get_mtd_device_nm(default_boot_name);
	struct mtd_oob_ops ops={.retlen=0 ,
							.mode=MTD_OOB_PLACE};
	u16    oob[]={0,0,0};
	ops.retlen=0;
	ops.oobbuf=(uint8_t*)&oob[0];
	if(mtd->writesize<=512)
	{
		area_size=mtd->erasesize;
		w_size=512;
		ops.ooblen=2;
		total_size=512*1024;
	}else{
		area_size=mtd->writesize*256;
		w_size=3*512;
		total_size=mtd->writesize*1024;
		ops.ooblen=6;
		if(len>w_size*256)
		{
			printk("Wrong Length\n");
			BUG();
		}
	}
	ops.len=w_size;
	for(i=0,cur=0;i<total_size&&cur<len;i+=area_size)
	{
		temp=cur;
		for(j=0;j<area_size;j+=mtd->erasesize)
		{
			for(k=0;k<mtd->erasesize;k+=mtd->writesize)
			{
				ops.datbuf=&p[temp];
				ret=mtd->read_oob(mtd,i+j+k,&ops);
				if(ret)
					break;
				temp+=w_size;
				if(oob[0]!=0x55aa){
					ret=1;
					break;
				}
				if(ops.ooblen==6&&oob[1]!=0x55aa&&oob[2]!=0x55aa)
				{
					ret=1;
					break;
				}
			}
			if(ret)
				break;



		}
		if(ret)
			continue;
		cur=temp;
	}
	if(i>total_size&&cur<len)
	{
		printk("Error \n");
		return -1;
	}
	return 0;
}*/
