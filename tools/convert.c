#include <stdio.h>
/*
 * only support M1 now
 */
static unsigned short buf[3*1024];
#define MAGIC_WORD1         0x4f53454d  /* "" */
#define MAGIC_WORD2         0x3130304e  /* "" */
static void m1_caculate(void)
{
	int i;
	unsigned short sum=0;
	unsigned * magic;
	// Calculate sum
	for(i=0;i<0x1b0/2;i++)
	{
		sum^=buf[i];
	}

	for(i=256;i<sizeof(buf)/2;i++)
	{
		sum^=buf[i];
	}
	buf[0x1b8/2]=sum;
	magic=(unsigned *)&buf[0x1b0/2];
	magic[0]=MAGIC_WORD1;
	magic[1]=MAGIC_WORD2;
}
int main(int argc,char * argv[])
{
	FILE * fp_in=NULL;
	FILE * fp_out=NULL;
	FILE * fp_spl=NULL;
	int ret=1;
	int count;
	if(argc!=5&&argc!=6)
	{
		printf("Parameter error\n");
		goto exit_prog;
	}
	if(argc==6){
		fp_spl=fopen(argv[1],"rb");
		fp_in=fopen(argv[2],"rb");
		fp_out=fopen(argv[3],"wb");
	}else{
		fp_spl=fopen(argv[1],"rb");
		fp_in=fp_spl;
		fp_out=fopen(argv[2],"wb");

	}
	if(fp_in==NULL||fp_out==NULL||fp_spl==NULL)
		goto exit_prog;
	memset(buf,0,sizeof(buf));
	fread(buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),fp_spl);
	m1_caculate();
	fwrite(buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),fp_out);
	while(!feof(fp_in))
	{
		count=fread(buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),fp_in);
		fwrite(buf,sizeof(buf[0]),count,fp_out);
	}
	ret=0;
exit_prog:
	if(fp_spl)
	{
		fclose(fp_spl);
	}else{
		printf("File open error %s",argv[1]);
	}
	if(fp_in)
	{
		if(argc==6)
			fclose(fp_in);
	}else{
		printf("File open error A %s",argv[2]);
	}
	if(fp_out)
	{
		fclose(fp_out);
	}else{
		if(argc==6)
			printf("File open error %s",argv[3]);
		else
			printf("File open error %s",argv[2]);
	}
	return ret;
}
