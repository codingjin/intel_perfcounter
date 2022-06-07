#include "example.h"

int main()
{
        long int test[100000000000000] = {0};
        for(long int i=1; i<100000000000000; i++)
        {
                test[i] =1+ test[i-1];
        }
        pebs_init();
        for(long int i=1; i<100000000000000; i++)
        {
                test[i] =1+ test[i-1];
        }
        return 0;
        pebs_shutdown();
}

// compile the program, gcc -g -Wall main.c example.c -O3 -fPIC -lm -lpthread -lpfm
// run the program with root access. E.g., sudo ./a.out