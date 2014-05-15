/*
 * handler_list.h - Simple list for handlers (blocking orders).
 * Copyright (C) 2008 Matthieu Crapet <mcrapet@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef HANDLER_LIST_H
#define HANDLER_LIST_H

#define LIST_MAX_ELEMENTS 15

enum order_types {
  ORDER_HANDLER_CLOCK,
  ORDER_HANDLER_MESSAGE,
  ORDER_HANDLER_MESSAGE_UPTIME
};

typedef struct {
  char *format;
} handler_clock_t;

typedef struct {
  char *message;
  unsigned short style;
} handler_text_t;

typedef int (*handler_func)(void *);

typedef struct element
{
  int command;
  handler_func cb;
  union {
    handler_clock_t clock;
    handler_text_t  text;
  } data;
} handler_t;


/* Internal structure */
typedef struct tablist_s {
  long nb; // current element number
  long index;
  struct element slot[LIST_MAX_ELEMENTS];
} list_t, handler_list_t;


/* Prototypes */
void handler_init(list_t *t);
struct element *handler_add(list_t *, struct element *);
struct element *handler_first(list_t *);
struct element *handler_last(list_t *);
struct element *handler_get(list_t *, int index);
long handler_count(list_t *);

#endif /* HANDLER_LIST_H */
