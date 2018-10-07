from pyrefo._refo import ffi, lib


class Instruction(object):
    def __init__(self, opcode):
        self._inst = ffi.new('Inst*')
        self._inst.opcode = opcode
        # new_handle对象必须保存在python变量空间，直接复制给<cdata void*>不能keep alive
        self._o = ffi.new_handle(self)
        self._inst.o = self._o


class Atom(Instruction):
    def __init__(self, comparison_function, succ=None):
        super().__init__(lib.Char)
        self.comparison_function = comparison_function
        if succ is not None:
            self.succ = succ
            self._inst.x = succ._inst

    def __repr__(self):
        return 'Atom({0})'.format(repr(self.comparison_function))


class Accept(Instruction):
    def __init__(self):
        super().__init__(lib.Match)
        self.succ = None
        self._inst.x = ffi.NULL

    def __repr__(self):
        return 'Accept'


class Split(Instruction):
    def __init__(self, s1=None, s2=None):
        super().__init__(lib.Split)
        if s1 is not None:
            self.set_succ(s1)
        if s2 is not None:
            self.set_split(s2)

    def __repr__(self):
        return 'Split({}, {})'.format(repr(self.succ), repr(self.split))

    def set_succ(self, succ):
        self.succ = succ
        self._inst.x = succ._inst

    def set_split(self, split):
        self.split = split
        self._inst.y = split._inst


class Save(Instruction):
    def __init__(self, n=None, succ=None):
        super().__init__(lib.Save)
        if n is not None:
            self._inst.n = n
        if succ is not None:
            self.set_succ(succ)

    def __repr__(self):
        return 'save({})'.format(repr(self._inst.n))

    def set_succ(self, succ):
        self.succ = succ
        self._inst.x = succ._inst

    def set_n(self, n):
        self._inst.n = n
