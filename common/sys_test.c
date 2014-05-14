#include <common.h>
#include <command.h>
#include <aml_i2c.h>
#include <aml_rtc.h>
#include <rtc.h>
#include <asm/arch/gpio.h>
#include <lcd_aml.h>
#include <ts.h>
#include <tsc2007.h>
#include "sys_test.h"
#include <nand.h>
#include <mmc.h>
#include <asm/arch/nand.h>

extern vidinfo_t panel_info;

int systest_free(char **buf)
{
	int i = 0;

	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	for(i=0; i<MAX_TEST_RESULT_NUM; i++) {
		if(buf[i]) {
			free((buf[i]));
		}
		buf[i] = NULL;
		
	}
	return 0;
}

int systest_malloc(char **buf)
{
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	*buf = (char *)malloc(MAX_TEST_RESULT_CHARS);
	if(*buf == NULL) {
		printf("%s: malloc is fail!\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

/* Nand Flash*/
static int nand_test(int item, unsigned int number, char **buf)
{
    nand_info_t *nand;
	int i = 0, count = 0;
	
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE+1; i++) {
		nand = nand_info[i];
		if (!nand) {
			nand_init();
			if (!nand) {
				if(systest_malloc(buf+count) < 0) {
					printf("%s: malloc is fail!", __FUNCTION__);
					return -1;
				}
				count++;
				sprintf(buf[count], "No NAND device available!");
				printf("%s: nand is null!\n", __FUNCTION__);
				return -1;
			}
		}
		if (nand->name) {
			nand = nand_info[i];
			struct nand_chip *chip = nand->priv;
				
			if(systest_malloc(buf+count) < 0) {
				printf("%s: malloc is fail!\n", __FUNCTION__);
				return -1;
			}
				
			//printf("Device %d:\n", i);
			sprintf(buf[count], "Device %d:", i);
			count++;
				
			if(systest_malloc(buf+count) < 0) {
				printf("%s: malloc is fail!\n", __FUNCTION__);
				return -1;
			}
#ifdef CONFIG_MTD_DEVICE
			sprintf(buf[count], "%s, %s sector size %u KiB",
					  nand->name, nand->info, nand->erasesize >> 10);
#else 
			sprintf(buf[count], "%s, sector size %u KiB",
					  nand->name, nand->erasesize >> 10);
#endif
            count++;
		}
	}
	return 1;
}

/* SD Card*/
static int sdcard_test(int item, unsigned int number, char **buf)
{
	struct mmc *mmc;
	int dev_num;
	int count = 0;
	
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}

	dev_num = simple_strtoul("mmcinfo", NULL, 0);

	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Device: %s", mmc->name);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Manufacturer ID: %x", mmc->cid[0] >> 24);
		count++;
		
		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "OEM: %x", (mmc->cid[0] >> 8) & 0xffff);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Name: %c%c%c%c%c", mmc->cid[0] & 0xff,
				(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
				(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
		count++;
		
		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Tran Speed: %d", mmc->tran_speed);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Rd Block Len: %d", mmc->read_bl_len);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "%s version %d.%d", IS_SD(mmc) ? "SD" : "MMC",
				(mmc->version >> 4) & 0xf, mmc->version & 0xf);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "High Capacity: %s", mmc->high_capacity ? "Yes" : "No");
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Capacity: %lld", mmc->capacity);
		count++;

		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!\n", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "Bus Width: %d-bit", mmc->bus_width);
	}
	else {
		if(systest_malloc(buf+count) < 0) {
			printf("%s: malloc is fail!", __FUNCTION__);
			return -1;
		}
		sprintf(buf[count], "No MMC device available!");
		printf("%s: mmc is null!\n", __FUNCTION__);
		return -1;
	}
}

/* USB Storage*/
static int usb_test(int item, unsigned int number, char **buf)
{
#if 0
    usb_power_on();
	run_command ("usb start", 0);
	run_command ("usb info", 0);
#endif
	return 1;
}

/* I2C */
#ifdef CONFIG_AML_I2C
struct i2c_board_info *aml_i2c_info = NULL;
static unsigned int i2c_dev_num = 0;

void i2c_init(struct i2c_board_info *i2c_info, unsigned int len)
{
    if(i2c_info != NULL)
		aml_i2c_info = i2c_info;
	i2c_dev_num = len;
}

