/*
 * Copyright (c) 2018, Alexander Feldman, PARC Inc.
 * Python bindings to DepQBF (http://lonsing.github.io/depqbf/)
 * This file is published under the same license as DepQBF.
 */
#define PYDEPQBF_URL "https://github.com/abfeldman/pydepqbf/"
#define DEPQBF_URL "http://lonsing.github.io/depqbf/"

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
#define PyUnicode_FromString PyString_FromString
#endif

#include <qdpll.h>

typedef struct
{
    QDPLLQuantifierType type;
    Py_ssize_t len;
    VarID *vars;
} Quantifier;

static int add_quantifier(QDPLL *qdpll,
                          PyObject *quantifier,
                          Nesting nesting,
                          Quantifier **outermost_quantifier)
{
    PyObject *iterator;         /* each quantifier is an iterable of variables */
    PyObject *var;              /* the variables are integers */
    PyObject *type;
    PyObject *vars;
    Py_ssize_t idx = 0;

    int v;
    int t;

    if (!PySequence_Check(quantifier) ||
        PySequence_Length(quantifier) != 2) {
        PyErr_SetString(PyExc_TypeError, "quantifier sequence of size two expected");

        return -1;
    }

    type = PySequence_GetItem(quantifier, 0);
    if (!IS_INT(type))  {
        PyErr_SetString(PyExc_TypeError, "integer expected");

        return -1;
    }
    
    vars = PySequence_GetItem(quantifier, 1);
    t = PyLong_AsLong(type);

    if (nesting == 1) {
        *outermost_quantifier = PyMem_Malloc(sizeof(Quantifier));
        if (*outermost_quantifier == NULL) {
            return -1;
        }
        (*outermost_quantifier)->type = t;
        (*outermost_quantifier)->len = PySequence_Length(vars);
        (*outermost_quantifier)->vars = PyMem_Malloc(sizeof(VarID) * (*outermost_quantifier)->len);
    }

    qdpll_new_scope_at_nesting(qdpll, t, nesting);

    iterator = PyObject_GetIter(vars);
    if (iterator == NULL) {
        return -1;
    }

    while ((var = PyIter_Next(iterator)) != NULL) {
        if (!IS_INT(var))  {
            Py_DECREF(var);
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_TypeError, "integer expected");

            return -1;
        }
        v = PyLong_AsLong(var);
        Py_DECREF(var);
        if (v == 0) {
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "non-zero integer expected");

            return -1;
        }
        if (nesting == 1) {
            (*outermost_quantifier)->vars[idx] = v;
            idx += 1;
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

static int add_quantifiers(QDPLL *qdpll,
                           PyObject *quantifiers,
                           Quantifier **outermost_quantifier)
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
        if (add_quantifier(qdpll,
                           item,
                           nesting,
                           outermost_quantifier) < 0) {
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

static QDPLL *setup_depqbf(PyObject *args,
                           PyObject *kwds,
                           Quantifier **outermost_quantifier)
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

    qdpll_configure(qdpll, "--no-dynamic-nenofex");

    if (add_quantifiers(qdpll,
                        quantifiers,
                        outermost_quantifier) < 0) {
        return NULL;
    }

    if (add_clauses(qdpll, clauses) < 0) {
        return NULL;
    }

    return qdpll;
}

static PyObject *get_pcert(QDPLL *qdpll,
                           Quantifier *outermost_quantifier,
                           QDPLLResult is_sat)
{
    PyObject *list;
    VarID max_idx;
    VarID idx;
    VarID i;
    QDPLLAssignment v;

    list = PyList_New(0);
    if (list == NULL) {
        return NULL;
    }

    if (outermost_quantifier == NULL) {
        max_idx = qdpll_get_max_declared_var_id(qdpll);
        for (i = 1; i <= max_idx; i++) {
            v = qdpll_get_value(qdpll, i);

            if (v == QDPLL_ASSIGNMENT_FALSE) {
                if (PyList_Append(list,
                                  PyInt_FromLong(-(long)i)) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
            if (v == QDPLL_ASSIGNMENT_TRUE) {
                if (PyList_Append(list,
                                  PyInt_FromLong((long)i)) < 0) {
                    Py_DECREF(list);
                    return NULL;
                }
            }
        }
        
        return list;
    }

    if ((is_sat == QDPLL_RESULT_SAT && outermost_quantifier->type != QDPLL_QTYPE_EXISTS) ||
        (is_sat == QDPLL_RESULT_UNSAT && outermost_quantifier->type != QDPLL_QTYPE_FORALL)) {
        return list;
    }

    max_idx = outermost_quantifier->len;
    for (idx = 0; idx < max_idx; idx++) {
        i = outermost_quantifier->vars[idx];
        v = qdpll_get_value(qdpll, i);

        if (v == QDPLL_ASSIGNMENT_FALSE) {
            if (PyList_Append(list,
                              PyInt_FromLong(-(long)i)) < 0) {
                Py_DECREF(list);
                return NULL;
            }
        }
        if (v == QDPLL_ASSIGNMENT_TRUE) {
            if (PyList_Append(list,
                              PyInt_FromLong((long)i)) < 0) {
                Py_DECREF(list);
                return NULL;
            }
        }
    }

    return list;
}

static PyObject *solve(PyObject *self,
                       PyObject *args,
                       PyObject *kwds)
{
    QDPLL *qdpll;

    PyObject *result = NULL;
    QDPLLResult is_sat;
    PyObject *pcert = NULL;

    Quantifier *outermost_quantifier = NULL;

    qdpll = setup_depqbf(args,
                         kwds,
                         &outermost_quantifier);
    if (qdpll == NULL) {
        return NULL;
    }
    is_sat = qdpll_sat(qdpll);

    result = PyTuple_New(2);
    if (result == NULL) {
        return NULL;
    }
    if (PyTuple_SetItem(result,
                        (Py_ssize_t)0,
                        PyInt_FromLong(is_sat)) < 0) {
        Py_DECREF(result);
        return NULL;
    }
    switch (is_sat) {
        case QDPLL_RESULT_SAT:
        case QDPLL_RESULT_UNSAT:
            pcert = get_pcert(qdpll, outermost_quantifier, is_sat);
            break;
        case QDPLL_RESULT_UNKNOWN:
            break;
        default:
            PyErr_Format(PyExc_SystemError, "pydepqbf return value: %d", is_sat);
    }
    
    if (PyTuple_SetItem(result,
                        (Py_ssize_t)1,
                        pcert) < 0) {
        Py_DECREF(result);
        return NULL;
    }

    if (outermost_quantifier != NULL) {
        PyMem_Free(outermost_quantifier->vars);
        PyMem_Free(outermost_quantifier);
    }
        
    qdpll_delete(qdpll);

    return result;
}

PyDoc_STRVAR(solve_doc,
"solve(quantifiers, clauses) -> (is_sat, pcert)\n\
\n\
Solve the QBF problem for the clauses, and return a tuple\n\
(is_sat, pcert). The first element of the result tuple (is_sat)\n\
is either QDPLL_RESULT_SAT, or QDPLL_RESULT_UNSAT, or\n\
QDPLL_RESULT_UNKNOWN. The second element is a partial certificate\n\
(pcert) and is a list of integers or None.\n\
Please see " DEPQBF_URL " and \n\
" PYDEPQBF_URL " for more details.");

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
