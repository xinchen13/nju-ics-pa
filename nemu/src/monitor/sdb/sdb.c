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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

#ifndef CONFIG_DEBUG_EXPR
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}
#endif

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  // extract the first argument
  char *step_count_str = strtok(NULL, " ");
  // default: step = 1 
  int step_count = 1;
  // update step
  if (step_count_str != NULL) {
    sscanf(step_count_str, "%d", &step_count);
  }
  // execute 
  cpu_exec(step_count);
  return 0;
}

static int cmd_info(char *args) {
  // extract the first argument
  char *info_type = strtok(NULL, " ");
  if (info_type != NULL) {
    if (strcmp(info_type, "r") == 0) {
      isa_reg_display();
    }
    else if (strcmp(info_type, "w") == 0) {
      watchpoint_display();
    }
    else {
      printf("Please offer a valid info type: \"r\" or \"w\"\n");
    }
  }
  else {
    printf("Please offer a valid info type: \"r\" or \"w\"\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  // extract the first argument
  char *count_str = strtok(NULL, " ");
  // extract the second argument
  char *base_addr_str = strtok(NULL, " ");
  // get the 4-byte memory count to scan 
  int count = 1;
  if (count_str != NULL) {
    sscanf(count_str, "%d", &count);
  }
  // get the base memory
  word_t base_addr = 0x80000000;
  bool eval_success = true;
  if (base_addr_str != NULL) {
    base_addr = expr(base_addr_str, &eval_success);
    if (eval_success){
      // print the value by vaddr_read()
      word_t addr;
      for (int i = 0; i < count; i++) {
        addr = base_addr + i * 4;
        printf("mem[" FMT_WORD "] = " FMT_WORD "\n", addr, vaddr_read(addr, 4));
      }
    }
    else {
      printf("Invalid address!\n");
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  bool success = true; // default: true
  // call expr() in monitor/sdb/expr.c
  word_t result = expr(args, &success);
  if (success){
    printf("%s = %d = " FMT_WORD "\n", args, result, result);
  }
  else {
    printf("Fail!!!! Check the expression!!\n");
  }
  return 0;
}

static int cmd_w(char *args) {
  bool success = true;
  if (args != NULL) {
    WP* wp = new_wp(args, &success);
    if (success) {
      printf("watchpoint %d: %s\n", wp->NO, wp->WP_expr);
    }
    else {
      printf("failed to set watchpoint!!!!\n");
      // if expr() in new_wp() fails, free the wp allocated
      free_wp(wp->NO);
    }
    return 0;
  }
  printf("please offer the expr to watch!!!!\n");
  return 0;
}

static int cmd_d(char *args) {
  int num = 0;
  if (args != NULL) {
    // get num and free
    sscanf(args, "%d", &num);
    free_wp(num);
  }
  else {
    printf("enter the number of wp to del it~\n");
  }
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Single step execution: [si N]", cmd_si },
  { "info", "Print program status: [info r / info w]", cmd_info },
  { "x", "Scan the memory: [x N expr]", cmd_x },
  { "p", "Expression evaluation: [p EXPR]", cmd_p },
  { "w", "Set watchpoint: [w expr]", cmd_w },
  { "d", "Delete watchpoint by number: [d N]", cmd_d },
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

#ifdef CONFIG_DEBUG_EXPR // run gen-expr test in nemu
// run this debug: $ make run > tools/gen-expr/build/result
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *result_str = NULL;
  unsigned int result_ref = 0;
  char *args = NULL;

  // open the file 
  fp = fopen("./tools/gen-expr/build/input", "r");
  if (fp == NULL)
      exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) {
      // printf("retrieved line of length %zu:\n", read);
      // printf("%s", line);

      // get the standard result to compare with
      result_str = strtok(line, " ");
      sscanf(result_str, "%u", &result_ref);
      // get the expression
      args = strtok(NULL, "\n");
      printf("expr: %s", args);
      // expression evaluation
      bool success = true;
      word_t result = expr(args, &success);
      // output the result
      if (success){
        if(result_ref == result){
          printf(" = %u(ref result) = %u(my nemu result), ", result_ref, result);
          printf("PASS!\n");
        }
        else{
          printf(" = %u(ref result) != %u(my nemu result), ", result_ref, result);
          exit(EXIT_FAILURE);
        }
      }
      else {
        printf("Invalid expression!\n");
      }
  }
  // close the file 
  fclose(fp);
  if (line)
      free(line);
  exit(EXIT_SUCCESS);
}
#else // run sdb in nemu
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
#endif

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