int i2c_xfer(unsigned int addr)
{
    unsigned char cmd = 0x0;
	
	struct i2c_msg msgs[] = {
	    {
	        .addr = addr,
	        .flags = 0,
	        .len = 1,
	        .buf = &cmd,
	    }
	};

	if(aml_i2c_xfer(msgs, 1) < 0) {
	    printf("%s: i2c transfer failed!\n", __FUNCTION__);
		return -1;
	}

	return 0;
}
#endif
static int i2c_test(int item, unsigned int number, char **buf)
{
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}
	
	if(systest_malloc(buf) < 0) {
		printf("%s: malloc is fail!\n", __FUNCTION__);
		return -1;
	}
	
	if(aml_i2c_info[item].device_init) 
		aml_i2c_info[item].device_init();
		
	if(i2c_xfer(aml_i2c_info[item].addr) < 0) {
		sprintf(*buf, "%s i2c transfer failed!", aml_i2c_info[item].type);
		printf("%s: i2c transfer failed!\n", __FUNCTION__);
		return -1;
	}
	sprintf(*buf, "%s, addr is %d", aml_i2c_info[item].type, aml_i2c_info[item].addr);
	return 1;
}

/* ADC */
// get saradc value
static int adc_test(int item, unsigned int number, char **buf)
{
	int adc_val;
	
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}
	
	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}
	
	if(systest_malloc(buf) < 0) {
		printf("%s: malloc is fail!\n", __FUNCTION__);
		return -1;
	}
	saradc_enable();
	adc_val = get_adc_sample(item);
	sprintf(*buf, "SARADC[%d]: 0x%03x", item, adc_val);
	return 1;
}

/* PWM */
static int bl_set_level(unsigned level)
{
	panel_info.set_bl_level(level);
	return 0;
}

// adjust backlight
static int pwm_test(int item, unsigned int number, char **buf)
{
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}

	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}
	
	if(systest_malloc(buf) < 0) {
		printf("%s: malloc is fail!\n", __FUNCTION__);
		return -1;
	}
	
	if(item == 0){
		bl_set_level(128/2);
		sprintf(*buf, "set backlight low!");
	} else if(item == 1) {
		bl_set_level(128*2);
		sprintf(*buf, "set backlight high!");
	} else {
		bl_set_level(128);
		sprintf(*buf, "set backlight normal!");
	}
	return 1;
}

/* UART */
static int uart_test(int item, unsigned int number, char **buf)
{
	return 1;
}

/* RTC */
static int rtc_read_time(struct rtc_time *tm)
{
	aml_rtc_read_time(tm);
	return 0;
}
static int rtc_write_time(struct rtc_time *tm)
{
	aml_rtc_write_time(tm);
	return 0;
}
// read/write RTC 
static int rtc_test(int item, unsigned int number, char **buf)
{
	struct rtc_time tm;
	
	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}

	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}
	
	if(systest_malloc(buf) < 0) {
		printf("%s: malloc is fail!\n", __FUNCTION__);
		return -1;
	}
	
	if(item == 0) {
		tm.tm_year = 111;
		tm.tm_mon = 0;
		tm.tm_mday = 1;						
		tm.tm_hour = 0;						
		tm.tm_min = 0;
		tm.tm_sec = 0;	 
		rtc_write_time(&tm); 
		sprintf(*buf, "set time: 2011-01-01 00:00:00");
	}
	else if(item == 1) {
		rtc_read_time(&tm);
		sprintf(*buf, "get time: %04d-%02d-%02d %02d:%02d:%02d", 
					tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else if(item == 2) {
		rtc_read_time(&tm);
		sprintf(*buf, "get time: %04d-%02d-%02d %02d:%02d:%02d", 
					tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else if(item == 3) {
		rtc_read_time(&tm);
		sprintf(*buf, "get time: %04d-%02d-%02d %02d:%02d:%02d", 
					tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else if(item == 4) {
		rtc_read_time(&tm);
		sprintf(*buf, "get time: %04d-%02d-%02d %02d:%02d:%02d", 
					tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);
	}			
	return 1;
}

/* GPIO */
// set GPIOA_2
static int gpio_test(int item, unsigned int number, char **buf)
{
	return 1;
}

/*TouchScreen*/
static int touch_test(int item, unsigned int number, char **buf)
{
	int ret = 0;
	struct ts_event tc;
	int countdown = 60*1000*10;//60 second

	if(buf == NULL) {
		printf("%s: buf is null!\n", __FUNCTION__);
		return -1;
	}

	if(number <= 0) {
		printf("%s: buf number is overloop!\n", __FUNCTION__);
		return 0;
	}
	
	ret = touch_init();
	if(ret < 0) {
		printf("%s: touch init fail!\n", __FUNCTION__);
		return -1;
	}
	
	while(countdown > 0) {
		udelay(100);
		ret = touch_work(&tc);
		if(ret < 0) {
			printf("%s: touch work fail!\n", __FUNCTION__);
			return -1;
		}
		else if(ret > 0) {
			if(systest_malloc(buf) < 0) {
				printf("%s: malloc is fail!\n", __FUNCTION__);
				return -1;
			}
			sprintf(*buf, "(x,y)=(%d,%d),(x1,y1)=(%d,%d),touch=%d", tc.x, tc.y, tc.x1, tc.y1,tc.touch);
			return 1;
		}
		countdown -= 1;	
	}
	return -1;
}

static int hdmi_test(int item, unsigned int number, char **buf)
{
    run_command ("tv open 720P", 0);
    return 1;
}

static titeminfo_t nand_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 1,
	.title       = "Nand Test",
	.do_action   = nand_test,
};

static titeminfo_t sdcard_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 1,
	.title       = "SD Card Test",
	.do_action   = sdcard_test,
};

static titeminfo_t usb_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 1,
	.title       = "USB Test",
	.do_action   = usb_test,
};

static titeminfo_t i2c_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 0,
	.title       = "I2C Test",
	.do_action   = i2c_test,
};

static titeminfo_t adc_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 6,
	.title       = "ADC Test",
	.do_action   = adc_test,
};

