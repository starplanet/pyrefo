from pyrefo.match import *


def test_search():
    m = search(Literal('a'), ['a'])
    assert m is not None
    assert m.group() == (0, 1)


def test_findall():
    m = findall(Literal('a'), ['a', 'a', 'a'])
    assert len(m) == 3
    assert m[0].group() == (0, 1)
    assert m[2].group() == (2, 3)


def test_pharse_search():
    m = search(Phrase('ab'), ['a', 'b', 'c'])
    assert m.group() == (0, 2)