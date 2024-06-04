# hostid
Get host unique ID - 获取主机的唯一 ID

## API

```c
/***************************
 * @brief get host unique ID - 获取主机的唯一 ID
 * @return current host unique ID - 当前主机的唯一 ID
 * @note return value is static - 返回值是 static 的
 ***************************/
const char* host_id();
```

## Example

```c
#include <stdio.h>
#include <hostid.h>

int main(void)
{
    const char* id = host_id();
    printf("%s\n", id);
    return 0;
}
```

```
$ hostid
764e7027fb13acd777e37bc6c4f7338b98d49b7ed5c2c1e8950c894948fc0ea0068cc1b4b0416d4c75ebcf9d900f2d68349c081e4dce57e0e51c5c576a4eda6b
```