#!/usr/bin/env python3

import unittest


class Universe:

    def __init__(self, ref):
        self.ref = ref

    def __len__(self):
        return len(self.ref)

    def find(self, member):
        if member in self.ref:
            return self.ref.index(member)
        else:
            return -1

    def index(self, member):
        return self.ref.index(member)

    def __getitem__(self, index):
        return self.ref[index]

    def subset(self, members=(), mask=None):
        return Subset(self, members, mask)

    def all(self):
        return self.subset(mask=(1 << len(self.ref)) - 1)

    def none(self):
        return self.subset(mask=0)


class Subset:

    def __init__(self, universe, members=(), mask=None):
        self.universe = universe
        if mask is None:
            self.mask = 0
            for m in members:
                self.add(m)
        else:
            assert isinstance(mask, int)
            self.mask = mask

    def __repr__(self):
        ref = self.universe.ref
        reprs = (repr(m) for (i, m) in enumerate(ref) if self.mask & (1 << i))
        return f'{{{" ".join(reprs)}}}'

    def __str__(self):
        ref = self.universe.ref
        strs = (str(m) for (i, m) in enumerate(ref) if self.mask & (1 << i))
        return f'{{{" ".join(strs)}}}'

    def __lt__(self, other):
        if isinstance(other, Subset):
            assert self.universe == other.universe
            other = other.mask
        return (self.mask & ~other) == 0 and self.mask != other

    def __le__(self, other):
        if isinstance(other, Subset):
            assert self.universe == other.universe
            other = other.mask
        return (self.mask & ~other) == 0

    def __eq__(self, other):
        if isinstance(other, Subset):
            assert self.universe == other.universe
            other = other.mask
        return self.mask == other

    def __gt__(self, other):
        if isinstance(other, Subset):
            assert self.universe == other.universe
            other = other.mask
        return (other & ~self.mask) == 0 and self.mask != other

    def __ge__(self, other):
        if isinstance(other, Subset):
            assert self.universe == other.universe
            other = other.mask
        return (other & ~self.mask) == 0

    def __hash__(self):
        return hash((id(self.universe), self.mask))

    def __bool__(self):
        return self.mask != 0

    def __len__(self):
        return bin(self.mask).count('1')

    def __contains__(self, member):
        ref = self.universe.ref
        return member in ref and self.mask & (1 << ref.index(member))

    def __sub__(self, other):
        assert self.universe == other.universe
        return type(self)(self.universe, mask=self.mask & ~other.mask)

    def __and__(self, other):
        assert self.universe == other.universe
        return type(self)(self.universe, mask=self.mask & other.mask)

    def __xor__(self, other):
        assert self.universe == other.universe
        return type(self)(self.universe, mask=self.mask ^ other.mask)

    def __or__(self, other):
        assert self.universe == other.universe
        return type(self)(self.universe, mask=self.mask | other.mask)

    def __invert__(self):
        u = self.universe
        return type(self)(u, mask=(1 << len(u.ref)) - 1 & ~self.mask)

    def iter_members(self):
        ref = self.universe.ref
        return (ref[i] for i in range(len(ref)) if self.mask & (1 << i))

    def iter_indices(self):
        ref = self.universe.ref
        return (i for i in range(len(ref)) if self.mask & (1 << i))

    def iter_masks(self):
        ref = self.universe.ref
        return (1 << i for i in range(len(ref)) if self.mask & (1 << i))

    def at(self, index):
        if index < 0 or index >= len(self.universe.ref):
            raise IndexError('Subset index out of range')
        return bool(self.mask & (1 << index))

    def get(self, member):
        return self.at(self.universe.ref.index(member))

    def add(self, m):
        self.mask |= 1 << self.universe.ref.index(m)


