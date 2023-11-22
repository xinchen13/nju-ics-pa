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
#include <memory/vaddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#define TOKEN_STR_LEN_MAX 32      // the max length of tokens[].str
#define TOKENS_COUNT_MAX 1024       // the max token count in tokens[]

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_DECNUM, TK_NEG, TK_HEXNUM, TK_REG,
  TK_NE, TK_AND, TK_DEREF
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"0x[0-9A-Fa-f]+", TK_HEXNUM},  // heximal
  {"\\$[\\$0-9a-z]+", TK_REG},   // gpr
  {"([0-9]+([.][0-9]*)?|[.][0-9]+)", TK_DECNUM}, // decimal
  {" +", TK_NOTYPE},    // spaces
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"==", TK_EQ},        // equal
  {"!=", TK_NE},        // not equal
  {"&&", TK_AND},       // logical and
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[TOKEN_STR_LEN_MAX];
} Token;

static Token tokens[TOKENS_COUNT_MAX] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // debug: print the rule matched to check the regex
        #ifdef CONFIG_TOKEN_DEBUG
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        #endif
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          // ignore spaces
          case TK_NOTYPE:
            break;
          // plus
          case '+':
            tokens[nr_token].type = '+';
            nr_token++;
            break;
          // ‘-’
          case '-':
            // negative
            // 1. the first sign of the expression
            // 2. after +, -, *, /, TK_NEG, (, TK_NE, TK_EQ, TK_AND
            if (nr_token == 0 || tokens[nr_token-1].type == '(' || 
            tokens[nr_token-1].type  == '+' || tokens[nr_token-1].type  == '-' || 
            tokens[nr_token-1].type  == '*' || tokens[nr_token-1].type  == '/' ||
            tokens[nr_token-1].type  == TK_NEG || tokens[nr_token-1].type  == TK_NE ||
            tokens[nr_token-1].type  == TK_EQ || tokens[nr_token-1].type  == TK_AND) {
              tokens[nr_token].type = TK_NEG;
              nr_token++;
              break;
            }
            // minus
            else {
              tokens[nr_token].type = '-';
              nr_token++;
              break;
            }
          // '*'
          case '*':
          // deref
            if (nr_token == 0 || tokens[nr_token-1].type == '(' || 
            tokens[nr_token-1].type  == '+' || tokens[nr_token-1].type  == '-' || 
            tokens[nr_token-1].type  == '*' || tokens[nr_token-1].type  == '/' ||
            tokens[nr_token-1].type  == TK_NEG || tokens[nr_token-1].type  == TK_NE ||
            tokens[nr_token-1].type  == TK_EQ || tokens[nr_token-1].type  == TK_AND) {
              tokens[nr_token].type = TK_DEREF;
              nr_token++;
              break;
            }
          // mul
            else {
              tokens[nr_token].type = '*';
              nr_token++;
              break;
            }
          // div
          case '/':
            tokens[nr_token].type = '/';
            nr_token++;
            break;
          // left bracket
          case '(':
            tokens[nr_token].type = '(';
            nr_token++;
            break;
          // right bracket
          case ')':
            tokens[nr_token].type = ')';
            nr_token++;
            break;
          // decimal 
          case TK_DECNUM:
            tokens[nr_token].type = TK_DECNUM;
            if (substr_len >= TOKEN_STR_LEN_MAX) {
              printf("Too long decimal number!!!");
              return false;
            }
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          // heximal
          case TK_HEXNUM:
            tokens[nr_token].type = TK_HEXNUM;
            if (substr_len >= TOKEN_STR_LEN_MAX) {
              printf("Too long heximal number!!!");
              return false;
            }
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          // reg
          case TK_REG:
            tokens[nr_token].type = TK_REG;
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          // equal
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            nr_token++;
            break;
          // not equal
          case TK_NE:
            tokens[nr_token].type = TK_NE;
            nr_token++;
            break;
          // logical and
          case TK_AND:
            tokens[nr_token].type = TK_AND;
            nr_token++;
            break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  // debug: prinf the tokens to check the function of make_token()
  #ifdef CONFIG_TOKEN_DEBUG
  for(int y = 0; y < nr_token; y++){
    printf("tokens[%d].type = %d, tokens[%d].str = %s\n",y,tokens[y].type,y,tokens[y].str);
  }
  #endif

  return true;
}

