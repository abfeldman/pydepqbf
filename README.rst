===========================================
pydepqbf: bindings to DepQBF (a QBF solver)
===========================================

Authors:
--------

A lot of Python binding code and documentation has been lifted from PycoSAT
<https://pypi.python.org/pypi/pycosat>

DepQBF is owned by Florian Lonsing <http://lonsing.github.io/depqbf/>

The pydepqbf Python module is distributed under the same terms as DepQBF.

Abstract
--------

`DepQBF <http://lonsing.github.io/depqbf/>`_ is an efficient
`QBF <https://en.wikipedia.org/wiki/True_quantified_Boolean_formula>`_ solver
written by Florian Lonsing in pure C.
This package provides efficient Python bindings to DepQBF on the C level,
i.e., when importing pydepqbf, the DepQBF solver becomes part of the
Python process itself.

Usage
-----

The ``pydepqbf`` module has one function: ``solve``.

The function ``solve`` returns a tuple containing two elements. The
first element is an integer, that is either QDPLL_RESULT_SAT or
QDPLL_RESULT_UNSAT or QDPLL_RESULT_UNKNOWN. The second element
contains a partial certificate for the input problem. If the outermost
(leftmost) quantifier block of a satisfiable QBF is existentially
quantified, then the partial certificate is an assignment to the
variables of this block (and dual for unsatisfiable QBFs and universal
variables from the outermost block, if that block is universally
quantified). The partial certificate is None if the outermost block of
a satisfiable QBF is universally quantified or if the outermost block
of an unsatisfiable QBF is existentially quantified.

Example
-------

Let us consider the following problem, represented by using
the QDIMACS `QDIMACS <http://www.qbflib.org/qdimacs.html>`_
format::

   p cnf 5 3
   a 1 2 0
   e 3 4 0
   -1 -3 0
   1 2 4 0
   1 -4 0

Here, we have 4 variables and 3 clauses, the first clause being
(not x\ :sub:`1`  or not x\ :sub:`3`).
In Python, each clause is
most conveniently represented as a list of integers.  Naturally, it makes
sense to represent each solution also as a list of integers, where the sign
corresponds to the Boolean value (+ for True and - for False) and the
absolute value corresponds to i\ :sup:`th` variable::

   >>> from pydepqbf import solve, QDPLL_QTYPE_FORALL, QDPLL_QTYPE_EXISTS
   >>> quantifiers = ((QDPLL_QTYPE_FORALL, (1, 2)), (QDPLL_QTYPE_EXISTS, (3, 4)))
   >>> clauses = ((-1, -3), (1, 2, 4), (1, -4))
   >>> solve(quantifiers, clauses)
   (20, [-1, -2])

The first element of the resulting pair is 20 (defined as
QDPLL_RESULT_UNSAT in the pydepqbf package) and the partial
certificate translates to: x\ :sub:`1` = x\ :sub:`2` = False.

