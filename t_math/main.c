#include<stdio.h>
#include "p_math.h"

int main(){
	int val_x = 6,val_y = 2;
	printf("%d+%d=%d\n",val_x,val_y,t_add(val_x,val_y));
	printf("%d*%d=%d\n",val_x,val_y,t_mul(val_x,val_y));
	return 0;
}
