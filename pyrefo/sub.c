// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int news = 0, frees = 0;

Sub*
newsub(int n)
{
	Sub *s;
	s = mal(sizeof *s);
	s->nsub = n;
	s->ref = 1;
	news += 1;
	printf("new sub %p, news %d\n", s, news);
	return s;
}

Sub*
incref(Sub *s)
{
	s->ref++;
	return s;
}

Sub*
update(Sub *s, int i, int p)
{
	Sub *s1;
	int j;

	if(s->ref > 1) {
		s1 = newsub(s->nsub);
		for(j=0; j<s->nsub; j++)
			s1->sub[j] = s->sub[j];
		s->ref--;
		s = s1;
	}
	s->sub[i] = p;
	return s;
}

void
decref(Sub *s)
{
	if(--s->ref == 0) {
		frees += 1;
		printf("free sub %p, frees %d\n", s, frees);
		free(s);
	}
}