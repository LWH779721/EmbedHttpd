#include <stdio.h>
#include "print_util.h"

long set_log_file(char *file)
{
    FILE *fp;
    
    if (NULL == (fp = freopen(file, "w", stdout)))
    {
        print_log("failed when reopen stdout");
        return -1;
    }
    
    return 0;
}