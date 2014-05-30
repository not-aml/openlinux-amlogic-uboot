/*
 * \file        cmd_imgread.c
 * \brief       command to read the actual size of boot.img/recovery.img and logo.img
 *
 * \version     1.0.0
 * \date        2013/10/29
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2013 Amlogic Inc.. All Rights Reserved.
 *
 */
#include <common.h>
#include <amlogic/storage_if.h>

#define debugP(fmt...) //printf("L%d:", __LINE__),printf(fmt)
#define errorP(fmt...) printf("Err imgread(L%d):", __LINE__),printf(fmt)
#define wrnP(fmt...)   printf("wrn:"fmt)
#define MsgP(fmt...)   printf("[imgread]"fmt)

#define IMG_PRELOAD_SZ  (1U<<20) //Total read 1M at first to read the image header
#define RES_OLD_FMT_READ_SZ (8U<<20)

#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#define DTB_PRELOAD_SZ  (4U<<10) //Total read 4k at first to read the image header

#if defined(CONFIG_ANDROID_IMG)
#define SECURE_BOOT_MAGIC_SZ    16
#define SECURE_IMG_HDR_MAGIC    "AMLSECU!"
#define SECURE_IMG_HDR_VESRION  (0x0801)

#pragma pack(push, 4)
typedef struct _encrypt_boot_img_info
{
    unsigned char magic[SECURE_BOOT_MAGIC_SZ];    //magic to identify whether it is a encrypted boot image

    unsigned int  version;                  //version for this header struct

    unsigned int  totalLenAfterEncrypted;   //totaol length after encrypted with AMLETool (not include the 2K header)

    unsigned char unused[1024 - SECURE_BOOT_MAGIC_SZ - 2 * sizeof(unsigned int)];

}AmlEncryptBootImgInfo;

typedef struct _boot_img_hdr_secure_boot
{
    boot_img_hdr            normalHead;
    unsigned char           reserve4Other[1024 - sizeof(boot_img_hdr)];

    AmlEncryptBootImgInfo   encrypteImgInfo;
}AmlSecureBootImgHeader;

#pragma pack(pop)

/*#define COMPILE_TYPE_CHK(expr, t)       typedef char t[(expr) ? 1 : -1]*/
/*COMPILE_TYPE_CHK(2048  == sizeof(AmlSecureBootImgHeader), _cc);*/

static unsigned _aml_get_secure_boot_kernel_size(const void* pLoadaddr)
{
    const AmlSecureBootImgHeader* amlSecureBootImgHead = (const AmlSecureBootImgHeader*)pLoadaddr;
    const AmlEncryptBootImgInfo*  amlEncrypteBootimgInfo = &amlSecureBootImgHead->encrypteImgInfo;
    int rc = 0;
    unsigned secureKernelImgSz = 0;

    rc = !memcmp(SECURE_IMG_HDR_MAGIC, amlEncrypteBootimgInfo->magic, 8) 
            && (SECURE_IMG_HDR_VESRION == amlEncrypteBootimgInfo->version);
    if(rc){
        secureKernelImgSz = amlEncrypteBootimgInfo->totalLenAfterEncrypted;
        MsgP("Secure kernel sz 0x%x\n", secureKernelImgSz);
    }

    return secureKernelImgSz;
}

#else
static unsigned _aml_get_secure_boot_kernel_size(const void* pLoadaddr)
{
    return 0;
}
#endif// #if defined(CONFIG_ANDROID_IMG)

