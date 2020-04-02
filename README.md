### pyrefo: a fast regex for object

### ![Build Status](https://github.com/yimian/pyrefo/workflows/Python%20package/badge.svg)

This project is based on [refo](https://github.com/machinalis/refo) and the paper [Regular Expression Matching: the Virtual Machine Approach](https://swtch.com/~rsc/regexp/regexp2.html), it use cffi to extend python with c to speed accelerate processing performance.

This project has done the following work:

1. full compatiable with refo api, support all patterns and match, search, finditer methods;
2. fix c source bug included in the paper;
3. use cffi to extend python with c;
4. add new feature which supports partial match;
5. add new `Phrase`pattern which can realize `'ab'`match `['a', 'b', 'c']`list;

### How to use it

`"ab" ` is `Literal("a")+Literal("b")`

`"a*"` is `Star(Literal("a"))`

`"aab?"` is `Literal("a")+Literal("a")+Question(Literal("b"), greedy=False)`

`a{3,4}` is `Repetition(Literal("a"), 3, 4, greedy=False)`

`"(ab)+|(bb*)?"` is 

```python
a = Literal("a")
b = Literal("b")
regex = Plus(a + b) | Star(b + b, greedy=False)
```

You can also assign a group to any sub-match and later on retrieve the matched content, for instance:

```python
regex = Group(Plus(a + b), "foobar") | Star(b + b, greedy=False)
m = match(regex, "abab")
print(m.span("foobar"))
```

`pyrefo` offers `match`, `search`, `findall`, `finditer` search functions:

- match: match pattern from first position
- search: search pattern from first position till find one
- findall: find all matched result
- finditer: return an iterator for all matched result

`pyrefo` offers the following predicates:

- Any
- Literal
- Star
- Plus
- Question
- Group
- Repetition
- Phrase

### Performance test

#### prerequisites

```python
import jieba
text = '为什么在本店买东西？因为物流迅速＋品质保证。为什么我购买的每件商品评价都一样呢？因为我买的东西太多了，积累了很多未评价的订单，所以我统一用这段话作为评价内容。如果我用了这段话作为评价，那就说明这款产品非常赞，非常好！'
tokens = list(jieba.cut(text))
```

#### CPython

- pyrefo

```python
from pyrefo import search, Group, Star, Any, Literal
%timeit search(Group(Literal('物流') + Star(Any()) + Literal('迅速'), 'a'), tokens)
```

```shell
95.9 µs ± 472 ns per loop (mean ± std. dev. of 7 runs, 10000 loops each)
```

- refo

```python
import refo
%timeit refo.search(refo.Group(refo.Literal('物流') + refo.Star(refo.Any()) + refo.Literal('迅速'), 'a'), tokens)
```

```shell
1.03 ms ± 7.27 µs per loop (mean ± std. dev. of 7 runs, 1000 loops each)
```

- re

```python
import re
%timeit re.search('(物流.*速度)', text)
```

```shell
989 ns ± 4.69 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
```

#### PyPy

- pyrefo

```python
from pyrefo import search, Group, Star, Any, Literal
%timeit search(Group(Literal('物流') + Star(Any()) + Literal('迅速'), 'a'), tokens)
```

```shell
53.4 µs ± 28 µs per loop (mean ± std. dev. of 7 runs, 1000 loops each)
```

- refo

```python
import refo
%timeit refo.search(refo.Group(refo.Literal('物流') + refo.Star(refo.Any()) + refo.Literal('迅速'), 'a'), tokens)
```

```shell
78 µs ± 35.8 µs per loop (mean ± std. dev. of 7 runs, 1000 loops each)
```

- re

```shell
import re
%timeit re.search('(物流.*速度)', text)
```

```shell
347 ns ± 3.26 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)
```