class Relation:

    def __init__(self, u1, u2):
        assert isinstance(u1, Universe) and isinstance(u2, Universe)
        self.u1 = u1
        self.u2 = u2
        self.matrix = [u2.subset() for i in u1.ref]

    def add(self, m1, m2):
        self.matrix[self.u1.ref.index(m1)].add(m2)

    def __contains__(self, pair):
        m1, m2 = pair
        return m2 in self.matrix[self.u1.ref.index(m1)]

    def __copy__(self):
        new = type(self)(self.u1, self.u2)
        new.matrix = self.matrix[:]
        return new

    def at(self, index):
        return self.matrix[index]

    def get(self, m1):
        return self.matrix[self.u1.index(m1)]


class TestUniverse(unittest.TestCase):

    def setUp(self):
        self.u = Universe('abc')

    def test_len(self):
        self.assertEqual(len(self.u), 3)

    def test_find(self):
        self.assertEqual(self.u.find('a'), 0)
        self.assertEqual(self.u.find('b'), 1)
        self.assertEqual(self.u.find('c'), 2)
        self.assertEqual(self.u.find('d'), -1)

    def test_index(self):
        self.assertEqual(self.u.index('a'), 0)
        self.assertEqual(self.u.index('b'), 1)
        self.assertEqual(self.u.index('c'), 2)
        with self.assertRaises(ValueError):
            self.u.index('d')

    def test_getitem(self):
        self.assertEqual(self.u[0], 'a')
        self.assertEqual(self.u[1], 'b')
        self.assertEqual(self.u[2], 'c')
        self.assertEqual(self.u[-1], 'c')
        with self.assertRaises(IndexError):
            self.u[3]

    def test_subset(self):
        s = self.u.subset()
        s1 = self.u.subset('a')
        self.assertIs(type(s1), Subset)
        self.assertTrue(bool(s1))

    def test_all(self):
        abc = self.u.all()
        self.assertEqual(abc.mask, 0b111)
        self.assertEqual(abc.universe, self.u)

    def test_none(self):
        empty = self.u.none()
        self.assertEqual(empty.mask, 0)
        self.assertEqual(empty.universe, self.u)

