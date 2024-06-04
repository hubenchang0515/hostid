#include <stdio.h>
#include <hostid.h>

int main(void)
{
    const char* id = host_id();
    printf("%s\n", id);
    return 0;
}