#include "hello_util.h"
#include <stdio.h>
#include "sdkconfig.h"

void hi(void) { printf("hi %s\n", CONFIG_HELLO_NAME); }

void hello(void) { printf("hello %s\n", CONFIG_HELLO_NAME); }