class TestSubsets(unittest.TestCase):

    def setUp(self):
        self.u = u = Universe('abcde')
        self.empty = empty = u.subset()
        self.a = a = u.subset('a')
        self.abc = abc = u.subset('abc')
        self.ace = ace = u.subset('ace')
        self.abcde = abcde = u.subset('abcde')
        self.pairs = [
                      (empty, 0b00000),
                      (a,     0b00001),
                      (abc,   0b00111),
                      (ace,   0b10101),
                      (abcde, 0b11111),
                     ]

    def test_repr(self):
        self.assertEqual(repr(self.empty), '{}')
        self.assertEqual(repr(self.empty), '{}')
        self.assertEqual(repr(self.a), "{'a'}")
        self.assertEqual(repr(self.abc), "{'a' 'b' 'c'}")
        self.assertEqual(repr(self.ace), "{'a' 'c' 'e'}")
        self.assertEqual(repr(self.abcde), "{'a' 'b' 'c' 'd' 'e'}")

    def test_str(self):
        self.assertEqual(str(self.empty), '{}')
        self.assertEqual(str(self.a), '{a}')
        self.assertEqual(str(self.abc), '{a b c}')
        self.assertEqual(str(self.ace), '{a c e}')
        self.assertEqual(str(self.abcde), '{a b c d e}')

    def test_lt(self):
        self.assertTrue(all(self.empty < s for s in (self.a,
                                                     self.abc,
                                                     self.ace,
                                                     self.abcde)))
        self.assertFalse(self.empty < self.empty)
        self.assertTrue(self.empty < self.a < self.abc < self.abcde)
        self.assertFalse(self.abc < self.ace)
        self.assertFalse(self.abc < 0)
        self.assertTrue(0 < self.abc)

    def test_le(self):
        self.assertTrue(all(self.empty <= s
                            for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertTrue(self.empty <= self.empty)
        self.assertTrue(self.a <= self.abc <= self.abc <= self.abcde)
        self.assertFalse(self.abc <= self.ace)

    def test_le_num(self):
        self.assertFalse(any(s <= 0
                             for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertTrue(self.empty <= 0)
        self.assertTrue(all(s <= 0b11111
                            for s in (self.empty,
                                      self.a,
                                      self.abc,
                                      self.ace,
                                      self.abcde)))
        self.assertTrue(all(0 <= s
                            for s in (self.empty,
                                      self.a,
                                      self.abc,
                                      self.ace,
                                      self.abcde)))
        self.assertFalse(any(0b11111 <= s
                             for s in (self.empty, self.a, self.abc, self.ace)))


    def test_eq(self):
        self.assertTrue(self.empty == self.empty)
        self.assertTrue(self.a == self.a)
        self.assertFalse(self.abc == self.ace)
        self.assertTrue(self.empty == 0)
        self.assertTrue(self.a == 0b00001)
        self.assertTrue(self.abc == 0b00111)
        self.assertTrue(self.ace == 0b10101)
        self.assertTrue(self.abcde == 0b11111)
        self.assertTrue(all(s == s and s == n and n == s
                            for (s, n) in self.pairs))

    def test_ne(self):
        self.assertTrue(self.abc != self.ace)
        self.assertFalse(any(s != s or s != n or n != s
                             for (s, n) in self.pairs))


    def test_gt(self):
        self.assertTrue(all(s > self.empty
                            for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertFalse(any(self.empty > s
                             for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertFalse(self.empty > self.empty)
        self.assertTrue(self.abcde > self.abc > self.a > self.empty)
        self.assertFalse(self.abc > self.ace or self.ace > self.abc)
        self.assertTrue(self.abc > 0b00011)
        self.assertFalse(self.abc > 0b00111)
        self.assertFalse(0b00111 > self.abc)
        self.assertTrue(0b01111 > self.abc)

    def test_ge(self):
        self.assertTrue(all(s >= self.empty
                           for s in (self.empty,
                                     self.a,
                                     self.abc,
                                     self.ace,
                                     self.abcde)))
        self.assertFalse(any(self.empty >= s
                             for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertTrue(all(s >= s
                            for s in (self.empty,
                                      self.a,
                                      self.abc,
                                      self.ace,
                                      self.abcde)))
        self.assertTrue(self.abcde >= self.abc
                        >= self.abc >= self.a >= self.empty)
        self.assertFalse(self.abc >= self.ace or self.ace >= self.abc)
        self.assertTrue(self.abc >= 0b00111 and not self.abc >= 0b01011)
        self.assertTrue(0b00111 >= self.abc)
        self.assertFalse(0b01011 >= self.abc)

    def test_hash(self):
        unused = hash(self.abc)

    def test_bool(self):
        self.assertIs(bool(self.empty), False)
        self.assertTrue(all(bool(s) is True
                            for s in (self.a, self.abc, self.ace, self.abcde)))

    def test_len(self):
        self.assertEqual(len(self.empty), 0)
        self.assertEqual(len(self.a), 1)
        self.assertEqual(len(self.abc), 3)
        self.assertEqual(len(self.ace), 3)
        self.assertEqual(len(self.abcde), 5)

    def test_at(self):
        self.assertIs(self.ace.at(0), True)
        self.assertIs(self.ace.at(1), False)
        with self.assertRaises(IndexError):
            self.ace.at(5)
        with self.assertRaises(IndexError):
            self.ace.at(-1)

    def test_get(self):
        self.assertIs(self.ace.get('c'), True)
        self.assertIs(self.ace.get('b'), False)
        with self.assertRaises(ValueError):
            self.ace.get('f')

    def test_containment(self):
        self.assertFalse('a' in self.empty)
        self.assertTrue(all('a' in s
                            for s in (self.a, self.abc, self.ace, self.abcde)))
        self.assertTrue(all('c' in s
                            for s in (self.abc, self.ace, self.abcde)))
        self.assertTrue(all('d' not in s
                            for s in (self.empty, self.abc, self.ace)))

    def test_sub(self):
        self.assertEqual(self.abcde - self.ace, self.u.subset('bd'))
        self.assertEqual(self.abc - self.ace, self.u.subset('b'))
        self.assertEqual(self.abc - self.abcde, self.empty)
        self.assertEqual(self.abc - self.empty, self.abc)

    def test_and(self):
        self.assertEqual(self.abc & self.empty, self.empty)
        self.assertEqual(self.abc & self.abcde, self.abc)
        self.assertEqual(self.abc & self.abc, self.abc)
        self.assertEqual(self.abc & self.ace, self.u.subset('ac'))

    def test_xor(self):
        self.assertEqual(self.abc ^ self.empty, self.abc)
        self.assertEqual(self.abc ^ self.abcde, self.u.subset('de'))
        self.assertEqual(self.abc ^ self.abc, 0)
        self.assertEqual(self.abc ^ self.ace, self.u.subset('be'))

    def test_or(self):
        self.assertEqual(self.abc | self.empty, self.abc)
        self.assertEqual(self.abc | self.abcde, self.abcde)
        self.assertEqual(self.abc | self.abc, self.abc)
        self.assertEqual(self.abc | self.ace, self.u.subset('abce'))

    def test_isub(self):
        t = self.u.subset('abc')
        t -= self.ace
        self.assertEqual(t, self.u.subset('b'))

    def test_iand(self):
        t = self.u.subset('abc')
        t &= self.ace
        self.assertEqual(t, self.u.subset('ac'))

    def test_ixor(self):
        t = self.u.subset('abc')
        t ^= self.ace
        self.assertEqual(t, self.u.subset('be'))

    def test_ior(self):
        t = self.u.subset('abc')
        t |= self.ace
        self.assertEqual(t, self.u.subset('abce'))

    def test_invert(self):
        self.assertEqual(~self.abc, self.u.subset('de'))
        self.assertEqual(~self.empty, self.abcde)
        self.assertEqual(~self.abcde, self.empty)

    def test_add_method(self):
        t = self.u.subset('abc')
        t.add('d')
        self.assertEqual(t, self.u.subset('abcd'))

    def test_iters(self):
        self.assertListEqual(list(self.ace.iter_members()), ['a', 'c', 'e'])
        self.assertListEqual(list(self.ace.iter_indices()), [0, 2, 4])
        self.assertListEqual(list(self.ace.iter_masks()), [1, 4, 16])


class TestRelations(unittest.TestCase):

    def setUp(self):
        self.u1 = Universe(('Leo', 'Mich', 'Don', 'Raph'))
        self.u2 = Universe('ROYGBIV')
        self.r = Relation(self.u1, self.u2)
        self.r.add('Leo', 'B')
        self.r.add('Mich', 'O')
        self.r.add('Don', 'V')
        self.r.add('Raph', 'R')

    def test_containment(self):
        self.assertTrue(('Mich', 'O') in self.r)
        self.assertFalse(('Mich', 'R') in self.r)

    def test_at(self):
        self.assertEqual(self.r.at(3), self.u2.subset('R'))
        self.assertEqual(self.r.at(-2), self.u2.subset('V'))
        with self.assertRaises(IndexError):
            self.r.at(4)
        with self.assertRaises(IndexError):
            self.r.at(-5)

    def test_get(self):
        self.assertEqual(self.r.get('Don'), self.u2.subset('V'))
        with self.assertRaises(ValueError):
            self.r.get('Splinter')

    def test_iter(self):
        don = self.r.get('Don')
        self.assertListEqual(list(don.iter_members()), ['V'])
        self.assertListEqual(list(don.iter_indices()), [6])
        self.assertListEqual(list(don.iter_masks()), [64])


if __name__ == '__main__':
    unittest.main()
