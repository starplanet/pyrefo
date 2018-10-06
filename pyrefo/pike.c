// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int gen;

void* 
mal(int n)
{
	void *v;
	v = malloc(n);
	if(v == nil) {
		fatal("out of memory");
	}
	memset(v, 0, n);
	return v;
}

void 
fatal(char *fmt, ...)
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

static ThreadList *
threadlist(int n)
{
	return mal(sizeof(ThreadList)+n*sizeof(Thread));
}

static void
addthread(ThreadList *l, Thread t, int sp)
{
	if(t.pc->gen == gen) {
		decref(t.sub);
		return;	// already on list
	}
	t.pc->gen = gen;
	
	switch(t.pc->opcode) {
	default:
		l->t[l->n] = t;
		l->n++;
		break;
	case Split:
		addthread(l, thread(t.pc->x, incref(t.sub)), sp);
		addthread(l, thread(t.pc->y, t.sub), sp);
		break;
	case Save:
		printf("save %d\n", t.pc->n);
		if(t.pc->n)
			addthread(l, thread(t.pc->x, update(t.sub, t.pc->n, sp)), sp);
		else
			addthread(l, thread(t.pc->x, t.sub), sp);
		break;
	}
}

int
pikevm(Prog *prog, Seq *seq, int *subp, int nsubp)
{
	int i, j, len;
	ThreadList *clist, *nlist, *tmp;
	Inst *pc;
	Sub *sub, *matched;
	
	for(i=0; i<nsubp; i++)
		subp[i] = -1;
	sub = newsub(nsubp);
	for(i=0; i<nsubp; i++)
		sub->sub[i] = -1;

	len = prog->len;
	clist = threadlist(len);
	nlist = threadlist(len);
	
	gen++;
	// printf("gen %d\n", gen);
	//printf("prog len %d, seq len %d\n", prog->len, seq->len);
	addthread(clist, thread(prog->start, sub), 0);
	matched = nil;
	for(i=0; i<seq->len; i++) {
		if(clist->n == 0)
			break;
		gen++;
		//printf("clist n %d, seq i %d\n", clist->n, i);
		for(j=0; j<clist->n; j++) {
			pc = clist->t[j].pc;
			sub = clist->t[j].sub;
			switch(pc->opcode) {
			case Char:
				if(comp_func_callback(pc->o, seq->start[i]) == 0) {  // not match
					decref(sub);
				} else {  // match
					addthread(nlist, thread(pc->x, sub), i+1);
				}
				break;
			case Match:
				matched = sub;
				for(j++; j<clist->n; j++)
					decref(clist->t[j].sub);
				goto BreakFor;
			}
		}
	BreakFor:
		tmp = clist;
		clist = nlist;
		nlist = tmp;
		nlist->n = 0;
		if(matched) {
			break;
		}
	}
	// free sub
	for(i=0; i<clist->n; i++) {
		decref(clist->t[i].sub);
	}
	if(matched) {
		for(i=0; i<nsubp; i++)
			subp[i] = matched->sub[i];
		decref(matched);
		return 1;
	}
	return 0;
}


int
search(Prog *prog, Seq *seq, SubMatch *m)
{
	int sub[MAXSUB];
	int k, l;
	Pos* pos;
	// printf("enter search\n");
	int matched = pikevm(prog, seq, sub, nelem(sub));
	if(matched) {
		printf("match");
		for(k=MAXSUB; k>0; k--) {
			if(sub[k-1] > -1)
				break;
		}
		for(l=1; l<k; l+=2) {
			pos = m->pos + l/2;
			pos->start = sub[l];
			pos->end = sub[l+1];
			printf(" (");
			printf("%d", sub[l]);
			printf(",");
			printf("%d", sub[l + 1]);
			printf(")");
		}
		printf("\n");
		for(k=0; k<m->len; k++) {
			pos = m->pos + k;
			printf("submatch start %d end %d\n", pos->start, pos->end);
		}
	}
	return matched;
}