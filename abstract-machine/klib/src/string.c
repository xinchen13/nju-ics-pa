#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // return the length of the null-terminated string s
  size_t len = 0;
  while (*s != '\0') {
    len++;
    s++;
  }
  return len;
  // panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  // copy src to dest, returning the address of the terminating '\0' in dest
    char *dst_addr = dst; // save the address
    while (*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0'; // teminating sign
    return dst_addr;
  // panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  // at most n bytes of src are copied
  // if there is no null byte among the first n  bytes of src, 
  // the string placed in dest will not be null-terminated.
  // if the length of src is less than n, strncpy() writes additional null
  // bytes to dest to ensure that a total of n bytes are written.
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++)
      dst[i] = src[i];
  for ( ; i < n; i++)
      dst[i] = '\0';
  return dst;
  // panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  // appends the src string to the dest string, over‐writing 
  // the terminating null byte ('\0') at the end of dest, 
  // and then adds a terminating null byte
  char *dst_addr = dst;
  while (*dst != '\0') {
    dst++;
  }
  while (*src != '\0') {
    *dst = *src;
    src++;
    dst++;
  }
  *dst = '\0';
  return dst_addr;
  // panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
    s1++;
    s2++;
  }
  return (unsigned char)(*s1) - (unsigned char)(*s2);
  // panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int i = 0;
  while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0' && i < n-1 ) {
    i++;
  }
  return (unsigned char)(s1[i]) - (unsigned char)(s2[i]);
  // panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  // fill the first n bytes of the memory area
  // pointed to by s with the constant byte c.
  unsigned char *p = s;
  while (n > 0) {
      *p = (unsigned char)c;
      p++;
      n--;
  }
  return s; 
  // panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  // copies n bytes from memory area src to memory area dest
  // the memory areas may overlap
  // warning!!!!!!!!!!!!: n < 256
  unsigned char tmp[256];
  unsigned char *d = dst;
  const unsigned char *s = src;
  int i = 0;
  int j = 0;
  while (i < n) {
    tmp[i] = *s;
    s++;
    i++;
  }
  while (j < n) {
    *d = tmp[j];
    d++;
    j++;
  }
  return 0;
  // panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  // copies n bytes from memory area src to memory area dest
  // the memory areas must not overlap
  unsigned char *d = out;
  const unsigned char *s = in;
  while (n > 0) {
    *d = *s;
    d++;
    s++;
    n--;
  }
  return 0;
  // panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  // compares the first n bytes (each interpreted as unsigned char) 
  // of the memory areas s1 and s2.
  const unsigned char *p = s1;
  const unsigned char *q = s2;
  while (n > 0 && *p == *q) {
    p++;
    q++;
    n--;
  }
  if (n == 0) {
    return 0;
  }
  else {
    return (int)(*p - *q);
  }
  // panic("Not implemented");
}

#endif
