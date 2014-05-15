/*
 * handler_list.c - Simple list for handlers (blocking orders).
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

#include <stdlib.h> // NULL
#include <string.h> // memcpy

#include "handler_list.h"


/* Functions definition */

void handler_init(list_t *t)
{
  t->nb = 0;
  t->index = LIST_MAX_ELEMENTS - 1;
}

struct element *handler_add(list_t *t, struct element *e)
{
  int i;

  if (t->nb >= LIST_MAX_ELEMENTS)
    return NULL;

  i = (t->index + t->nb) % LIST_MAX_ELEMENTS;

  memcpy(&t->slot[i], e, sizeof(struct element));
  t->nb++;

  return &t->slot[i];
}

struct element *handler_first(list_t *t)
{
  return (t->nb == 0) ? NULL : &t->slot[t->index];
}

struct element *handler_last(list_t *t)
{
  return (t->nb == 0) ? NULL :
    &t->slot[(t->index + t->nb - 1) % LIST_MAX_ELEMENTS];
}

struct element *handler_get(list_t *t, int index)
{
  if (t->nb > 0) {
    if (index < 0 && -index <= t->nb)
      return &t->slot[(t->index + t->nb + index) % LIST_MAX_ELEMENTS];
    else if (index < t->nb) // index>=0
      return &t->slot[(t->index + index) % LIST_MAX_ELEMENTS];
  }
  return NULL;
}

long handler_count(list_t *t)
{
  return t->nb;
}