#ifndef CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2
static int do_image_read_dtb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned    kernel_size;
    unsigned    ramdisk_size;
    boot_img_hdr *hdr_addr = NULL;
    int genFmt = 0;
    unsigned actualBootImgSz = 0;
    unsigned dtbSz = 0;
    const char* const partName = argv[1];
    unsigned char* loadaddr = 0;
    int rc = 0;
    const int PreloadSz = DTB_PRELOAD_SZ;
    unsigned char* dtbLoadAddr = 0;
    int DtbStartOffset = 0;
    unsigned char* DtbDestAddr = (unsigned char*)CONFIG_DTB_LOAD_ADDR;
    unsigned secureKernelImgSz = 0;

    debugP("argc %d, argv:%s, %s, %s\n", argc, argv[0], argv[1], argv[2]);
    if(2 < argc){
        loadaddr = (unsigned char*)simple_strtoul(argv[2], NULL, 16);
    }
    else{
        loadaddr = (unsigned char*)simple_strtoul(getenv("loadaddr"), NULL, 16);
    }
    hdr_addr = (boot_img_hdr*)loadaddr;

    if(argc > 3){
        DtbDestAddr = (unsigned char*)simple_strtoul(argv[3], NULL, 16);
    }

    rc = store_read_ops((unsigned char*)partName, loadaddr, 0, PreloadSz);
    if(rc){
        errorP("Fail to read 0x%xB from part[%s] at offset 0\n", PreloadSz, partName);
        return __LINE__;
    }

    genFmt = genimg_get_format(hdr_addr);
    if(IMAGE_FORMAT_ANDROID != genFmt) {
        errorP("Fmt unsupported!genFmt 0x%x != 0x%x\n", genFmt, IMAGE_FORMAT_ANDROID);
        return __LINE__;
    }

    kernel_size     =(hdr_addr->kernel_size + (hdr_addr->page_size-1)+hdr_addr->page_size)&(~(hdr_addr->page_size -1));
    ramdisk_size    =(hdr_addr->ramdisk_size + (hdr_addr->page_size-1))&(~(hdr_addr->page_size -1));
    dtbSz           = hdr_addr->second_size;
    DtbStartOffset = kernel_size + ramdisk_size;
    actualBootImgSz = DtbStartOffset + dtbSz;
    debugP("kernel_size 0x%x, page_size 0x%x, totalSz 0x%x\n", hdr_addr->kernel_size, hdr_addr->page_size, kernel_size);
    debugP("ramdisk_size 0x%x, totalSz 0x%x\n", hdr_addr->ramdisk_size, ramdisk_size);
    debugP("dtbSz 0x%x\n", dtbSz);
    dtbLoadAddr = loadaddr + DtbStartOffset;//default dtb load size

    secureKernelImgSz = _aml_get_secure_boot_kernel_size(loadaddr);
    if(secureKernelImgSz)
    {
        if(actualBootImgSz > PreloadSz)
        {
            rc = store_read_ops((unsigned char*)partName, loadaddr + PreloadSz, PreloadSz, secureKernelImgSz - PreloadSz);
            if(rc){
                errorP("Fail in store read: part(%s), buf(0x%p), offset(0x%x), size(0x%x)\n", 
                        partName, loadaddr + PreloadSz, PreloadSz, secureKernelImgSz - PreloadSz);
                return __LINE__;
            }
        }

#ifdef CONFIG_AML_SECU_BOOT_V2
#ifdef CONFIG_MESON_TRUSTZONE
        extern int meson_trustzone_boot_check(unsigned char *addr);
        rc = meson_trustzone_boot_check(loadaddr + 2048);
#else
        extern int aml_sec_boot_check(unsigned char *pSRC);
        rc = aml_sec_boot_check(loadaddr + 2048);
#endif
        if(rc){
            errorP("Fail when sec_check, rc=%d\n", rc);
            return __LINE__;
        }
#endif//#ifdef CONFIG_AML_SECU_BOOT_V2
    }
    else
    {
        if(actualBootImgSz > PreloadSz)
        {
            dtbLoadAddr = loadaddr + PreloadSz;//Just load PreloadSz + dtbSz
            rc = store_read_ops((unsigned char*)partName, dtbLoadAddr, DtbStartOffset, dtbSz);
            if(rc){
                errorP("Fail to read 0x%xB from part[%s] at offset 0x%x\n", dtbSz, partName, DtbStartOffset);
                return __LINE__;
            }
        }
    }

    memcpy(DtbDestAddr, dtbLoadAddr, dtbSz);
    if(fdt_check_header(DtbDestAddr)){
        errorP("dtb head check failed\n");
        return __LINE__;
    }

    return 0;
}
#else
static int do_image_read_kernel(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

static int do_image_read_dtb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char* loadaddr = 0;
    int rc = 0;
    const char* loadaddrStr = NULL;
    unsigned char* DtbDestAddr = (unsigned char*)CONFIG_DTB_LOAD_ADDR;
    unsigned char* dtbLoadAddr = 0;
    boot_img_hdr *hdr_addr = NULL;
    int genFmt = 0;
    unsigned    kernel_size = 0;
    unsigned    ramdisk_size = 0;
    unsigned    DtbStartOffset = 0;
    unsigned dtbSz = 0;

    debugP("%s, %s, %s\n", argv[0], argv[1], argv[2]);
    loadaddrStr = (2 < argc) ? argv[2] : getenv("loadaddr");
    loadaddr = (unsigned char*)simple_strtoul(loadaddrStr, NULL, 16);
    hdr_addr = (boot_img_hdr*)loadaddr;

    rc = do_image_read_kernel(cmdtp, flag, argc, argv);
    if(rc){
        errorP("Fail to read kernel img for dtb, rc=%d\n", rc);
        return __LINE__;
    }

#ifdef CONFIG_MESON_TRUSTZONE
	extern int meson_trustzone_boot_check(unsigned char *addr);
	rc = meson_trustzone_boot_check((unsigned char *)loadaddr);
#else
	extern int aml_sec_boot_check(unsigned char *pSRC);
	rc = aml_sec_boot_check((unsigned char *)loadaddr);
#endif
	if(rc){
        errorP("Fail when sec_check, rc=%d\n", rc);
        return __LINE__;
    }

    genFmt = genimg_get_format(hdr_addr);
    if(IMAGE_FORMAT_ANDROID != genFmt) {
        errorP("Fmt unsupported!genFmt 0x%x != 0x%x\n", genFmt, IMAGE_FORMAT_ANDROID);
        return __LINE__;
    }

    kernel_size     =(hdr_addr->kernel_size + (hdr_addr->page_size-1)+hdr_addr->page_size)&(~(hdr_addr->page_size -1));
    ramdisk_size    =(hdr_addr->ramdisk_size + (hdr_addr->page_size-1))&(~(hdr_addr->page_size -1));
    dtbSz           = hdr_addr->second_size;
    DtbStartOffset = kernel_size + ramdisk_size;
    debugP("kernel_size 0x%x, page_size 0x%x, totalSz 0x%x\n", hdr_addr->kernel_size, hdr_addr->page_size, kernel_size);
    debugP("ramdisk_size 0x%x, totalSz 0x%x\n", hdr_addr->ramdisk_size, ramdisk_size);
    debugP("dtbSz 0x%x\n", dtbSz);
    dtbLoadAddr =   loadaddr + DtbStartOffset;

    if(fdt_check_header(dtbLoadAddr)){
        errorP("dtb head check failed\n");
        return __LINE__;
    }
    if((1U<<20) < dtbSz){
        errorP("dtb size 0x%x > max 1M\n", dtbSz);
        return __LINE__;
    }
    memcpy(DtbDestAddr, dtbLoadAddr, dtbSz);

    return 0;
}
#endif//#ifndef CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2