// get the priority of operators
int priority(int op) {
  switch (op) { 
    case TK_AND:
      return 6;
    case TK_EQ:
    case TK_NE:
      return 8;
    case '+':
    case '-':
      return 10; 
    case '*':
    case '/':
      return 11;
    case TK_NEG:
    case TK_DEREF:
      return 15;
    default:
      return -1;
  }
}

// confirm whether the expression is surrounded by a matched pair of parentheses
bool check_parentheses(int p, int q, bool *success) {
  bool valid_parentheses = true; //default: true
  int flag = 0;
  for (int index = p; index <= q; index++) {
    if (index == p) {
      if (tokens[index].type == '(') {
        flag++;
      }
      else if (tokens[index].type == ')') {
        flag--;
        valid_parentheses = false;
      }
      else {
        valid_parentheses = false;
      }
    }
    else if (index == q) {
      if (tokens[index].type == '(') {
        flag++;
        valid_parentheses = false;
      }
      else if (tokens[index].type == ')') {
        flag--;
      }
      else {
        valid_parentheses = false;
      }
    }
    else {
      if (tokens[index].type == '(') {
        flag++;
      }
      else if (tokens[index].type == ')') {
        flag--;
      }
      else {
        ;
      }
    }

    if (index != q && flag == 0) {
      valid_parentheses = false;
    }
    else if (index == q && flag != 0) {
      *success = false;
    }
    else if (flag < 0) {
      *success = false;
    }
  }
  return valid_parentheses;
}

// get the main operator 
int get_main_operator(int p, int q, bool *success) {
  int mp_position = -1;
  int level = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      level++;
    }
    else if (tokens[i].type == ')') {
      level--;
    }
    else if (level == 0 && priority(tokens[i].type) > 0 ) {
       if (mp_position == -1 || (priority(tokens[mp_position].type) >= priority(tokens[i].type) &&
        tokens[i].type != TK_NEG && tokens[i].type != TK_DEREF)){
          mp_position = i;
       }
    }
  }
  if (mp_position == -1) {
    *success = false;
    return 0;
  }
  return mp_position;
}

word_t eval(int p, int q, bool *success) {
  word_t value = 0;   // default: 0
  if (p > q) {
    // bad expression
    *success = false;
  }
  else if (p == q) {
    // single token. return the value
    switch (tokens[p].type) {
      // dec
      case TK_DECNUM:
        sscanf(tokens[p].str, "%u", &value);
        break;
      // hex
      case TK_HEXNUM:
        sscanf(tokens[p].str, "%x", &value);
        break;
      // reg
      case TK_REG:
        value = isa_reg_str2val(tokens[p].str, success);
        break;
      // default: failed
      default:
        *success = false;
    }
  }
  else {
    bool valid_parentheses = check_parentheses(p, q, success);
    if (valid_parentheses && *success) {
      // the expression is surrounded by a matched pair of parentheses
      return eval(p+1, q-1, success);
    }
    else if (!valid_parentheses && *success) {
      // find main operator  
      int mp_position = get_main_operator(p, q, success);
      // divide and conquer
      switch (tokens[mp_position].type) {
        case '+': 
          return eval(p, mp_position - 1, success) + eval(mp_position + 1, q, success);
        case '-': 
          return eval(p, mp_position - 1, success) - eval(mp_position + 1, q, success);
        case '*':
          return eval(p, mp_position - 1, success) * eval(mp_position + 1, q, success);
        case '/':
          if (eval(mp_position + 1, q, success) == 0) {
            *success = false;
            printf("div by 0 !!!!\n");
            return 0;
          }
          return eval(p, mp_position - 1, success) / eval(mp_position + 1, q, success);
        case TK_NEG:
          return 0 - eval(mp_position + 1, q, success);
        case TK_EQ:
          return eval(p, mp_position - 1, success) == eval(mp_position + 1, q, success);
        case TK_NE:
          return eval(p, mp_position - 1, success) != eval(mp_position + 1, q, success);
        case TK_AND:
          return eval(p, mp_position - 1, success) && eval(mp_position + 1, q, success);
        case TK_DEREF:
          return vaddr_read(eval(mp_position + 1, q, success), 4);
        // unknow type: failed
        default: 
          return 0;
      }
    }
    else {
      // *success == false
      ; // pass
    }
  }
  return value;
}

word_t expr(char *e, bool *success) {
  word_t value = 0;   // default: 0

  // if make_token() failed
  if (!make_token(e)) {
    *success = false;
  }

  /* TODO: Insert codes to evaluate the expression. */
  else {
    // evaluate the expression, and confirm its success
    value = eval(0, nr_token-1, success);
  }
  
  return value;
}
