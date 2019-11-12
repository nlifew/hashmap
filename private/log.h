

#ifndef _UTIL_LOG_H
#define _UTIL_LOG_H 1

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LOGI(...)   printf(__VA_ARGS__)
#define LOGV(...)   printf(__VA_ARGS__)
#define LOGD(...)   printf(__VA_ARGS__)
#define LOGW(...)   fprintf(stderr, __VA_ARGS__)
#define LOGE(...)   fprintf(stderr, __VA_ARGS__); \
fprintf(stderr, "%s[%d]\n", strerror(errno), errno);


#endif