#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>

extern char * strpbrk(const char *,const char *);
extern char * strsep(char **,const char *);
extern size_t strspn(const char *,const char *);
extern char * strrchr(const char * s, int c);
extern char * strchr(const char * s, int c);
extern void * memcpy(void *, const void *, size_t);
extern void * memmove(void *, const void *, size_t);
extern void * memchr(const void *, int, size_t);
extern void * memset(void *, int, size_t);
extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern char * strcat(char *, const char *);
extern char * strncat(char *, const char *, size_t);
extern int strcmp(const char *,const char *);
extern int strncmp(const char *,const char *,size_t);
extern char * strstr(const char *,const char *);
extern size_t strlen(const char *);
extern size_t strnlen(const char *,size_t);
extern void * memscan(void *,int,size_t);
extern int memcmp(const void *,const void *,size_t);

#endif /* _STRING_H_ */
