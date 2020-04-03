from pyrefo._refo import ffi, lib

from pyrefo.patterns import (Any, Star, Group)


@ffi.def_extern()
def comp_func_callback(o, y, ylen):
    o = ffi.from_handle(o)
    if ylen == 1:
        y = ffi.from_handle(y[0])
    else:
        y = [ffi.from_handle(y[i]) for i in range(ylen)]
    r = o.comparison_function(y)
    return r


class Seq(object):
    def __init__(self, iterable):
        self._p = [ffi.new_handle(item) for item in iterable]
        self._seq = ffi.new('Seq*', {'len': len(iterable), 'start': self._p})

    def __len__(self):
        return self._seq.len

    @property
    def len(self):
        return self._seq.len


class Match(object):
    def __init__(self, state):
        self.state = state
        self.len = len(self.state)
        self._pos = ffi.new('Pos[]', self.len)
        self._m = ffi.new('SubMatch*', {'len': self.len, 'pos': self._pos})

    def span(self, key=None):
        return self[key]

    def start(self, key=None):
        return self[key][0]

    def end(self, key=None):
        return self[key][1]

    def group(self, key=None):
        return self[key]

    def __getitem__(self, key):
        try:
            pos = self._m.pos[self.state[key] - 1]
            return pos.start, pos.end
        except KeyError:
            raise KeyError(key)

    def __contains__(self, key):
        return key in self.state

    def __iter__(self):
        for key in self.state:
            yield key

    def __repr__(self):
        return 'Match(%d, %d)' % (self.start(), self.end())


def _match(pat, iterable):
    code = pat.compile()
    prog = ffi.new('Prog*', {'len': len(pat), 'start': code._inst})
    seq = Seq(iterable)
    match = Match(pat._state_i)
    matched = lib.search(prog, seq._seq, match._m)
    if matched:
        return match
    return None


def search(pat, iterable):
    pat = Star(Any(), greedy=False) + Group(pat, None)
    return _match(pat, iterable)


def match(pat, iterable):
    pat = Group(pat, None)
    return _match(pat, iterable)


def findall(pat, iterable, nmax=20):
    pat = Star(Any(), greedy=False) + Group(pat, None)
    code = pat.compile()
    prog = ffi.new('Prog*', {'len': len(pat), 'start': code._inst})
    seq = Seq(iterable)
    m = [Match(pat._state_i) for _ in range(nmax)]
    ms = ffi.new('SubMatch*[]', [x._m for x in m])
    c = lib.findall(prog, seq._seq, ms, nmax)
    return m[:c]


def finditer(pat, iterable, nmax=20):
    m = findall(pat, iterable, nmax)
    for item in m:
        yield item
