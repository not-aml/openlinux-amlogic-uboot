/*
 * (C) Copyright 2008
 * Stuart Wood, Lab X Technologies <stuart.wood@labxtechnologies.com>
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.

 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>

#if defined(CONFIG_CMD_SAVEENV) && defined(CONFIG_CMD_NAND)
#define CMD_SAVEENV
#elif defined(CONFIG_ENV_OFFSET_REDUND)
#error Cannot use CONFIG_ENV_OFFSET_REDUND without CONFIG_CMD_SAVEENV & CONFIG_CMD_NAND
#endif

#if defined(CONFIG_ENV_SIZE_REDUND) && (CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

#ifdef CONFIG_INFERNO
#error CONFIG_INFERNO not supported yet
#endif

#ifndef CONFIG_ENV_RANGE
#define CONFIG_ENV_RANGE	CONFIG_ENV_SIZE
#endif

#ifndef CONFIG_ENV_BLOCK_NUM
#define CONFIG_ENV_BLOCK_NUM	4
#endif

//#define __DBG__ENV__
#ifdef __DBG__ENV__
#define _debug(fmt,args...) printk(fmt, ##args)
#else
#define _debug(fmt,args...)
#endif

int nand_legacy_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "NAND";


#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

typedef struct _env_blockmap{
	u_char block_id;
	u_char block_valid;
	u_char env_valid;
	u_char reserved;
}t_env_blockmap;
static t_env_blockmap env_map[CONFIG_ENV_BLOCK_NUM];
static u_char last_valid_block = 0;
static u_char current_valid_block = 0;
uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which does the real
 * validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash. This way
 * the SPL loads not only the U-Boot image from NAND but also the
 * environment.
 */
