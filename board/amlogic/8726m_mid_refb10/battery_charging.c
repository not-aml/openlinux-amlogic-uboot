#include <common.h>
#include <asm/arch/am_regs.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>


#define is_ac_connected ((READ_CBUS_REG(ASSIST_HW_REV)&(1<<9))? 1:0)
static int bat_value_table[37]={
0,  //0    
700,//0
717,//5
729,//10
734,//15
735,//16
737,//18
739,//20
741,//23
743,//26
745,//29
747,//32
748,//35
749,//37
751,//40
753,//43
755,//46
757,//49
758,//51
761,//54
763,//57
767,//60
770,//63
774,//66
777,//68
781,//71
785,//74
788,//77
791,//80
798,//83
804,//85
808,//88
812,//91
821,//95
825,//97
834,//100
836 //100
};

static int bat_charge_value_table[37]={
0,  //0    
713,//0
730,//5
741,//10
747,//15
748,//16
750,//18
752,//20
754,//23
756,//26599,//29
758,//32
759,//35
760,//37
762,//40
766,//43
768,//46
770,//49
771,//51
774,//54
776,//57
780,//60
783,//63
787,//66
790,//68
794,//71
798,//74
801,//77
804,//80
812,//83
817,//85
821,//88
825,//91
834,//95
838,//97
840,//100
843 //100
};

static int bat_level_table[37]={
0,
0,
5,
10,
15,
16,
18,
20,
23,
26,
29,
32,
35,
37,
40,
43,
46,
49,
51,
54,
57,
60,
63,
66,
68,
71,
74,
77,
80,
83,
85,
88,
91,
95,
97,
100,
100  
};

static void set_charging_mode(int flags)
{
    //GPIOD_22 low: fast charge high: slow charge
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1<<18));
    if(flags == 1)
    {
        set_gpio_val(GPIOD_bank_bit2_24(22), GPIOD_bit_bit2_24(22), 0); //fast charge
    }
    else
    {
        set_gpio_val(GPIOD_bank_bit2_24(22), GPIOD_bit_bit2_24(22), 1);	//slow charge
    }
    set_gpio_mode(GPIOD_bank_bit2_24(22), GPIOD_bit_bit2_24(22), GPIO_OUTPUT_MODE);
}

static inline int get_bat_vol(void)
{
    return get_adc_sample(5);
}

static inline int get_charging_status()
{
    return (READ_CBUS_REG(ASSIST_HW_REV)&(1<<8))? 1:0;
}

int get_charging_percentage(void)
{
    int adc_val, i;
    adc_val = get_bat_vol();
    //printf("adc_val: %d\n", adc_val);
    for(i=0; i<36; i++)
    {
        if(adc_val <= bat_charge_value_table[i])
        {
            return(bat_level_table[i]);
        }
    }
    return  100;
}

int get_battery_percentage(void)
{
    int adc_val, i;
    adc_val = get_bat_vol();
    printf("adc_val: %d\n", adc_val);
    for(i=0; i<36; i++)
    {
        if(adc_val <= bat_value_table[i])
        {
            return(bat_level_table[i]);
        }
    }
    return  100;
}

inline int any_key_pressed()
{
    int adc_val = get_adc_sample(4);
    printf("get_adc_sample(4): 0x%x\n", adc_val);
    return((((adc_val >= 0x40) && (adc_val < 0x3c0)) | powerkey_scan()) ? 1 : 0);
}

#define bat_pic_x   230
#define bat_pic_y    100
#define bat_pic_num   5
#define bat_pic_start   0x4000000
#define bat_pic_size    0x100000

int battery_charging(void)
{
    char	str[128];
    int i = 0, first_up = 1;
    int sleep_val = 0;
    int tmp=-1, charging_percentage=-2;
#ifdef	CONFIG_UBI_SUPPORT    
    run_command ("ubi part system", 0);    
    run_command ("ubifsmount system", 0);
    //run_command ("ubifsls", 0);
#endif

    set_charging_mode(1);

    for(i=0; i<26; i++)
    {
        lcd_printf("\n");
    }
    for(i=0; i<52; i++)
    {
        lcd_printf(" ");
    }
    i = 0;
    while(1)
    {
        if(!is_ac_connected)
        {
            printf("Ac adapter removed!!!\n");
            power_unhold();
            printf("Power Down!\n");
            hang();
        }
        if(any_key_pressed())
        {
            if(sleep_val >= 7)
            {
                run_command("lcd bl on", 0);
            }
            sleep_val = 0;
        }
        if(powerkey_hold(1000))
        {
            return  0;
        }
        if(sleep_val++ < 7)
        {
            if(get_charging_status())
            {
#ifdef	CONFIG_UBI_SUPPORT
                sprintf(str, "ubifsload ${loadaddr} resource/battery_pic/%d.bmp", (bat_pic_num-1));
#else
                sprintf(str, "nand read ${loadaddr} %x %x",
                    bat_pic_start+bat_pic_size*(bat_pic_num-1), bat_pic_size);
#endif
                run_command(str, 0);
                sprintf(str, "bmp display ${loadaddr} %d %d", bat_pic_x, bat_pic_y);
                run_command(str, 0);
                if(first_up)
                {
                    run_command ("lcd bl on", 0);
                    first_up = 0;
                }
                if(tmp != 100)
                {
                    tmp = charging_percentage = 100;
                    //lcd_printf("\b\b\b\b%3d% ", charging_percentage);
                }
            }
            else
            {
#ifdef	CONFIG_UBI_SUPPORT
                sprintf(str, "ubifsload ${loadaddr} resource/battery_pic/%d.bmp", i);
#else
                sprintf(str, "nand read ${loadaddr} %x %x",
                    bat_pic_start+bat_pic_size*i, bat_pic_size);
#endif
                run_command(str, 0);
                sprintf(str, "bmp display ${loadaddr} %d %d", bat_pic_x, bat_pic_y);
                run_command(str, 0);
                if(first_up)
                {
                    run_command ("lcd bl on", 0);
                    first_up = 0;
                }
                
                i = (i+1)%bat_pic_num;

                charging_percentage = get_charging_percentage();
                if(tmp != charging_percentage)
                {
                    tmp = charging_percentage;
                    //lcd_printf("\b\b\b\b%3d% ", charging_percentage);
                }
            }
            if(sleep_val >= 7)
            {
                tmp = -1;
                run_command("lcd bl off", 0);
            }
        }
        if(sdelay(1))
        {
            return  1;
        }
    }
}

void power_low_display(void)
{
    char	str[128];

#ifdef	CONFIG_UBI_SUPPORT
    run_command ("ubi part system", 0);    
    run_command ("ubifsmount system", 0);
    //run_command ("ubifsls", 0);
    sprintf(str, "ubifsload ${loadaddr} resource/battery_pic/power_low.bmp");
#else
    sprintf(str, "nand read ${loadaddr} %x %x",
        bat_pic_start+bat_pic_size*bat_pic_num, bat_pic_size);
#endif
    run_command(str, 0);
    sprintf(str, "bmp display ${loadaddr} %d %d", bat_pic_x, bat_pic_y);
    run_command(str, 0);
    run_command ("lcd bl on", 0);
}

