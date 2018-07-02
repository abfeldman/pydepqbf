"""Test the pydepqbf module.

Copyright 2018, Palo Alto Research Center
"""

import unittest

from pydepqbf import solve, \
    QDPLL_QTYPE_FORALL, QDPLL_QTYPE_EXISTS, \
    QDPLL_RESULT_SAT, QDPLL_RESULT_UNSAT


class TestSolve(unittest.TestCase):
    def test_wrong_args(self):
        self.assertRaises(TypeError, solve)
        self.assertRaises(TypeError, solve, None)
        self.assertRaises(TypeError, solve, None, None)
        self.assertRaises(TypeError, solve, 1, 2)
        self.assertRaises(TypeError, solve, 1.0, 2.0)
        self.assertRaises(TypeError, solve, 'a', 'b')
        self.assertRaises(TypeError, solve, object(), object())
        self.assertRaises(TypeError, solve, [()], [])
        self.assertRaises(TypeError, solve, [(QDPLL_RESULT_SAT, (), ())], [])
        self.assertRaises(TypeError, solve, (('a', (1, 2)), (QDPLL_QTYPE_EXISTS, (3, 4))), ((1, -4),))
        self.assertRaises(TypeError, solve, ((1.0, (1, 2)), (QDPLL_QTYPE_EXISTS, (3, 4))), ((1, -4),))

    def test_empty(self):
        result = solve([], [])
        self.assertEqual(result, (QDPLL_RESULT_SAT, []))

    def test_basic_qbf_unsat(self):
        result = solve(((QDPLL_QTYPE_FORALL, (1, 2)),
                        (QDPLL_QTYPE_EXISTS, (3, 4))),
                       ((-1, -3),
                        (1, 2, 4),
                        (1, -4)))
        self.assertEqual(result, (QDPLL_RESULT_UNSAT, [-1, -2]))

    def test_basic_qbf_sat(self):
        result = solve(((QDPLL_QTYPE_FORALL, (1, 2)),
                        (QDPLL_QTYPE_EXISTS, (3, 4))),
                       ((1, -4),))
        self.assertEqual(result, (QDPLL_RESULT_SAT, []))

    def test_basic_sat(self):
        result = solve((),
                       ((-1, 2), (-1, -2), (1, -2)))
        self.assertEqual(result, (QDPLL_RESULT_SAT, [-1, -2]))


if __name__ == '__main__':
    unittest.main()