#else
static int do_image_read_dtb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    errorP("unsupported as CONFIG_OF_LIBFDT undef\n");
    return __LINE__;
}
#endif//#ifdef CONFIG_OF_LIBFDT

#ifndef CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2
static int do_image_read_kernel(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned    kernel_size;
    unsigned    ramdisk_size;
    boot_img_hdr *hdr_addr = NULL;
    int genFmt = 0;
    unsigned actualBootImgSz = 0;
    unsigned dtbSz = 0;
    const char* const partName = argv[1];
    unsigned char* loadaddr = 0;
    int rc = 0;
    uint64_t flashReadOff = 0;
    unsigned secureKernelImgSz = 0;
    
    if(2 < argc){
        loadaddr = (unsigned char*)simple_strtoul(argv[2], NULL, 16);
    }
    else{
        loadaddr = (unsigned char*)simple_strtoul(getenv("loadaddr"), NULL, 16);
    }
    hdr_addr = (boot_img_hdr*)loadaddr;

    rc = store_read_ops((unsigned char*)partName, loadaddr, flashReadOff, IMG_PRELOAD_SZ);
    if(rc){
        errorP("Fail to read 0x%xB from part[%s] at offset 0\n", IMG_PRELOAD_SZ, partName);
        return __LINE__;
    }
    flashReadOff = IMG_PRELOAD_SZ;

    genFmt = genimg_get_format(hdr_addr);
    if(IMAGE_FORMAT_ANDROID != genFmt) {
        errorP("Fmt unsupported!genFmt 0x%x != 0x%x\n", genFmt, IMAGE_FORMAT_ANDROID);
        return __LINE__;
    }

    //Check if encrypted image
    secureKernelImgSz = _aml_get_secure_boot_kernel_size(loadaddr);
    if(secureKernelImgSz)
    {
        actualBootImgSz = secureKernelImgSz;
    }
    else
    {
        kernel_size     =(hdr_addr->kernel_size + (hdr_addr->page_size-1)+hdr_addr->page_size)&(~(hdr_addr->page_size -1));
        ramdisk_size    =(hdr_addr->ramdisk_size + (hdr_addr->page_size-1))&(~(hdr_addr->page_size -1));
        dtbSz           = hdr_addr->second_size;
        actualBootImgSz = kernel_size + ramdisk_size + dtbSz;
        debugP("kernel_size 0x%x, page_size 0x%x, totalSz 0x%x\n", hdr_addr->kernel_size, hdr_addr->page_size, kernel_size);
        debugP("ramdisk_size 0x%x, totalSz 0x%x\n", hdr_addr->ramdisk_size, ramdisk_size);
        debugP("dtbSz 0x%x, Total actualBootImgSz 0x%x\n", dtbSz, actualBootImgSz);
    }

    if(actualBootImgSz > IMG_PRELOAD_SZ)
    {
        const unsigned leftSz = actualBootImgSz - IMG_PRELOAD_SZ;

        debugP("Left sz 0x%x\n", leftSz);
        rc = store_read_ops((unsigned char*)partName, loadaddr + (unsigned)flashReadOff, flashReadOff, leftSz);
        if(rc){
            errorP("Fail to read 0x%xB from part[%s] at offset 0x%x\n", leftSz, partName, IMG_PRELOAD_SZ);
            return __LINE__;
        }
    }
    debugP("totalSz=0x%x\n", actualBootImgSz);

    return 0;
}
#else
//To read the whole partition size if CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2 defined
static int do_image_read_kernel(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char* partName = (unsigned char*)argv[1];
    unsigned char* loadaddr = 0;
    int rc = 0;
    uint64_t flashReadOff = 0;
    const char* loadaddrStr = (2 < argc) ? argv[2] : getenv("loadaddr");
    u64 partSz = 0;

    loadaddr = (unsigned char*)simple_strtoul(loadaddrStr, NULL, 16);

    debugP("%s, %s, %s\n", argv[0], argv[1], argv[2]);
    rc = store_get_partititon_size(partName, &partSz);
    if(rc || !partSz){
        errorP("Fail to get capacity for part %s, partSz %lldSec\n", partName, partSz);
        return __LINE__;
    }
    partSz <<= 9;//trans sector to byte
    printf("part[%s] sz %lldMB\n", partName, partSz>>20);
    rc = store_read_ops(partName, loadaddr, flashReadOff, partSz);
    if(rc){
        errorP("Fail to read 0x%xB from part[%s] at offset 0\n", IMG_PRELOAD_SZ, partName);
        return __LINE__;
    }

    return 0;
}
#endif//#ifndef CONFIG_CMD_IMGREAD_FOR_SECU_BOOT_V2

