
#define kernel_args *((volatile unsigned long*)0xC9001E00)

typedef enum kernel_cmd
{
	NORMAL_BOOT=0,
	RECOVERY_BOOT,
}kernel_cmd_t;

kernel_cmd_t get_kernel_cmd(void);