static titeminfo_t pwm_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 3,
	.title       = "PWM Test",
	.do_action   = pwm_test,
};

static titeminfo_t uart_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 0,
	.title       = "UART Test",
	.do_action   = uart_test,
};

static titeminfo_t rtc_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 5,
	.title       = "RTC Test",
	.do_action   = rtc_test,
};

static titeminfo_t gpio_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 0,
	.title       = "GPIO Test",
	.do_action   = gpio_test,
};

static titeminfo_t touch_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 5,
	.title       = "Touch Test",
	.do_action   = touch_test,
};

static titeminfo_t hdmi_test_item = {
	.open_close  = ITEM_CLOSE,
	.test_type   = TEST_START,
	.pos         = 0,
	.title_width = 0,
	.item_nums   = 1,
	.title       = "HDMI Test",
	.do_action   = hdmi_test,
};

typedef struct st {
	int number;
	titeminfo_t test_items[MAX_TEST_ITEMS_NUM];
	char *buff[MAX_TEST_RESULT_NUM];
}st_t;
st_t st;

static void set_test_items(st_t *st)
{
	int i = 0;
	int width = 0;
	int height = 0;
	int world_len = 0;
	
	st->number  = 0;
	if(ITEM_NAND_TEST == ITEM_OPEN) {
		st->test_items[i] = nand_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_SDCARD_TEST == ITEM_OPEN) {
		st->test_items[i] = sdcard_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_USB_TEST == ITEM_OPEN) {
		st->test_items[i] = usb_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_I2C_TEST == ITEM_OPEN) {
		st->test_items[i] = i2c_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		#ifdef CONFIG_AML_I2C
		if(aml_i2c_info != NULL) {
			st->test_items[i].item_nums = i2c_dev_num;
			//printf("%s: item_nums is %d\n", __FUNCTION__, st->test_items[i].item_nums);
		}
		#endif
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_ADC_TEST == ITEM_OPEN) {
		st->test_items[i] = adc_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_PWM_TEST == ITEM_OPEN) {
		st->test_items[i] = pwm_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_UART_TEST == ITEM_OPEN) {
		st->test_items[i] = uart_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_RTC_TEST == ITEM_OPEN) {
		st->test_items[i] = rtc_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_GPIO_TEST == ITEM_OPEN) {
		st->test_items[i] = gpio_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_TOUCH_TEST == ITEM_OPEN) {
		st->test_items[i] = touch_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}
	if(ITEM_HDMI_TEST == ITEM_OPEN) {
		st->test_items[i] = hdmi_test_item;
		st->test_items[i].open_close = ITEM_OPEN;
		width = 0;
		GetTextSize(st->test_items[i].title, strlen(st->test_items[i].title), &width, &height, &world_len);
		st->test_items[i].title_width = width;
		st->test_items[i].pos = i;
		i++;
		st->number = i;
		if(st->number > MAX_TEST_ITEMS_NUM) {
			return;
		}
	}

	printf("%s: st_t number is %d\n", __FUNCTION__, st->number);
}

