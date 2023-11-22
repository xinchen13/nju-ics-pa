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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static int wp_num = 1;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// get a new watchpoints in free_ and return
WP* new_wp(char* WP_expr, bool *success) {
  // no available watchpoints in free_
  if (free_->next == NULL) {
    assert(0);
  }
  // reserve the rest watchpoints in free_
  WP* wp_return = free_->next;
  free_->next = wp_return->next;
  // initialize the watchpoint: Num and expr
  wp_return->next = NULL;
  wp_return->NO = wp_num++;
  strcpy(wp_return->WP_expr, WP_expr);
  wp_return->val = expr(wp_return->WP_expr, success);
  // put wp_return into head
  if (head == NULL) {
    head = wp_return;
  }
  else {
    wp_return->next = head->next;
    head->next = wp_return;
  }
  return wp_return;
}

// free a watchpoint in head by num
void free_wp(int num) {
  // there's no wp
  if (head == NULL) {
    printf("there's no breakpoint 0.o?\n");
  }
  else {
    // if the wp to free is head 
    if (head->NO == num) {
      WP* buf = head->next;
      head->next = free_->next;
      free_->next = head;
      head = buf;
      return;
    }
    WP* wp_ptr = head;
    // traversal
    while (wp_ptr->next != NULL) {
      if (wp_ptr->next->NO == num) {
        // remove from head and put into free_
        WP* buf = wp_ptr->next->next;
        wp_ptr->next->next = free_->next;
        free_->next = wp_ptr->next;
        wp_ptr->next = buf;
        return;
      }
      else {
        wp_ptr = wp_ptr->next;
      }
    }
    // no match wp
    printf("no breakpoint number %d\n", num);
  }
}

// display all of the wp
void watchpoint_display() {
  WP* wp_ptr = head;
  if (wp_ptr == NULL){
    printf("no watcpoints 0.o?\n");
  }
  else{
    printf("num\twhat\tvalue\n");
    while (wp_ptr != NULL){
      printf("%d\t%s\t%d\n", wp_ptr->NO, wp_ptr->WP_expr, wp_ptr->val);
      wp_ptr = wp_ptr->next;
    }
  }
}

// check all the wp, return whether changed
int check_watchpoint() {
  bool success = true;
  word_t new_val;
  int break_flag = 0;
  WP* wp_ptr = head;
  
  while (wp_ptr != NULL) {
    new_val = expr(wp_ptr->WP_expr, &success);
    if (success) {
        if (wp_ptr->val != new_val) {
          printf("watchpoint %d: %s\nold value = %d = " FMT_WORD "\nnew value = %d = " FMT_WORD "\n",
          wp_ptr->NO, wp_ptr->WP_expr, wp_ptr->val, wp_ptr->val, new_val, new_val);
          wp_ptr->val = new_val;
          break_flag = 1;
        }
    }
    else {
      assert(0);
    }
    wp_ptr = wp_ptr->next;
  }
  return break_flag;
}
