/*
 * Copyright (c) 2018, Alexander Feldman, PARC Inc.
 * Python bindings to DepQBF (http://lonsing.github.io/depqbf/)
 * This file is published under the same license as DepQBF.
 */
#define PYDEPQBF_URL  "http://lonsing.github.io/depqbf/"

#include <Python.h>

#ifdef _MSC_VER
#define NGETRUSAGE
#define inline __inline
#endif

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#define PyInt_FromLong  PyLong_FromLong
#define IS_INT(x)  (PyLong_Check(x))
#else
#define IS_INT(x)  (PyInt_Check(x) || PyLong_Check(x))
#endif

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 5
#define PyUnicode_FromString  PyString_FromString
#endif

#include <qdpll.h>

inline static void *py_malloc(void *mmgr, size_t bytes)
{
    return PyMem_Malloc(bytes);
}

inline static void *py_realloc(void *mmgr, void *ptr, size_t old, size_t new)
{
    return PyMem_Realloc(ptr, new);
}

inline static void py_free(void *mmgr, void *ptr, size_t bytes)
{
    PyMem_Free(ptr);
}

static int add_quantifier(QDPLL *qdpll, PyObject *quantifier, Nesting nesting)
{
    PyObject *iterator;         /* each clause is an iterable of literals */
    PyObject *lit;              /* the literals are integers */
    PyObject *type;
    PyObject *lits;
/*
    Py_ssize_t l;
    int f;
*/
    int v;
    int t;

/*
    f = PySequence_Check(quantifier);
    l = PySequence_Length(quantifier);
*/

    type = PySequence_GetItem(quantifier, 0);
    lits = PySequence_GetItem(quantifier, 1);
/*
    IS_INT(type);
*/
    t = PyLong_AsLong(type);

    qdpll_new_scope_at_nesting(qdpll, t, nesting);

    iterator = PyObject_GetIter(lits);
    if (iterator == NULL) {
        return -1;
    }

    while ((lit = PyIter_Next(iterator)) != NULL) {
        if (!IS_INT(lit))  {
            Py_DECREF(lit);
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_TypeError, "integer expected");

            return -1;
        }
        v = PyLong_AsLong(lit);
        Py_DECREF(lit);
        if (v == 0) {
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "non-zero integer expected");

            return -1;
        }
        qdpll_add(qdpll, v);
    }

    Py_DECREF(iterator);
    if (PyErr_Occurred()) {
        return -1;
    }

    qdpll_add(qdpll, 0);

    return 0;
}

static int add_clause(QDPLL *qdpll, PyObject *clause)
{
    PyObject *iterator;         /* each clause is an iterable of literals */
    PyObject *lit;              /* the literals are integers */
    int v;

    iterator = PyObject_GetIter(clause);
    if (iterator == NULL) {
        return -1;
    }

    while ((lit = PyIter_Next(iterator)) != NULL) {
        if (!IS_INT(lit))  {
            Py_DECREF(lit);
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_TypeError, "integer expected");

            return -1;
        }
        v = PyLong_AsLong(lit);
        Py_DECREF(lit);
        if (v == 0) {
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "non-zero integer expected");

            return -1;
        }
        qdpll_add(qdpll, v);
    }

    Py_DECREF(iterator);
    if (PyErr_Occurred()) {
        return -1;
    }

    qdpll_add(qdpll, 0);

    return 0;
}

static int add_quantifiers(QDPLL *qdpll, PyObject *quantifiers)
{
    PyObject *iterator;       /* clauses can be any iterable */
    PyObject *item;           /* each clause is an iterable of intergers */
    Nesting nesting;

    nesting = 1;

    iterator = PyObject_GetIter(quantifiers);
    if (iterator == NULL) {
        return -1;
    }

    while ((item = PyIter_Next(iterator)) != NULL) {
        if (add_quantifier(qdpll, item, nesting) < 0) {
            Py_DECREF(item);
            Py_DECREF(iterator);

            return -1;
        }
        Py_DECREF(item);

        nesting += 1;
    }

    Py_DECREF(iterator);
    if (PyErr_Occurred()) {
        return -1;
    }

    return 0;
}