typedef struct sd {
	int first_x;
	int first_y;
	int test_items_y;
	int result_title_y;
	int result_y;
	int max_item_width;
}sd_t;
sd_t sd = {
	.first_x        = 40,
	.first_y        = 20,
	.test_items_y   = 0,
	.result_title_y = 0,
	.result_y       = 0,
	.max_item_width   = 0,
};

static void display_test_title(sd_t *sd)
{
	uchar context[13] = "System test:";
	AsciiPrintf(context, sd->first_x, sd->first_y, DISPLAY_WHITE_COLOR);
	sd->test_items_y = sd->first_y + GetCharHeight();
}

static void displsy_all_test_items(st_t *st, sd_t *sd)
{
	int font_height = GetCharHeight();
	int i = 0;
	
	sd->test_items_y += 10;
	for(i=0; i<st->number; i++) {
		if(st->test_items[i].open_close == ITEM_OPEN) {
			if(sd->max_item_width < st->test_items[i].title_width) {
				sd->max_item_width = st->test_items[i].title_width;
			}
			AsciiPrintf((uchar *)st->test_items[i].title, sd->first_x, sd->test_items_y+font_height*i, DISPLAY_WHITE_COLOR);
		}
		else {
			break;
		}
	}
	sd->result_title_y = sd->test_items_y+font_height*i;
}

static void display_test_items(st_t *st, sd_t *sd, int pos, int bg_color) 
{
	int font_height = GetCharHeight();
	
	if(pos >= st->number) {
		return;
	}
	DrawRect(sd->first_x, sd->test_items_y+font_height*pos, st->test_items[pos].title_width, font_height, bg_color);
	AsciiPrintf((uchar *)st->test_items[pos].title, sd->first_x, sd->test_items_y+font_height*pos, DISPLAY_WHITE_COLOR);
}

static void display_result_title(sd_t *sd) 
{
	uchar context[20] = "System test result:";
	
	DrawRect(sd->first_x, sd->result_title_y+10, panel_info.vl_col-sd->first_x*2, 1, DISPLAY_WHITE_COLOR);
	AsciiPrintf(context, sd->first_x, sd->result_title_y+20, DISPLAY_WHITE_COLOR);
		
	sd->result_y = sd->result_title_y+GetCharHeight()+20;
}

static void display_result(sd_t *sd, int pos, char **buf) 
{
	int i = 0, j = 0;
	int font_height = GetCharHeight();
	
	DrawRect(sd->first_x, sd->result_y, panel_info.vl_col-sd->first_x, panel_info.vl_row-sd->result_y, DISPLAY_BLACK_COLOR);
		
	for(i=0; i<MAX_TEST_RESULT_NUM; i++) {
		if(buf[i]) {
			AsciiPrintf((uchar *)buf[i], sd->first_x, sd->result_y+font_height*j, DISPLAY_WHITE_COLOR);
			j++;
		}
	}		
}

static void display_pass_or_fail(sd_t *sd, int pos, int flag) 
{
	int font_height = GetCharHeight();

	if(flag == TEST_SUCESS) {
		uchar context[5] = "PASS";
		
		DrawRect(sd->first_x*2+sd->max_item_width, sd->test_items_y+font_height*pos, panel_info.vl_col-sd->first_x*2-sd->max_item_width, font_height, DISPLAY_BLACK_COLOR);
		AsciiPrintf(context, sd->first_x*2+sd->max_item_width, sd->test_items_y+font_height*pos, DISPLAY_BLUE_COLOR);
	}
	else if(flag == TEST_FAIL) {
		uchar context[5] = "FAIL";
		
		DrawRect(sd->first_x*2+sd->max_item_width, sd->test_items_y+font_height*pos, panel_info.vl_col-sd->first_x*2-sd->max_item_width, font_height, DISPLAY_BLACK_COLOR);
		AsciiPrintf(context, sd->first_x*2+sd->max_item_width, sd->test_items_y+font_height*pos, DISPLAY_RED_COLOR);
	}		
}

