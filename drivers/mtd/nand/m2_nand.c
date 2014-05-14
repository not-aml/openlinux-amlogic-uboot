
#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/nand.h>
#include <malloc.h>
#include <linux/err.h>
#include <asm/cache.h>
#include <asm/arch/pinmux.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>

int nand_curr_device = -1;
//extern struct aml_nand_platform aml_nand_platform[];
extern struct aml_nand_device aml_nand_mid_device;
#define CONFIG_SYS_NAND_MAX_DEVICE 		4
nand_info_t *nand_info[CONFIG_SYS_NAND_MAX_DEVICE];

static void aml_m2_select_chip(struct aml_nand_chip *aml_chip, int chipnr)
{
	switch (chipnr) {
		case 0:
		case 1:
		case 2:
		case 3:
			NFC_SEND_CMD_IDLE(aml_chip->chip_enable[chipnr], 0);
			aml_chip->chip_selected = aml_chip->chip_enable[chipnr];
			aml_chip->rb_received = aml_chip->rb_enable[chipnr];

			if (!((aml_chip->chip_selected >> 10) & 1))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 4));
			if (!((aml_chip->chip_selected >> 10) & 2))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 3));
			if (!((aml_chip->chip_selected >> 10) & 4))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 14));
			if (!((aml_chip->chip_selected >> 10) & 8))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 13));

			if (!((aml_chip->rb_received >> 10) & 1))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 2));
			if (!((aml_chip->rb_received >> 10) & 2))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 1));
			if (!((aml_chip->rb_received >> 10) & 4))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 12));
			if (!((aml_chip->rb_received >> 10) & 8))
				SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 11));

			break;

		default:
			BUG();
			aml_chip->chip_selected = CE_NOT_SEL;
			break;
	}
	return;
}

static int m1_nand_probe(struct aml_nand_platform *plat, unsigned dev_num)
{
	struct aml_nand_chip *aml_chip = NULL;
	struct nand_chip *chip = NULL;
	struct mtd_info *mtd = NULL;
	int err = 0;

	if (!plat) {
		printk("no platform specific information\n");
		goto exit_error;
	}

	aml_chip = kzalloc(sizeof(*aml_chip), GFP_KERNEL);
	if (aml_chip == NULL) {
		printk("no memory for flash info\n");
		err = -ENOMEM;
		goto exit_error;
	}

	/* initialize mtd info data struct */
	aml_chip->aml_nand_select_chip = aml_m2_select_chip;
	aml_chip->platform = plat;
	chip = &aml_chip->chip;
	chip->priv = &aml_chip->mtd;
	mtd = &aml_chip->mtd;
	mtd->priv = chip;
	mtd->name = plat->name;

	err = aml_nand_init(aml_chip);
	if (err)
		goto exit_error;

	nand_info[dev_num] = &aml_chip->mtd;
	return 0;

exit_error:
	if (aml_chip)
		kfree(aml_chip);
	mtd->name = NULL;
	return err;
}

#define DRV_NAME	"aml_m2_nand"
#define DRV_VERSION	"1.0"
#define DRV_AUTHOR	"xiaojun_yoyo"
#define DRV_DESC	"Amlogic nand flash uboot driver for m2"

void nand_init(void)
{
	struct aml_nand_platform *plat = NULL;
	int i, ret;
	printk(KERN_INFO "%s, Version %s (c) 2010 Amlogic Inc.\n", DRV_DESC, DRV_VERSION);

	for (i=0; i<aml_nand_mid_device.dev_num; i++) {
		plat = &aml_nand_mid_device.aml_nand_platform[i];
		if (!plat) {
			printk("error for not platform data\n");
			continue;
		}
		ret = m1_nand_probe(plat, i);
		if (ret) {
			printk("nand init faile: %d\n", ret);
			continue;
		}
	}
	if (aml_nand_mid_device.dev_num  >  0)
		nand_curr_device = (aml_nand_mid_device.dev_num - 1);
	else
		nand_curr_device = 0;
	return;
}