#define AML_RES_IMG_VERSION         0x01
#define AML_RES_IMG_V1_MAGIC_LEN    8
#define AML_RES_IMG_V1_MAGIC        "AML_RES!"//8 chars
#define AML_RES_IMG_ITEM_ALIGN_SZ   16
#define AML_RES_IMG_HEAD_SZ         (AML_RES_IMG_ITEM_ALIGN_SZ * 4)//64
#define AML_RES_ITEM_HEAD_SZ        (AML_RES_IMG_ITEM_ALIGN_SZ * 4)//64

//typedef for amlogic resource image
#pragma pack(push, 4)
typedef struct {
    __u32   crc;    //crc32 value for the resouces image
    __s32   version;//current version is 0x01

    __u8    magic[AML_RES_IMG_V1_MAGIC_LEN];  //resources images magic

    __u32   imgSz;  //total image size in byte
    __u32   imgItemNum;//total item packed in the image

    __u32   alignSz;//AML_RES_IMG_ITEM_ALIGN_SZ
    __u8    reserv[AML_RES_IMG_HEAD_SZ - 8 * 3 - 4];

}AmlResImgHead_t;
#pragma pack(pop)

#define LOGO_OLD_FMT_READ_SZ (8U<<20)//if logo format old, read 8M

static int do_image_read_res(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    const char* const partName = argv[1];
    unsigned char* loadaddr = 0;
    int rc = 0;
    AmlResImgHead_t* pResImgHead = NULL;
    unsigned totalSz    = 0;
    uint64_t flashReadOff = 0;

    if(2 < argc){
        loadaddr = (unsigned char*)simple_strtoul(argv[2], NULL, 16);
    }
    else{
        loadaddr = (unsigned char*)simple_strtoul(getenv("loadaddr"), NULL, 16);
    }
    pResImgHead = (AmlResImgHead_t*)loadaddr;

    rc = store_read_ops((unsigned char*)partName, loadaddr, flashReadOff, IMG_PRELOAD_SZ);
    if(rc){
        errorP("Fail to read 0x%xB from part[%s] at offset 0\n", IMG_PRELOAD_SZ, partName);
        return __LINE__;
    }
    flashReadOff = IMG_PRELOAD_SZ;

    rc = memcmp(pResImgHead->magic, AML_RES_IMG_V1_MAGIC, AML_RES_IMG_V1_MAGIC_LEN);
    if(rc)
    {
        wrnP("Magic error, read old version in size 0x%x\n", RES_OLD_FMT_READ_SZ);

        rc = store_read_ops((unsigned char*)partName, loadaddr + (unsigned)flashReadOff, flashReadOff, RES_OLD_FMT_READ_SZ - flashReadOff);
        return rc;
    }

    //Read the actual size of the new version res imgae
    totalSz = pResImgHead->imgSz;
    if(totalSz > IMG_PRELOAD_SZ )
    {
        const unsigned leftSz = totalSz - flashReadOff;

        rc = store_read_ops((unsigned char*)partName, loadaddr + (unsigned)flashReadOff, flashReadOff, leftSz);
        if(rc){
            errorP("Fail to read 0x%xB from part[%s] at offset 0x%x\n", leftSz, partName, IMG_PRELOAD_SZ);
            return __LINE__;
        }
    }
    debugP("totalSz=0x%x\n", totalSz);

    return 0;
}