void main_test(void)
{
	int i = 0, j = 0;
	
	set_test_items(&st);
	display_test_title(&sd);
	displsy_all_test_items(&st, &sd);
	display_result_title(&sd);
	display_test_items(&st, &sd, i, DISPLAY_BLUE_COLOR);
	
	if(st.number <= 0) {
		printf("%s: st_t number <= 0\n", __FUNCTION__);
		return;
	}
	
	for(j=0; j<MAX_TEST_RESULT_NUM; j++) {
		st.buff[j] = NULL;
	}
	
	sdelay(5);
	if(TESTACTION_TYPE == AUTO_TEST) {
		for(i=0; i<st.number; i++) {
			if((st.test_items[i].open_close == ITEM_OPEN) && (st.test_items[i].do_action != NULL)) {
				if(st.test_items[i].test_type != TEST_START) {
					printf("%s: item%d test_type != TEST_START\n", __FUNCTION__, i);
					continue;
				}
				if(i > 0)
					display_test_items(&st, &sd, i-1, DISPLAY_BLACK_COLOR);
				display_test_items(&st, &sd, i, DISPLAY_BLUE_COLOR);
				systest_free(st.buff);
				for(j=0; j<st.test_items[i].item_nums; j++) {
					if(st.test_items[i].do_action(j, MAX_TEST_RESULT_NUM-j, st.buff+j) < 0) {
						display_result(&sd, i, st.buff);
						sdelay(5);
						st.test_items[i].test_type = TEST_FAIL;
						display_pass_or_fail(&sd, i, TEST_FAIL);
						break;
					}
					else {
						display_result(&sd, i, st.buff);
						sdelay(1);
					}
				}
				if(j == st.test_items[i].item_nums) {
					sdelay(5);
					//display_result(&sd, i, st.buff);
					st.test_items[i].test_type = TEST_SUCESS;
					display_pass_or_fail(&sd, i, TEST_SUCESS);
				}
			}
			else {
				printf("%s: item%d open_close != ITEM_OPEN or do_action is null\n", __FUNCTION__, i);
				break;
			}
		}
	}
	else {
		while(1){
			mdelay(200);
			if(i >= st.number) {
				printf("i >= systest number break\n", __FUNCTION__);
				break;
			}
			int tmp = get_key_value();
			if(get_key())
				printf("%s: get_key = %d\n", __FUNCTION__, tmp);
			if(KEY_EXIT(tmp)) {
				printf("KEY_EXIT break\n");
				break;
			}
			else if(KEY_UP(tmp)) {
				display_test_items(&st, &sd, i, DISPLAY_BLACK_COLOR);
				if(i <= 0){
					i = st.number-1;
				}
				else {
					i--;
				}
				display_test_items(&st, &sd, i, DISPLAY_BLUE_COLOR);
			}
			else if(KEY_DOWN(tmp)) {
				display_test_items(&st, &sd, i, DISPLAY_BLACK_COLOR);
				if(i >= st.number-1) {
					i = 0;
				}
				else {
					i++;
				}
				display_test_items(&st, &sd, i, DISPLAY_BLUE_COLOR);
			}
			else if(KEY_ENTER(tmp)) {
				if((st.test_items[i].open_close != ITEM_OPEN) || (st.test_items[i].do_action == NULL)) {
					printf("KEY_ENTER parameter error\n");
					break;
				}
				if(st.test_items[i].test_type != TEST_START) {
					continue;
				}
				systest_free(st.buff);
				for(j=0; j<st.test_items[i].item_nums; j++) {
					if(st.test_items[i].do_action(j, MAX_TEST_RESULT_NUM-j, st.buff+j) < 0) {
						display_result(&sd, i, st.buff);
						sdelay(5);
						st.test_items[i].test_type = TEST_FAIL;
						display_pass_or_fail(&sd, i, TEST_FAIL);
						break;
					}
					else {
						display_result(&sd, i, st.buff);
						sdelay(1);
					}
				}
				if(j == st.test_items[i].item_nums) {
					sdelay(5);
					//display_result(&sd, i, st.buff);
					st.test_items[i].test_type = TEST_SUCESS;
					display_pass_or_fail(&sd, i, TEST_SUCESS);
				}
			}
		}
	}
	systest_free(st.buff);
}
