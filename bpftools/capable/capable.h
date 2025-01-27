// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
//
// Copyright 2022 Sony Group Corporation

#ifndef __CAPABLE_H
#define __CAPABLE_H

#define TASK_COMM_LEN 16

struct cap_event
{
  unsigned int pid;
  unsigned int cap;
  unsigned int tgid;
  unsigned int uid;
  int audit;
  int insetid;
  char task[TASK_COMM_LEN];
};

struct cap_key_t
{
  unsigned int pid;
  unsigned int tgid;
  int user_stack_id;
  int kern_stack_id;
};

enum uniqueness
{
  UNQ_OFF,
  UNQ_PID,
  UNQ_CGROUP
};

#endif /* __CAPABLE_H */
