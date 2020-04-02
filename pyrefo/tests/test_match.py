from pyrefo import search, Literal, findall, finditer, Phrase


def test_search():
    m = search(Literal('a'), ['a'])
    assert m is not None
    assert m.group() == (0, 1)


def test_findall():
    m = findall(Literal('a'), ['a', 'a', 'a'])
    assert len(m) == 3
    assert m[0].group() == (0, 1)
    assert m[2].group() == (2, 3)

    m = findall(Literal('a') + Literal('b'), ['a', 'a', 'a'])
    assert len(m) == 0


def test_pharse_search():
    m = search(Phrase('ab'), ['a', 'b', 'c'])
    assert m.group() == (0, 2)

    m = findall(Phrase('ab'), ['a', 'b', 'c'])
    assert len(m) == 1


def test_finditer():
    items = []
    for item in finditer(Literal('a'), ['a', 'a', 'a']):
        items.append(item)
    assert len(items) == 3
