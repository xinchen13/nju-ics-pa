/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char buf_expr[65536] = {}; //  for nemu
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

// get random number in [0, n)
uint32_t choose(uint32_t n){
  return rand() % n;
}

// generate a random decimal number (unsigned, e.g. 13u)
void gen_num(){
  char tmp[20];
  tmp[0] = '\0';
  uint32_t number = choose(100); // 0-99
  sprintf(tmp ,"%u", number);
  strcat(tmp, "u");
  strcat(buf, tmp);
}

// generate a string (e.g. operator)
void gen(char gen_str){
  char tmp[2] = {gen_str, '\0'};
  strcat(buf, tmp);
}

// genrate ramdom blanks
void gen_rand_blank(){
  switch(choose(3)){
    case 0:
      break;
    case 1:
      gen(' ');
      break;
    case 2:
      gen(' ');
      gen(' ');
      break;
    default:
      break;
  }
}

// genrate random operators: +, -, *, /
void gen_rand_op(){
  switch(choose(4)){
    case 0:
      gen('+');
      break;
    case 1:
      gen('-');
      break;
    case 2:
      gen('*');
      break;
    default:
      gen('/');
      break;
  }
}

// generate random expression, limit length
static void gen_rand_expr(unsigned int cnt) {
  if (strlen(buf) > 900 || cnt == 0){
    gen_num();
  }
  else{
    switch (choose(3)) {
      case 0: 
        gen_num();
        break;
      case 1: 
        gen('('); 
        gen_rand_blank();
        gen_rand_expr(cnt-1);
        gen_rand_blank(); 
        gen(')'); 
        break;
      default: 
        gen_rand_blank();
        gen_rand_expr(cnt-1);
        gen_rand_blank(); 
        gen_rand_op(); 
        gen_rand_blank();
        gen_rand_expr(cnt-1);
        gen_rand_blank();
        break;
    }
  }
}

// generate expression to nemu, drop 'u' from buf
void gen_nemu_expr(char *buf, char *buf_expr){
  int i = 0;
  int j = 0;
  while(buf[i] != '\0'){
    if(buf[i] != 'u'){
      buf_expr[j] = buf[i];
      j++;
    }
    i++;
  }
  buf_expr[j] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr(10);
    gen_nemu_expr(buf, buf_expr);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf_expr);
  }
  return 0;
}
