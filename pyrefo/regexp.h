// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef REGEXP_H
#define REGEXP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define DEBUG 0
#define nil ((void*)0)
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

#define debug_printf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

typedef struct Prog Prog;
typedef struct Inst Inst;
typedef struct Seq Seq;
typedef struct Pos Pos;
typedef struct SubMatch SubMatch;

void fatal(char*, ...);
void *mal(int);

struct Prog
{
	Inst *start;
	int len;
};

struct Inst
{
	int opcode;
	void *o;
	int n;
	Inst *x;
	Inst *y;
	int gen;	// global state, oooh!
};

struct Seq
{
	int len;
	void *start[];
};

enum	/* Inst.opcode */
{
	Char = 1,
	Match,
	Split,
	Save,
};

enum {
	MAXSUB = 20,
	MAXCACHE = 10,
};

typedef struct Sub Sub;
typedef struct Cache Cache;

struct Sub
{
	int ref;
	int nsub;
	int sub[MAXSUB];
};

struct Cache
{
	int ref;
	int nlen;
	void *cache[MAXCACHE];
};

struct Pos
{
	int start;
	int end;
};

struct SubMatch
{
	int len;
	Pos *pos;
};

Sub *newsub(int n);
Sub *incref(Sub*);
Sub *update(Sub*, int, int);
void decref(Sub*);

Cache *newcache(void);
Cache *incref_cache(Cache*);
Cache *update_cache(Cache*, int, void*);
void decref_cache(Cache*);

void mk_submatch(Sub *sub, SubMatch *m);
void print_submatch(SubMatch *m);
int findall(Prog *prog, Seq *seq, SubMatch **m, int nlen);
int search(Prog *prog, Seq *seq, SubMatch *m);

Seq* Seq_new(int n);
void Seq_delete(Seq *seq);
SubMatch* SubMatch_new(int n);
void SubMatch_delete(SubMatch *m);
extern int comp_func_callback(void *o, void *y[], int ylen);

#endif