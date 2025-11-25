#include<stdio.h>
#include<errno.h>
#include<string.h>

int main(int argc,char *argv[]){
	FILE *fp = fopen(argv[1],"r");
	if(fp==NULL){
#if 0
		printf("fopen failed...%d\n",errno);
		printf("fopen failed...%s\n",strerror(errno));
#endif
		perror("fopen");
		return -1;
	}
	printf("fopen succeed...\n");
	fclose(fp);
	return 0;
}