static int add_clauses(QDPLL *qdpll, PyObject *clauses)
{
    PyObject *iterator;       /* clauses can be any iterable */
    PyObject *item;           /* each clause is an iterable of intergers */

    iterator = PyObject_GetIter(clauses);
    if (iterator == NULL) {
        return -1;
    }

    while ((item = PyIter_Next(iterator)) != NULL) {
        if (add_clause(qdpll, item) < 0) {
            Py_DECREF(item);
            Py_DECREF(iterator);

            return -1;
        }
        Py_DECREF(item);
    }

    Py_DECREF(iterator);
    if (PyErr_Occurred()) {
        return -1;
    }

    return 0;
}

static QDPLL *setup_depqbf(PyObject *args, PyObject *kwds)
{
    QDPLL *qdpll;

    PyObject *quantifiers;      /* iterable of quantifiers */
    PyObject *clauses;          /* iterable of clauses */

    static char *kwlist[] = { "quantifiers",
                              "clauses",
                              NULL };

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwds,
                                     "OO:(iter)solve",
                                     kwlist,
                                     &quantifiers,
                                     &clauses)) {
        return NULL;
    }

    qdpll = qdpll_create();
    if (qdpll == NULL) {
        return NULL;
    }
/*
    qdpll_configure(qdpll, "--dep-man=simple");
*/

    if (add_quantifiers(qdpll, quantifiers) < 0) {
        return NULL;
    }

    if (add_clauses(qdpll, clauses) < 0) {
        return NULL;
    }

    return qdpll;
}

static PyObject *solve(PyObject *self, PyObject *args, PyObject *kwds)
{
    QDPLL *qdpll;

    PyObject *result = NULL;
    QDPLLResult rc;

    qdpll = setup_depqbf(args, kwds);
/*
    qdpll_print(qdpll, stdout);
*/
    rc = qdpll_sat(qdpll);

    result = PyInt_FromLong(rc);

    qdpll_delete(qdpll);

    return result;
}

PyDoc_STRVAR(solve_doc,
"solve(clauses [, kwargs]) -> list\n\
\n\
Solve the QBF problem for the clauses, and return a solution as a\n\
list of integers, or one of the strings \"UNSAT\", \"UNKNOWN\".\n\
Please see " PYDEPQBF_URL " for more details.");

/* Method definitions */

/* declaration of methods supported by this module */
static PyMethodDef module_functions[] = {
    {"solve", (PyCFunction)solve, METH_VARARGS | METH_KEYWORDS, solve_doc},
    {NULL,    NULL}  /* sentinel */
};

PyDoc_STRVAR(module_doc, "\
pydepqbf: bindings to depqbf\n\
============================\n\n\
Please see " PYDEPQBF_URL " for more details.");

/* initialization routine for the shared libary */
#ifdef IS_PY3K
static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "pydepqbf", module_doc, -1, module_functions,
};

PyMODINIT_FUNC PyInit_pydepqbf(void)
#else
PyMODINIT_FUNC initpydepqbf(void)
#endif
{
    PyObject *m;

#ifdef IS_PY3K
    m = PyModule_Create(&moduledef);
    if (m == NULL) {
        return NULL;
    }
#else
    m = Py_InitModule3("pydepqbf", module_functions, module_doc);
    if (m == NULL) {
        return;
    }
#endif

    PyModule_AddIntConstant(m, "QDPLL_QTYPE_FORALL", QDPLL_QTYPE_FORALL);
    PyModule_AddIntConstant(m, "QDPLL_QTYPE_EXISTS", QDPLL_QTYPE_EXISTS);

    PyModule_AddIntConstant(m, "QDPLL_RESULT_UNKNOWN", QDPLL_RESULT_UNKNOWN);
    PyModule_AddIntConstant(m, "QDPLL_RESULT_SAT", QDPLL_RESULT_SAT);
    PyModule_AddIntConstant(m, "QDPLL_RESULT_UNSAT", QDPLL_RESULT_UNSAT);

    PyModule_AddIntConstant(m, "QDPLL_ASSIGNMENT_FALSE", QDPLL_ASSIGNMENT_FALSE);
    PyModule_AddIntConstant(m, "QDPLL_ASSIGNMENT_UNDEF", QDPLL_ASSIGNMENT_UNDEF);
    PyModule_AddIntConstant(m, "QDPLL_ASSIGNMENT_TRUE", QDPLL_ASSIGNMENT_TRUE);

#ifdef PYDEPQBF_VERSION
    PyModule_AddObject(m, "__version__", PyUnicode_FromString(PYDEPQBF_VERSION));
#endif

#ifdef IS_PY3K
    return m;
#endif
}
