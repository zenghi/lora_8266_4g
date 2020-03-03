#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define MAX 50
int main()
{
int i;    
	FILE *fp;
	if ((fp = fopen("test.txt", "w")) == NULL);
	{
		printf("文件开始写入\n");
	}
    for(i=5000;i<7100;i++)
//        fprintf(fp,"021808030000%x,%d,%d\n ",i,i,i%10);
fprintf(fp,"021808030000%04x,%04d,%d\n",i,i,i%10);

	printf("写入成功\n");

	fclose(fp);
	printf("文件排序完毕结果请看文件\n");    

return 0;
}
