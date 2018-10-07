from collections import OrderedDict

from pyrefo.inst import (Accept, Atom, Split, Save)


class Pattern(object):
    def __init__(self):
        self._c = 0  # 指令计数
        self._n = 0  # group计数
        self._state = OrderedDict()  # 记录group key与submatch save inst关系
        self._state_i = {}  # 记录group key与submatch index关系
        self.arg = None

    def _compile(self, cont):
        raise NotImplementedError

    def compile(self):
        code = self._compile(Accept())
        self._c += 1
        self.number_save()
        return code

    def number_save(self):
        i = 1
        for k, (s, e) in self._state.items():
            s.set_n(2 * i - 1)
            e.set_n(2 * i)
            self._state_i[k] = i
            i += 1

    def __or__(self, other):
        return Disjunction(self, other)

    def __add__(self, other):
        xs = []
        for item in [self, other]:
            if isinstance(item, Concatenation):
                xs.extend(item.xs)
            else:
                xs.append(item)
        return Concatenation(*xs)

    def __mul__(self, x):
        if isinstance(x, int):
            mn = x
            mx = x
        else:
            assert isinstance(x, tuple)
            mn, mx = x
            if mn is None:
                mn = 0
        return Repetition(self, mn=mn, mx=mx)

    def __str__(self):
        return str(self.arg)

    def __repr__(self):
        return '{1}{0!r}'.format(self.arg, self.__class__.__name__)

    def __len__(self):
        return self._c


class Predicate(Pattern):
    def __init__(self, f):
        super().__init__()
        self.f = f
        self.arg = f

    def _compile(self, cont):
        """
        :param cont: continue instructions
        :return:
        """
        x = Atom(self.f, succ=cont)
        self._c = 1
        return x


class Any(Predicate):
    def __init__(self):
        super(Any, self).__init__(self._comp_func)

    def __str__(self):
        return "Any()"

    def __repr__(self):
        return "Any()"

    def _comp_func(self, y):
        return True


class Literal(Predicate):
    def __init__(self, x):
        super(Literal, self).__init__(self._comp_func)
        self.x = x
        self.arg = x

    def _comp_func(self, y):
        return self.x == y


class Disjunction(Pattern):
    def __init__(self, a, b):
        super().__init__()
        self.a = a
        self.b = b

    def _compile(self, cont):
        a = self.a._compile(cont)
        b = self.b._compile(cont)
        self._c = self.a._c + self.b._c + 1
        self._state.update(self.a._state)
        self._state.update(self.b._state)
        return Split(a, b)

    def __str__(self):
        return "(" + " | ".join(map(str, [self.a, self.b])) + ")"

    def __repr__(self):
        return "(" + " | ".join(map(repr, [self.a, self.b])) + ")"


class Concatenation(Pattern):
    def __init__(self, *patterns):
        super().__init__()
        self.xs = list(patterns)
        assert len(self.xs) != 0

    def _compile(self, cont):
        code = cont
        self._c = 0
        for x in reversed(self.xs):
            code = x._compile(code)
            self._c += x._c
        for x in self.xs:
            self._state.update(x._state)
        return code

    def __str__(self):
        return "(" + " + ".join(map(str, self.xs)) + ")"

    def __repr__(self):
        return "(" + " + ".join(map(repr, self.xs)) + ")"


class Star(Pattern):
    def __init__(self, pattern, greedy=True):
        super().__init__()
        self.x = pattern
        self.greedy = greedy
        self.arg = pattern

    def _compile(self, cont):
        # In words: split to (`x` and return to split) and `cont`
        split = Split()
        x = self.x._compile(split)
        if self.greedy:
            split.set_succ(x)
            split.set_split(cont)
        else:
            split.set_succ(cont)
            split.set_split(x)
        self._c = self.x._c + 1
        self._state.update(self.x._state)
        return split

    def __str__(self):
        return str(self.x) + "*"


class Plus(Pattern):
    def __init__(self, pattern, greedy=True):
        super().__init__()
        self.x = pattern
        self.greedy = greedy
        self.arg = pattern

    def _compile(self, cont):
        # In words: take `x` and split to `x` and `cont`
        split = Split()
        x = self.x._compile(split)
        if self.greedy:
            split.set_succ(x)
            split.set_split(cont)
        else:
            split.set_succ(cont)
            split.set_split(x)
        # `Star` would return `split`
        self._c = self.x._c + 1
        self._state.update(self.x._state)
        return x

    def __str__(self):
        return str(self.x) + "+"


class Question(Pattern):
    def __init__(self, pattern, greedy=True):
        super().__init__()
        self.x = pattern
        self.greedy = greedy
        self.arg = pattern

    def _compile(self, cont):
        xcode = self.x._compile(cont)
        self._c = self.x._c + 1
        self._state.update(self.x._state)
        if self.greedy:
            return Split(xcode, cont)
        else:
            return Split(cont, xcode)

    def __str__(self):
        return str(self.x) + "?"


class Group(Pattern):
    def __init__(self, pattern, key):
        super().__init__()
        self.x = pattern
        self.key = key

    def _compile(self, cont):
        start = Save()
        end = Save()
        self._state[self.key] = (start, end)
        code = self.x._compile(end)
        start.set_succ(code)
        end.set_succ(cont)
        self._c = self.x._c + 2
        self._state.update(self.x._state)
        return start

    def __str__(self):
        return "Group({0!s}, {1!s})".format(self.x, self.key)

    def __repr__(self):
        return "Group({0!r}, {1!r})".format(self.x, self.key)


class Repetition(Pattern):
    def __init__(self, pattern, mn=0, mx=None, greedy=True):
        super().__init__()
        assert mn is not None or mx is not None or mn <= mx
        self.x = pattern
        self.mn = mn
        self.mx = mx
        self.greedy = greedy

    def _compile(self, cont):
        code = cont
        self._c = 0
        if self.mx is not None:
            q = Question(self.x, self.greedy)
            for _ in range(self.mx - self.mn):
                code = q._compile(code)
                self._c += q._c
        else:
            q = Star(self.x, greedy=self.greedy)
            code = q._compile(code)
            self._c += q._c
        for _ in range(self.mn):
            code = self.x._compile(code)
            self._c += self.x._c
        self._state.update(self.x._state)
        return code

    def __str__(self):
        return self._tostring("{0!s}")

    def __repr__(self):
        return self._tostring("{0!r}")

    def _tostring(self, s):
        base = "(" + s + ")*"
        if self.mn == 0 and self.mx is None:
            return base.format(self.x)
        if self.mn == self.mx:
            return (base + "{1}").format(self.x, self.mn)
        return (base + "*({1},{2})").format(self.x, self.mn, self.mx)


class Phrase(Predicate):
    def __init__(self, x):
        super().__init__(self.match)
        self.x = x
        self.arg = x

    def match(self, y):
        if not isinstance(y, (tuple, list)):
            y = [y]
        y = ''.join(y)
        if self.x == y:
            return 1
        elif self.x.startswith(y):
            return 2
        else:
            return 0
