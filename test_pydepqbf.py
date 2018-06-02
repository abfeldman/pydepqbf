"""Test the pydepqbf module.

Copyright 2018, Palo Alto Research Center
"""

import unittest

from pydepqbf import solve, \
    QDPLL_QTYPE_FORALL, QDPLL_QTYPE_EXISTS, \
    QDPLL_RESULT_SAT, QDPLL_RESULT_UNSAT


class TestSolve(unittest.TestCase):
    def test_basic_unsat(self):
        result = solve(((QDPLL_QTYPE_FORALL, (1, 2)),
                        (QDPLL_QTYPE_EXISTS, (3, 4))),
                       ((-1, -3),
                        (1, 2, 4),
                        (1, -4)))
        self.assertEqual(result, QDPLL_RESULT_UNSAT)

    def test_basic_sat(self):
        result = solve(((QDPLL_QTYPE_FORALL, (1, 2)),
                        (QDPLL_QTYPE_EXISTS, (3, 4))),
                       ((1, -4),))
        self.assertEqual(result, QDPLL_RESULT_SAT)


if __name__ == '__main__':
    unittest.main()