static cmd_tbl_t cmd_imgread_sub[] = {
	U_BOOT_CMD_MKENT(dtb,    4, 0, do_image_read_dtb, "", ""),
	U_BOOT_CMD_MKENT(kernel, 3, 0, do_image_read_kernel, "", ""),
	U_BOOT_CMD_MKENT(res,    3, 0, do_image_read_res, "", ""),
};

static int do_image_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	/* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_imgread_sub[0], ARRAY_SIZE(cmd_imgread_sub));

	if (c) {
		return	c->cmd(cmdtp, flag, argc, argv);
	} else {
		cmd_usage(cmdtp);
		return 1;
	}
}

U_BOOT_CMD(
   imgread,         //command name
   5,               //maxargs
   0,               //repeatable
   do_image_read,   //command function
   "Read the image from internal flash with actual size",           //description
   "    argv: <imageType> <part_name> <loadaddr> \n"   //usage
   "    - <image_type> Current support is kernel/res(ource).\n"
   "imgread kernel  --- Read image in fomart IMAGE_FORMAT_ANDROID\n"
   "imgread kernel  --- Load dtb from image boot.img or recovery.img\n"
   "imgread resouce --- Read image packed by 'Amlogic resource packer'\n"
   "    - e.g. \n"
   "        to read boot.img     from part boot     from flash: <imgread kernel boot loadaddr> \n"   //usage
   "        to read dtb          from part boot     from flash: <imgread dtb boot loadaddr [destAddr]> \n"   //usage
   "        to read recovery.img from part recovery from flash: <imgread kernel recovery loadaddr> \n"   //usage
   "        to read logo.img     from part logo     from flash: <imgread res    logo loadaddr> \n"   //usage
);

