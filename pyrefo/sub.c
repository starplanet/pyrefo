// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int news = 0, frees = 0;
int newc = 0, freec = 0;

Sub *newsub(int n)
{
	Sub *s;
	s = mal(sizeof *s);
	s->nsub = n;
	s->ref = 1;
	news += 1;
	debug_printf("new sub %p, news %d\n", s, news);
	return s;
}

Sub *incref(Sub *s)
{
	s->ref++;
	return s;
}

Sub *update(Sub *s, int i, int p)
{
	Sub *s1;
	int j;

	if (s->ref > 1)
	{
		s1 = newsub(s->nsub);
		for (j = 0; j < s->nsub; j++)
			s1->sub[j] = s->sub[j];
		s->ref--;
		s = s1;
	}
	s->sub[i] = p;
	return s;
}

void decref(Sub *s)
{
	if (--s->ref == 0)
	{
		frees += 1;
		debug_printf("free sub %p, frees %d\n", s, frees);
		free(s);
	}
}

Cache *newcache()
{
	Cache *c;
	c = mal(sizeof *c);
	c->ref = 1;
	c->nlen = 0;
	newc += 1;
	debug_printf("new cache %p, newc %d\n", c, newc);
	return c;
}

Cache *incref_cache(Cache *c)
{
	c->ref++;
	return c;
}

Cache *update_cache(Cache *c, int i, void *p)
{
	Cache *c1;
	int j;

	if (c->ref > 1)
	{
		c1 = newcache();
		for (j = 0; j < i; j++)
			c1->cache[j] = c->cache[j];
		c->ref--;
		c = c1;
	}
	if (i == -1)
	{
		c->nlen = 0;
	}
	else
	{
		c->nlen = i + 1;
		c->cache[i] = p;
	}
	return c;
}

void decref_cache(Cache *c)
{
	if (--c->ref == 0)
	{
		freec += 1;
		debug_printf("free cache %p, freec %d\n", c, freec);
		free(c);
	}
}