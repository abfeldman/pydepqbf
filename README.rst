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

The second element of the result pair contains a partial certificate
for the input problem. If the outermost (leftmost) quantifier block of
a satisfiable QBF is existentially quantified, then the partial
certificate is an assignment to the variables of this block (and dual
for unsatisfiable QBFs and universal variables from the outermost
block, if that block is universally quantified). The partial
certificate is None if the outermost block of a satisfiable QBF is
universally quantified or if the outermost block of an unsatisfiable
QBF is existentially quantified.

Example
-------

Let us consider the following clauses, represented using
the DIMACS `cnf <http://en.wikipedia.org/wiki/Conjunctive_normal_form>`_
format::

   p cnf 5 3
   1 -5 4 0
   -1 5 3 4 0
   -3 -4 0

Here, we have 5 variables and 3 clauses, the first clause being
(x\ :sub:`1`  or not x\ :sub:`5` or x\ :sub:`4`).
Note that the variable x\ :sub:`2` is not used in any of the clauses,
which means that for each solution with x\ :sub:`2` = True, we must
also have a solution with x\ :sub:`2` = False.  In Python, each clause is
most conveniently represented as a list of integers.  Naturally, it makes
sense to represent each solution also as a list of integers, where the sign
corresponds to the Boolean value (+ for True and - for False) and the
absolute value corresponds to i\ :sup:`th` variable::

   >>> from pydepqbf import solve
   >>> qcnf = [[1, -5, 4], [-1, 5, 3, 4], [-3, -4]]
   >>> solve(qcnf)
   [1, -2, -3, -4, 5]

This solution translates to: x\ :sub:`1` = x\ :sub:`5` = True,
x\ :sub:`2` = x\ :sub:`3` = x\ :sub:`4` = False
