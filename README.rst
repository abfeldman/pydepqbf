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
