#ifndef _STRING_H_
#define _STRING_H_

typedef unsigned size_t;
#define NULL 0

extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern int strcmp(const char *,const char *);
extern int strncmp(const char *,const char *,size_t);
extern size_t strlen(const char *);

extern void * memset(void *, int, size_t);
extern void * memcpy(void *, const void *, size_t);
extern void * memmove(void *, const void *, size_t);

#endif /* _STRING_H_ */
