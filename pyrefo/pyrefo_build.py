from cffi import FFI

ffi = FFI()
ffi.cdef('''

typedef struct Prog Prog;
typedef struct Inst Inst;
typedef struct Seq Seq;
typedef struct Pos Pos;
typedef struct SubMatch SubMatch;

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


Seq* Seq_new(int n);
void Seq_delete(Seq *seq);
SubMatch* SubMatch_new(int n);
void SubMatch_delete(SubMatch *m);

int findall(Prog *prog, Seq *seq, SubMatch **m, int nlen);
int search(Prog*, Seq*, SubMatch*);

extern "Python+C" int comp_func_callback(void *o, void *y[], int ylen);
''')

ffi.set_source('pyrefo._refo', '''
#include   "regexp.h"
''', sources=['pyrefo/pike.c', 'pyrefo/sub.c'], include_dirs=['pyrefo'])

if __name__ == '__main__':
    ffi.compile(verbose=True)
