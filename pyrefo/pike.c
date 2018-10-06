// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int gen;

void *mal(int n)
{
	void *v;
	v = malloc(n);
	if (v == nil)
	{
		fatal("out of memory");
	}
	memset(v, 0, n);
	return v;
}

void fatal(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	fprintf(stderr, "fatal error:");
	vfprintf(stderr, fmt, arg);
	fprintf(stderr, "\n");
	va_end(arg);
	exit(2);
}

typedef struct Thread Thread;
struct Thread
{
	Inst *pc;
	Sub *sub;
};

typedef struct ThreadList ThreadList;
struct ThreadList
{
	int n;
	Thread t[1];
};

static Thread
thread(Inst *pc, Sub *sub)
{
	Thread t = {pc, sub};
	return t;
}

static ThreadList *threadlist(int n)
{
	return mal(sizeof(ThreadList) + n * sizeof(Thread));
}

static void
addthread(ThreadList *l, Thread t, int sp)
{
	if (t.pc->gen == gen)
	{
		decref(t.sub);
		return; // already on list
	}
	t.pc->gen = gen;

	switch (t.pc->opcode)
	{
	default:
		l->t[l->n] = t;
		l->n++;
		break;
	case Split:
		addthread(l, thread(t.pc->x, incref(t.sub)), sp);
		addthread(l, thread(t.pc->y, t.sub), sp);
		break;
	case Save:
		if (t.pc->n)
			addthread(l, thread(t.pc->x, update(t.sub, t.pc->n, sp)), sp);
		else
			addthread(l, thread(t.pc->x, t.sub), sp);
		break;
	}
}

int findall(Prog *prog, Seq *seq, SubMatch **m, int nlen)
{
	int i, j, len, c;
	ThreadList *clist, *nlist, *tmp;
	Inst *pc;
	Sub *sub;

	//printf("findall submatch %d\n", (*m)->len);
	c = 0;
	sub = newsub(MAXSUB);
	for (i = 0; i < sub->nsub; i++)
		sub->sub[i] = -1;

	len = prog->len;
	clist = threadlist(len);
	nlist = threadlist(len);

	gen++;
	printf("prog len %d, seq len %d\n", prog->len, seq->len);
	addthread(clist, thread(prog->start, sub), 0);
	for (i = 0; i < seq->len + 1; i++)
	{
		if (clist->n == 0)
			break;
		gen++;
		//printf("clist n %d, seq i %d\n", clist->n, i);
		for (j = 0; j < clist->n; j++)
		{
			pc = clist->t[j].pc;
			sub = clist->t[j].sub;
			switch (pc->opcode)
			{
			case Char:
				if (i == seq->len ||
					comp_func_callback(pc->o, seq->start[i]) == 0)
				{ // not match
					decref(sub);
				}
				else
				{ // match
					addthread(nlist, thread(pc->x, sub), i + 1);
				}
				break;
			case Match:
				mk_submatch(sub, *m);
				m += 1;
				nlen -= 1;
				c += 1;
				decref(sub);
				if (nlen == 0)
				{
					for (j++; j < clist->n; j++)
						decref(clist->t[j].sub);
					goto BreakFor;
				}
			}
		}
	BreakFor:
		tmp = clist;
		clist = nlist;
		nlist = tmp;
		nlist->n = 0;
		if (nlen == 0)
		{
			break;
		}
	}
	// free sub
	for (i = 0; i < clist->n; i++)
	{
		decref(clist->t[i].sub);
	}
	return c;
}

void mk_submatch(Sub *sub, SubMatch *m)
{
	int k, l;
	Pos *pos;
	for (k = sub->nsub; k > 0; k--)
	{
		if (sub->sub[k - 1] > -1)
			break;
	}
	for (l = 1; l < k; l += 2)
	{
		pos = m->pos + l / 2;
		pos->start = sub->sub[l];
		pos->end = sub->sub[l + 1];
	}
}

void print_submatch(SubMatch *m)
{
	int k;
	Pos *pos;
	for (k = 0; k < m->len; k++)
	{
		pos = m->pos + k;
		printf("submatch start %d end %d\n", pos->start, pos->end);
	}
}

int search(Prog *prog, Seq *seq, SubMatch *m)
{
	int matched = findall(prog, seq, &m, 1);
	if (matched)
	{
		print_submatch(m);
	}
	return matched;
}

Seq *Seq_new(int n)
{
	return mal(sizeof(Seq) + n * sizeof(void *));
}

void Seq_delete(Seq *seq)
{
	free(seq);
}

SubMatch *SubMatch_new(int n)
{
	return mal(sizeof(SubMatch) + n * sizeof(Pos));
}

void SubMatch_delete(SubMatch *m)
{
	free(m);
}