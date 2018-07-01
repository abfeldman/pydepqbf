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
