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

#define nil ((void*)0)
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

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
	MAXSUB = 20
};

typedef struct Sub Sub;

struct Sub
{
	int ref;
	int nsub;
	int sub[MAXSUB];
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
Sub *copy(Sub*);
Sub *update(Sub*, int, int);
void decref(Sub*);

int search(Prog*, Seq*, SubMatch*);
extern int comp_func_callback(void *o, void *y);

#endif