int env_init(void)
{
	int blocksize = nand_info[0].erasesize;
	nand_erase_options_t nand_erase_options;
	
	nand_erase_options.length = blocksize;
	nand_erase_options.quiet = 0;
	nand_erase_options.jffs2 = 0;
	nand_erase_options.scrub = 0;
	nand_erase_options.offset = CONFIG_ENV_OFFSET;
	
#if defined(ENV_IS_EMBEDDED)
	size_t total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1, *tmp_env2;

	total = CONFIG_ENV_SIZE;

	tmp_env1 = env_ptr;
	tmp_env2 = (env_t *)((ulong)env_ptr + CONFIG_ENV_SIZE);

	crc1_ok = (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok)
		gd->env_valid = 0;
	else if(crc1_ok && !crc2_ok)
		gd->env_valid = 1;
	else if(!crc1_ok && crc2_ok)
		gd->env_valid = 2;
	else {
		/* both ok - check serial */
		if(tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if(tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if(tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if(tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	if (gd->env_valid == 1)
		env_ptr = tmp_env1;
	else if (gd->env_valid == 2)
		env_ptr = tmp_env2;
#else /* ENV_IS_EMBEDDED */
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;

#endif /* ENV_IS_EMBEDDED */

	return (0);
}

#ifdef CMD_SAVEENV
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
int writeenv(size_t offset, u_char *buf)
{
	size_t addr;
	size_t amount_saved = 0;
	size_t pagesize, blocksize, len;
	
	u_char *char_ptr;
	u_char num = 0;
	blocksize = nand_info[0].erasesize;
	pagesize = nand_info[0].writesize;
	len = min(pagesize, CONFIG_ENV_SIZE);
	u_char tmp_buf[pagesize];
	tmp_buf[0] = 0xaa;
	tmp_buf[1] = 0x55;
	last_valid_block = current_valid_block++;
	current_valid_block = current_valid_block % CONFIG_ENV_BLOCK_NUM;
	_debug("write env block: %d\n",current_valid_block);
	while(num < CONFIG_ENV_BLOCK_NUM) {
		if(env_map[current_valid_block].block_valid) {
			addr = offset + current_valid_block*blocksize;
			while (amount_saved < CONFIG_ENV_SIZE ) {
				char_ptr = &buf[amount_saved];
				if (nand_write(&nand_info[0], addr, &len, char_ptr)) {
					env_map[current_valid_block].block_valid = 0;
					amount_saved = 0;
					break;
				}
				addr += pagesize;
				len = min(pagesize, CONFIG_ENV_SIZE - amount_saved);
				amount_saved += pagesize;
			}
			if(amount_saved >= CONFIG_ENV_SIZE){
				nand_write(&nand_info[0], addr, &len, tmp_buf);
				return 0;
			}
		}
		num++;
		current_valid_block++;
		current_valid_block = current_valid_block % CONFIG_ENV_BLOCK_NUM;
	}
	if (amount_saved < CONFIG_ENV_SIZE)
		return 1;

	return 0;
}
#ifdef CONFIG_ENV_OFFSET_REDUND
int saveenv(void)
{
	size_t total;
	int ret = 0;
	nand_erase_options_t nand_erase_options;

	env_ptr->flags++;
	total = CONFIG_ENV_SIZE;

	nand_erase_options.length = CONFIG_ENV_RANGE;
	nand_erase_options.quiet = 0;
	nand_erase_options.jffs2 = 0;
	nand_erase_options.scrub = 0;

	if (CONFIG_ENV_RANGE < CONFIG_ENV_SIZE)
		return 1;
	if(gd->env_valid == 1) {
		puts ("Erasing redundant Nand...\n");
		nand_erase_options.offset = CONFIG_ENV_OFFSET_REDUND;
		if (nand_erase_opts(&nand_info[0], &nand_erase_options))
			return 1;

		puts ("Writing to redundant Nand... ");
		ret = writeenv(CONFIG_ENV_OFFSET_REDUND, (u_char *) env_ptr);
	} else {
		puts ("Erasing Nand...\n");
		nand_erase_options.offset = CONFIG_ENV_OFFSET;
		if (nand_erase_opts(&nand_info[0], &nand_erase_options))
			return 1;

		puts ("Writing to Nand... ");
		ret = writeenv(CONFIG_ENV_OFFSET, (u_char *) env_ptr);
	}
	if (ret) {
		puts("FAILED!\n");
		return 1;
	}

	puts ("done\n");
	gd->env_valid = (gd->env_valid == 2 ? 1 : 2);
	return ret;
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
int saveenv(void)
{
	size_t total;
	int ret = 0;
	size_t offset = CONFIG_ENV_OFFSET;
	
	nand_erase_options_t nand_erase_options;
	int blocksize = nand_info[0].erasesize;
	nand_erase_options.length = blocksize;
	nand_erase_options.quiet = 1;
	nand_erase_options.jffs2 = 0;
	nand_erase_options.scrub = 0;

	if (CONFIG_ENV_SIZE > blocksize){
		puts("Error,env size bigger then blocksize! \
		\nPlease use CONFIG_ENV_IS_IN_NAND to config default storage way!\n");
		return 1;
	}
			
	puts ("Writing to Nand... \n");
	if (writeenv(CONFIG_ENV_OFFSET, (u_char *) env_ptr)) {
		puts("FAILED!\n");
		return 1;
	}
	puts("Successful!\n");
	nand_erase_options.offset = CONFIG_ENV_OFFSET + last_valid_block*blocksize;
	if (nand_erase_opts(&nand_info[0], &nand_erase_options)){
		env_map[last_valid_block].block_valid = 0;
	}		
	return ret;
}
#endif /* CONFIG_ENV_OFFSET_REDUND */
#endif /* CMD_SAVEENV */

int readenv (size_t offset, u_char * buf)
{
	size_t addr = 0;
	size_t amount_loaded = 0;
	size_t pagesize, blocksize, len;

	int i;
	u_char env_flag = 0;
	static u_char map_flag = 0;
	u_char *char_ptr;
	u_char blk_num = 0;
	
	pagesize = nand_info[0].writesize;
	blocksize = nand_info[0].erasesize;
	len = min(pagesize, CONFIG_ENV_SIZE);
	u_char temp_buf[pagesize];
	nand_erase_options_t nand_erase_options;
	nand_erase_options.length = blocksize;
	nand_erase_options.quiet = 1;
	nand_erase_options.jffs2 = 0;
	nand_erase_options.scrub = 0;	
	if(map_flag == 0) {
		for(i=0; i<CONFIG_ENV_BLOCK_NUM; i++){  //scan block, find valid env block
			addr = offset + i*blocksize;
			env_map[i].block_id = (u_char)i;
			env_map[i].block_valid = (nand_block_isbad(&nand_info[0], addr)==0);
			nand_read(&nand_info[0], addr+CONFIG_ENV_SIZE, &len, temp_buf);
			env_map[i].env_valid = ((temp_buf[0] == 0xaa)&&(temp_buf[1] == 0x55));
			env_flag += env_map[i].env_valid;
			_debug("read %d: %x, %x, %x\n",i,temp_buf[0],temp_buf[1],env_map[i].block_valid);
		}
		if(env_flag == 0){
			for(i=0;i<CONFIG_ENV_BLOCK_NUM;i++){
				nand_erase_options.offset = CONFIG_ENV_OFFSET + i*blocksize;
				env_map[i].block_valid = (nand_erase_opts(&nand_info[0], &nand_erase_options)==0);
				if(env_map[i].block_valid)
					current_valid_block = i;
			}			
		}
		if(env_flag > 1){ //conflict resolve, if more than one block has valid env flag
			if((env_map[0].env_valid==1) && (env_map[CONFIG_ENV_BLOCK_NUM-1].env_valid==1)){
				nand_erase_options.offset = CONFIG_ENV_OFFSET + (CONFIG_ENV_BLOCK_NUM-1)*blocksize;
				env_map[CONFIG_ENV_BLOCK_NUM-1].block_valid=(nand_erase_opts(&nand_info[0], &nand_erase_options)==0);
			}
			else{
				for(i=0;i<CONFIG_ENV_BLOCK_NUM-1;i++){
					if(env_map[i].env_valid==1){
						nand_erase_options.offset = CONFIG_ENV_OFFSET + i*blocksize;
						env_map[i].block_valid = (nand_erase_opts(&nand_info[0], &nand_erase_options)==0);
						break;
					}
				}
			}
		}
		map_flag++;
	}
	
	while(blk_num < CONFIG_ENV_BLOCK_NUM){
		if(env_map[blk_num].env_valid){
			current_valid_block = blk_num;
			break;
		}
		blk_num++;
	}
	if(blk_num == CONFIG_ENV_BLOCK_NUM)
		if(!env_map[blk_num-1].env_valid)
			return 1;
	_debug("read env block: %d\n",current_valid_block);
	addr = offset + current_valid_block * blocksize;
	_debug("read addr: %x\n",addr);
	while (amount_loaded < CONFIG_ENV_SIZE ) {	
		char_ptr = &buf[amount_loaded];
		if (nand_read(&nand_info[0], addr, &len, char_ptr))
			return 1;
		addr += pagesize;
		len = min(pagesize, CONFIG_ENV_SIZE-amount_loaded);
		amount_loaded += pagesize;

	}
	if (amount_loaded < CONFIG_ENV_SIZE)
		return 1;

	return 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED)
	size_t total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1, *tmp_env2;

	total = CONFIG_ENV_SIZE;

	tmp_env1 = (env_t *) malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *) malloc(CONFIG_ENV_SIZE);

	if (readenv(CONFIG_ENV_OFFSET, (u_char *) tmp_env1))
		puts("No Valid Environment Area Found\n");
	if (readenv(CONFIG_ENV_OFFSET_REDUND, (u_char *) tmp_env2))
		puts("No Valid Reundant Environment Area Found\n");

	crc1_ok = (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if(!crc1_ok && !crc2_ok) {
		free(tmp_env1);
		free(tmp_env2);
		return use_default();
	} else if(crc1_ok && !crc2_ok)
		gd->env_valid = 1;
	else if(!crc1_ok && crc2_ok)
		gd->env_valid = 2;
	else {
		/* both ok - check serial */
		if(tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if(tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if(tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if(tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;

	}

	free(env_ptr);
	if(gd->env_valid == 1) {
		env_ptr = tmp_env1;
		free(tmp_env2);
	} else {
		env_ptr = tmp_env2;
		free(tmp_env1);
	}

#endif /* ! ENV_IS_EMBEDDED */
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret;

	ret = readenv(CONFIG_ENV_OFFSET, (u_char *) env_ptr);
	if (ret)
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif /* ! ENV_IS_EMBEDDED */
}
#endif /* CONFIG_ENV_OFFSET_REDUND */

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts ("*** Warning - bad CRC or NAND, using default environment\n\n");
	set_default_env();
}
#endif
