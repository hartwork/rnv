#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv) {
        double d;
        d=atof(*(argv+1));
        printf("%f\n",d);
        return 0;
}
