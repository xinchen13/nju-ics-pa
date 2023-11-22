#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  // produce output according to a format: fmt
  // write output to the character string: out
  va_list ap;
  va_start(ap, fmt);
  char *p = out;
  int count = 0;  // the size of str

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;  // skip '%'
      switch (*fmt) {
        case 's':
          char *arg = va_arg(ap, char*);
          while (*arg) {
              *p = *arg++;
              p++;
              count++;
          }
          break;
        case 'd':
          int num = va_arg(ap, int);       
          if (num < 0) {
              *p = '-';
              p++;
              count++;
              num = -num;
          }
          int num_digits = 0;
          int temp = num;
          while (temp > 0) {
            temp /= 10;
            num_digits++;
          }
          p = p + num_digits - 1;
          while (num > 0) {
            *p = '0' + num % 10;
            p--;
            num /= 10;
          }
          p = p + num_digits + 1; 
          count += num_digits;
          break;
        default:
          *p = *fmt;
          p++;
          count++;
          break;
      }
    } 
    else {
      *p = *fmt;
      p++;
      count++;
    }
    fmt++;
  }
  *p = '\0';  // 添加字符串结束标志
  va_end(ap);
  return count;
  // panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
