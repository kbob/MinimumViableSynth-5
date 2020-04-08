#!/usr/bin/env python3

def next_class(s):
    mro = s.__self_class__.__mro__
    try:
        i = mro.index(s.__thisclass__)
    except ValueError:
        return '???'
    return mro[i + 1].__name__


class mixin:
    def __init__(self):
        print(f'mixin -> {next_class(super())}')
        super().__init__()

class abstract:
    def __init__(self):
        print(f'abstract -> {next_class(super())}')
        super().__init__()

class base(abstract, mixin):
    def __init__(self):
        super().__init__()
        print(f'base -> {next_class(super())}')

class derived(base):
    def __init__(self):
        super().__init__()
        print(f'derived -> {next_class(super())}')

print(f'MRO = {" ".join(s.__name__ for s in derived.__mro__)}')
d = derived()
