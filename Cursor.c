//-----------------------------------------------------------------------------
// Cursor.c
//   Definition of the Python type OracleCursor.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type "Cursor"
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    OCIStmt *handle;
    udt_Connection *connection;
    udt_Environment *environment;
    PyObject *statement;
    PyObject *bindVariables;
    PyObject *fetchVariables;
    PyObject *rowFactory;
    int arraySize;
    int bindArraySize;
    int fetchArraySize;
    int numbersAsStrings;
    int setInputSizes;
    int outputSize;
    int outputSizeColumn;
    int rowCount;
    int actualRows;
    int rowNum;
    int statementType;
    int isDML;
    int isOpen;
    int isOwned;
} udt_Cursor;


//-----------------------------------------------------------------------------
// dependent function defintions
//-----------------------------------------------------------------------------
static void Cursor_Free(udt_Cursor*);


//-----------------------------------------------------------------------------
// functions for the Python type "Cursor"
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetIter(udt_Cursor*);
static PyObject *Cursor_GetNext(udt_Cursor*);
static PyObject *Cursor_Close(udt_Cursor*, PyObject*);
static PyObject *Cursor_CallFunc(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_CallProc(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Execute(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_ExecuteMany(udt_Cursor*, PyObject*);
static PyObject *Cursor_ExecuteManyPrepared(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchOne(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchMany(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_FetchAll(udt_Cursor*, PyObject*);
static PyObject *Cursor_FetchRaw(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Parse(udt_Cursor*, PyObject*);
static PyObject *Cursor_Prepare(udt_Cursor*, PyObject*);
static PyObject *Cursor_SetInputSizes(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_SetOutputSize(udt_Cursor*, PyObject*);
static PyObject *Cursor_Var(udt_Cursor*, PyObject*);
static PyObject *Cursor_ArrayVar(udt_Cursor*, PyObject*);
static PyObject *Cursor_BindNames(udt_Cursor*, PyObject*);
static PyObject *Cursor_GetDescription(udt_Cursor*, void*);
static PyObject *Cursor_New(PyTypeObject*, PyObject*, PyObject*);
static int Cursor_Init(udt_Cursor*, PyObject*, PyObject*);
static PyObject *Cursor_Repr(udt_Cursor*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "Cursor"
//-----------------------------------------------------------------------------
static PyMethodDef g_CursorMethods[] = {
    { "execute", (PyCFunction) Cursor_Execute, METH_VARARGS | METH_KEYWORDS },
    { "fetchall", (PyCFunction) Cursor_FetchAll, METH_NOARGS },
    { "fetchone", (PyCFunction) Cursor_FetchOne, METH_NOARGS },
    { "fetchmany", (PyCFunction) Cursor_FetchMany,
              METH_VARARGS | METH_KEYWORDS },
    { "fetchraw", (PyCFunction) Cursor_FetchRaw,
              METH_VARARGS | METH_KEYWORDS },
    { "prepare", (PyCFunction) Cursor_Prepare, METH_VARARGS },
    { "parse", (PyCFunction) Cursor_Parse, METH_VARARGS },
    { "setinputsizes", (PyCFunction) Cursor_SetInputSizes,
              METH_VARARGS | METH_KEYWORDS },
    { "executemany", (PyCFunction) Cursor_ExecuteMany, METH_VARARGS },
    { "callproc", (PyCFunction) Cursor_CallProc,
              METH_VARARGS  | METH_KEYWORDS },
    { "callfunc", (PyCFunction) Cursor_CallFunc,
              METH_VARARGS  | METH_KEYWORDS },
    { "executemanyprepared", (PyCFunction) Cursor_ExecuteManyPrepared,
              METH_VARARGS },
    { "setoutputsize", (PyCFunction) Cursor_SetOutputSize, METH_VARARGS },
    { "var", (PyCFunction) Cursor_Var, METH_VARARGS },
    { "arrayvar", (PyCFunction) Cursor_ArrayVar, METH_VARARGS },
    { "bindnames", (PyCFunction) Cursor_BindNames, METH_NOARGS },
    { "close", (PyCFunction) Cursor_Close, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "Cursor"
//-----------------------------------------------------------------------------
static PyMemberDef g_CursorMembers[] = {
    { "arraysize", T_INT, offsetof(udt_Cursor, arraySize), 0 },
    { "bindarraysize", T_INT, offsetof(udt_Cursor, bindArraySize), 0 },
    { "rowcount", T_INT, offsetof(udt_Cursor, rowCount), READONLY },
    { "statement", T_OBJECT, offsetof(udt_Cursor, statement), READONLY },
    { "connection", T_OBJECT_EX, offsetof(udt_Cursor, connection), READONLY },
    { "numbersAsStrings", T_INT, offsetof(udt_Cursor, numbersAsStrings), 0 },
    { "rowfactory", T_OBJECT, offsetof(udt_Cursor, rowFactory), 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "Connection"
//-----------------------------------------------------------------------------
static PyGetSetDef g_CursorCalcMembers[] = {
    { "description", (getter) Cursor_GetDescription, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type "Cursor"
//-----------------------------------------------------------------------------
static PyTypeObject g_CursorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "OracleCursor",                     // tp_name
    sizeof(udt_Cursor),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Cursor_Free,           // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Cursor_Repr,             // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                        // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    (getiterfunc) Cursor_GetIter,       // tp_iter
    (iternextfunc) Cursor_GetNext,      // tp_iternext
    g_CursorMethods,                    // tp_methods
    g_CursorMembers,                    // tp_members
    g_CursorCalcMembers,                // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    (initproc) Cursor_Init,             // tp_init
    0,                                  // tp_alloc
    Cursor_New,                         // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// Cursor_AllocateHandle()
//   Allocate a new handle.
//-----------------------------------------------------------------------------
static int Cursor_AllocateHandle(
    udt_Cursor *self)                   // cursor object
{
    sword status;

    self->isOwned = 1;
    status = OCIHandleAlloc(self->environment->handle,
            (dvoid**) &self->handle, OCI_HTYPE_STMT, 0, 0);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_New()") < 0)
        return -1;

    return 0;
}

//-----------------------------------------------------------------------------
// Cursor_FreeHandle()
//   Free the handle which may be reallocated if necessary.
//-----------------------------------------------------------------------------
static int Cursor_FreeHandle(
    udt_Cursor *self,                   // cursor object
    int raiseException)                 // raise an exception, if necesary?
{
#ifdef ORACLE_9I
    sword status;
#endif

    if (self->handle) {
#ifdef ORACLE_9I
        if (self->isOwned) {
#endif
            OCIHandleFree(self->handle, OCI_HTYPE_STMT);
#ifdef ORACLE_9I
        } else {
            if (self->connection->handle != 0) {
                status = OCIStmtRelease(self->handle,
                        self->environment->errorHandle, NULL, 0, OCI_DEFAULT);
                if (raiseException && Environment_CheckForError(
                        self->environment, status, "Cursor_FreeHandle()") < 0)
                    return -1;
            }
        }
#endif
    }
    return 0;
}


#include "Variable.c"


//-----------------------------------------------------------------------------
// Cursor_IsOpen()
//   Determines if the cursor object is open and if so, if the connection is
// also open.
//-----------------------------------------------------------------------------
static int Cursor_IsOpen(
    udt_Cursor *self)                   // cursor to check
{
    if (!self->isOpen) {
        PyErr_SetString(g_InterfaceErrorException, "not open");
        return -1;
    }
    return Connection_IsConnected(self->connection);
}


//-----------------------------------------------------------------------------
// Cursor_New()
//   Create a new cursor object.
//-----------------------------------------------------------------------------
static PyObject *Cursor_New(
    PyTypeObject *type,                 // type object
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    return type->tp_alloc(type, 0);
}


//-----------------------------------------------------------------------------
// Cursor_Init()
//   Create a new cursor object.
//-----------------------------------------------------------------------------
static int Cursor_Init(
    udt_Cursor *self,                   // cursor object
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    udt_Connection *connection;

    // parse arguments
    if (!PyArg_ParseTuple(args, "O!", &g_ConnectionType, &connection))
        return -1;

    // initialize members
    Py_INCREF(connection);
    self->connection = connection;
    self->environment = connection->environment;
    self->arraySize = 1;
    self->bindArraySize = 1;
    self->statementType = -1;
    self->outputSize = -1;
    self->outputSizeColumn = -1;

#ifndef ORACLE_9I
    // Oracle 8i must always allocate a handle
    if (Cursor_AllocateHandle(self) < 0)
        return -1;
#endif

    // mark cursor as open
    self->isOpen = 1;

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_Repr()
//   Return a string representation of the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Repr(
    udt_Cursor *cursor)                 // cursor to return the string for
{
    PyObject *connectionRepr, *module, *name, *result;

    connectionRepr = PyObject_Repr((PyObject*) cursor->connection);
    if (!connectionRepr)
        return NULL;
    if (GetModuleAndName(cursor->ob_type, &module, &name) < 0) {
        Py_DECREF(connectionRepr);
        return NULL;
    }
    result = PyString_FromFormat("<%s.%s on %s>",
            PyString_AS_STRING(module), PyString_AS_STRING(name),
            PyString_AS_STRING(connectionRepr));
    Py_DECREF(connectionRepr);
    Py_DECREF(module);
    Py_DECREF(name);
    return result;
}


//-----------------------------------------------------------------------------
// Cursor_Free()
//   Deallocate the cursor.
//-----------------------------------------------------------------------------
static void Cursor_Free(
    udt_Cursor *self)                   // cursor object
{
    Cursor_FreeHandle(self, 0);
    Py_XDECREF(self->statement);
    Py_XDECREF(self->bindVariables);
    Py_XDECREF(self->fetchVariables);
    Py_XDECREF(self->connection);
    Py_XDECREF(self->rowFactory);
    self->ob_type->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Cursor_GetBindNames()
//   Return a list of bind variable names. At this point the cursor must have
// already been prepared.
//-----------------------------------------------------------------------------
static int Cursor_GetBindNames(
    udt_Cursor *self,                   // cursor to get information from
    int numElements,                    // number of elements (IN/OUT)
    PyObject **names)                   // list of names (OUT)
{
    ub1 *bindNameLengths, *indicatorNameLengths, *duplicate;
    char *buffer, **bindNames, **indicatorNames;
    OCIBind **bindHandles;
    int elementSize, i;
    sb4 foundElements;
    PyObject *temp;
    sword status;

    // ensure that a statement has already been prepared
    if (!self->statement) {
        PyErr_SetString(g_ProgrammingErrorException,
                "statement must be prepared first");
        return -1;
    }

    // avoid bus errors on 64-bit platforms
    numElements = numElements + (sizeof(void*) - numElements % sizeof(void*));

    // initialize the buffers
    elementSize = sizeof(char*) + sizeof(ub1) + sizeof(char*) + sizeof(ub1) +
            sizeof(ub1) + sizeof(OCIBind*);
    buffer = (char*) PyMem_Malloc(numElements * elementSize);
    if (!buffer) {
        PyErr_NoMemory();
        return -1;
    }
    bindNames = (char**) buffer;
    bindNameLengths = (ub1*) (((char*) bindNames) +
            sizeof(char*) * numElements);
    indicatorNames = (char**) (((char*) bindNameLengths) +
            sizeof(ub1) * numElements);
    indicatorNameLengths = (ub1*) (((char*) indicatorNames) +
            sizeof(char*) * numElements);
    duplicate = (ub1*) (((char*) indicatorNameLengths) +
            sizeof(ub1) * numElements);
    bindHandles = (OCIBind**) (((char*) duplicate) +
            sizeof(ub1) * numElements);

    // get the bind information
    status = OCIStmtGetBindInfo(self->handle,
            self->environment->errorHandle, numElements, 1, &foundElements,
            (text**) bindNames, bindNameLengths, (text**) indicatorNames,
            indicatorNameLengths, duplicate, bindHandles);
    if (status != OCI_NO_DATA &&
            Environment_CheckForError(self->environment, status,
            "Cursor_GetBindNames()") < 0)
        return -1;
    if (foundElements < 0) {
        *names = NULL;
        PyMem_Free(buffer);
        return abs(foundElements);
    }

    // create the list which is to be returned
    *names = PyList_New(0);
    if (!*names) {
        PyMem_Free(buffer);
        return -1;
    }

    // process the bind information returned
    for (i = 0; i < foundElements; i++) {
        if (!duplicate[i]) {
            temp = PyString_FromStringAndSize(bindNames[i],
                    bindNameLengths[i]);
            if (!temp) {
                Py_DECREF(*names);
                PyMem_Free(buffer);
                return -1;
            }
            if (PyList_Append(*names, temp) < 0) {
                Py_DECREF(*names);
                Py_DECREF(temp);
                PyMem_Free(buffer);
                return -1;
            }
            Py_DECREF(temp);
        }
    }
    PyMem_Free(buffer);

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_PerformDefine()
//   Perform the defines for the cursor. At this point it is assumed that the
// statement being executed is in fact a query.
//-----------------------------------------------------------------------------
static int Cursor_PerformDefine(
    udt_Cursor *self)                   // cursor to perform define on
{
    int numParams, pos;
    udt_Variable *var;
    sword status;

    // determine number of items in select-list
    status = OCIAttrGet(self->handle, OCI_HTYPE_STMT, (dvoid*) &numParams, 0,
            OCI_ATTR_PARAM_COUNT, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_PerformDefine()") < 0)
        return -1;

    // create a list corresponding to the number of items
    self->fetchVariables = PyList_New(numParams);
    if (!self->fetchVariables)
        return -1;

    // define a variable for each select-item
    self->fetchArraySize = self->arraySize;
    for (pos = 1; pos <= numParams; pos++) {
        var = Variable_Define(self, self->fetchArraySize, pos);
        if (!var)
            return -1;
        PyList_SET_ITEM(self->fetchVariables, pos - 1, (PyObject *) var);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_SetRowCount()
//   Set the rowcount variable.
//-----------------------------------------------------------------------------
static int Cursor_SetRowCount(
    udt_Cursor *self)                   // cursor to set the rowcount on
{
    ub4 rowCount;
    sword status;

    if (self->statementType == OCI_STMT_SELECT) {
        self->rowCount = 0;
        self->actualRows = -1;
        self->rowNum = 0;
    } else if (self->statementType == OCI_STMT_INSERT ||
               self->statementType == OCI_STMT_UPDATE ||
               self->statementType == OCI_STMT_DELETE) {
        status = OCIAttrGet(self->handle, OCI_HTYPE_STMT, &rowCount, 0,
                OCI_ATTR_ROW_COUNT, self->environment->errorHandle);
        if (Environment_CheckForError(self->environment, status,
                "Cursor_SetRowCount()") < 0)
            return -1;
        self->rowCount = rowCount;
    } else {
        self->rowCount = -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_InternalExecute()
//   Perform the work of executing a cursor and set the rowcount appropriately
// regardless of whether an error takes place.
//-----------------------------------------------------------------------------
static int Cursor_InternalExecute(
    udt_Cursor *self,                   // cursor to perform the execute on
    ub4 numIters)                       // number of iterations to execute
{
    sword status;
    ub4 mode;

    if (self->connection->autocommit)
        mode = OCI_COMMIT_ON_SUCCESS;
    else mode = OCI_DEFAULT;

    Py_BEGIN_ALLOW_THREADS
    status = OCIStmtExecute(self->connection->handle, self->handle,
            self->environment->errorHandle, numIters, 0, 0, 0, mode);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(self->environment, status,
            "Cursor_InternalExecute()") < 0) {
        if (Cursor_SetRowCount(self) < 0)
            PyErr_Clear();
        return -1;
    }
    return Cursor_SetRowCount(self);
}


//-----------------------------------------------------------------------------
// Cursor_GetStatementType()
//   Determine if the cursor is executing a select statement.
//-----------------------------------------------------------------------------
static int Cursor_GetStatementType(
    udt_Cursor *self)                   // cursor to perform binds on
{
    ub2 statementType;
    sword status;

    status = OCIAttrGet(self->handle, OCI_HTYPE_STMT,
            (dvoid*) &statementType, 0, OCI_ATTR_STMT_TYPE,
            self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_GetStatementType()") < 0)
        return -1;
    self->statementType = statementType;
    if (self->fetchVariables) {
        Py_DECREF(self->fetchVariables);
        self->fetchVariables = NULL;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_FixupBoundCursor()
//   Fixup a cursor so that fetching and returning cursor descriptions are
// successful after binding a cursor to another cursor.
//-----------------------------------------------------------------------------
static int Cursor_FixupBoundCursor(
    udt_Cursor *self)                   // cursor that may have been bound
{
    if (self->handle && self->statementType < 0) {
        if (Cursor_GetStatementType(self) < 0)
            return -1;
        if (self->statementType == OCI_STMT_SELECT &&
                Cursor_PerformDefine(self) < 0)
            return -1;
        if (Cursor_SetRowCount(self) < 0)
            return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_ItemDescriptionHelper()
//   Helper for Cursor_ItemDescription() used so that it is not necessary to
// constantly free the descriptor when an error takes place.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ItemDescriptionHelper(
    udt_Cursor *self,                   // cursor object
    unsigned pos,                       // position in description
    OCIParam *param)                    // parameter to use for description
{
    PyObject *tuple, *var, *type;
    int displaySize, index;
    ub2 internalSize;
    ub4 nameLength;
    sb2 precision;
    sword status;
    char *name;
    ub1 nullOk;
    sb1 scale;

    // acquire internal size of item
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &internalSize, 0,
            OCI_ATTR_DATA_SIZE, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_ItemDescription(): internal size") < 0)
        return NULL;

    // aquire name of item
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &name,
            &nameLength, OCI_ATTR_NAME, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_ItemDescription(): name") < 0)
        return NULL;

    // lookup precision and scale
    scale = 0;
    precision = 0;
    var = PyList_GET_ITEM(self->fetchVariables, pos - 1);
    if (var->ob_type == &g_NumberVarType) {
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &scale, 0,
                OCI_ATTR_SCALE, self->environment->errorHandle);
        if (Environment_CheckForError(self->environment, status,
                "Cursor_ItemDescription(): scale") < 0)
            return NULL;
        status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &precision, 0,
                OCI_ATTR_PRECISION, self->environment->errorHandle);
        if (Environment_CheckForError(self->environment, status,
                "Cursor_ItemDescription(): precision") < 0)
            return NULL;
    }

    // lookup whether null is permitted for the attribute
    status = OCIAttrGet(param, OCI_HTYPE_DESCRIBE, (dvoid*) &nullOk, 0,
            OCI_ATTR_IS_NULL, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_ItemDescription(): nullable") < 0)
        return NULL;

    // set display size based on data type
    type = (PyObject*) var->ob_type;
    if (type == (PyObject*) &g_StringVarType)
        displaySize = internalSize;
    else if (type == (PyObject*) &g_BinaryVarType)
        displaySize = internalSize;
    else if (type == (PyObject*) &g_FixedCharVarType)
        displaySize = internalSize;
    else if (type == (PyObject*) &g_NumberVarType) {
        if (precision) {
            displaySize = precision + 1;
            if (scale > 0)
                displaySize += scale + 1;
        }
        else displaySize = 127;
    } else if (type == (PyObject*) &g_DateTimeVarType) {
#ifdef NATIVE_DATETIME
        type = (PyObject*) PyDateTimeAPI->DateTimeType;
#else
        type = (PyObject*) &g_ExternalDateTimeVarType;
#endif
        displaySize = 23;
    } else {
        displaySize = -1;
    }

    // create the tuple and populate it
    tuple = PyTuple_New(7);
    if (!tuple)
        return NULL;

    // set each of the items in the tuple
    PyTuple_SET_ITEM(tuple, 0, PyString_FromStringAndSize(name, nameLength));
    Py_INCREF(type);
    PyTuple_SET_ITEM(tuple, 1, type);
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(displaySize));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(internalSize));
    PyTuple_SET_ITEM(tuple, 4, PyInt_FromLong(precision));
    PyTuple_SET_ITEM(tuple, 5, PyInt_FromLong(scale));
    PyTuple_SET_ITEM(tuple, 6, PyInt_FromLong(nullOk != 0));

    // make sure the tuple is ok
    for (index = 0; index < 7; index++) {
        if (!PyTuple_GET_ITEM(tuple, index)) {
            Py_DECREF(tuple);
            return NULL;
        }
    }

    return tuple;
}


//-----------------------------------------------------------------------------
// Cursor_ItemDescription()
//   Return a tuple describing the item at the given position.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ItemDescription(
    udt_Cursor *self,                   // cursor object
    unsigned pos)                       // position
{
    PyObject *tuple;
    OCIParam *param;
    sword status;

    // acquire parameter descriptor
    status = OCIParamGet(self->handle, OCI_HTYPE_STMT,
            self->environment->errorHandle, (void**) &param, pos);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_ItemDescription(): parameter") < 0)
        return NULL;

    // use helper routine to get tuple
    tuple = Cursor_ItemDescriptionHelper(self, pos, param);
    OCIDescriptorFree(param, OCI_DTYPE_PARAM);
    return tuple;
}


//-----------------------------------------------------------------------------
// Cursor_GetDescription()
//   Return a list of 7-tuples consisting of the description of the define
// variables.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetDescription(
    udt_Cursor *self,                   // cursor object
    void *arg)                          // optional argument (ignored)
{
    PyObject *results, *tuple;
    int numItems, index;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // fixup bound cursor, if necessary
    if (Cursor_FixupBoundCursor(self) < 0)
        return NULL;

    // if no statement has been executed yet, return None
    if (!self->fetchVariables) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // create a list of the required length
    numItems = PyList_GET_SIZE(self->fetchVariables);
    results = PyList_New(numItems);
    if (!results)
        return NULL;

    // create tuples corresponding to the select-items
    for (index = 0; index < numItems; index++) {
        tuple = Cursor_ItemDescription(self, index + 1);
        if (!tuple) {
            Py_DECREF(results);
            return NULL;
        }
        PyList_SET_ITEM(results, index, tuple);
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_Close()
//   Close the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Close(
    udt_Cursor *self,                   // cursor to close
    PyObject *args)                     // arguments
{
    // make sure we are actually open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // close the cursor
    if (Cursor_FreeHandle(self, 1) < 0)
        return NULL;

    self->handle = NULL;
    self->isOpen = 0;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_SetBindVariableHelper()
//   Helper for setting a bind variable.
//-----------------------------------------------------------------------------
static int Cursor_SetBindVariableHelper(
    udt_Cursor *self,                   // cursor to perform bind on
    unsigned numElements,               // number of elements to create
    unsigned arrayPos,                  // array position to set
    PyObject *value,                    // value to bind
    udt_Variable *origVar,              // original variable bound
    udt_Variable **newVar)              // new variable to be bound
{
    int isValueVar;

    // initialization
    *newVar = NULL;
    isValueVar = Variable_Check(value);

    // handle case where variable is already bound
    if (origVar) {

        // if the value is a variable object, rebind it if necessary
        if (isValueVar) {
            if ( (PyObject*) origVar != value) {
                Py_INCREF(value);
                *newVar = (udt_Variable*) value;
            }

        // if the number of elements has changed, create a new variable
        // this is only necessary for executemany() since execute() always
        // passes a value of 1 for the number of elements
        } else if (numElements > origVar->allocatedElements) {
            *newVar = Variable_New(self, numElements, origVar->type,
                    origVar->maxLength);
            if (!*newVar)
                return -1;
            if (Variable_SetValue(*newVar, arrayPos, value) < 0)
                return -1;

        // otherwise, attempt to set the value
        } else if (Variable_SetValue(origVar, arrayPos, value) < 0) {

            // executemany() should simply fail after the first element
            if (arrayPos > 0)
                return -1;

            // anything other than index error or type error should fail
            if (!PyErr_ExceptionMatches(PyExc_IndexError) &&
                    !PyErr_ExceptionMatches(PyExc_TypeError))
                return -1;

            // clear the exception and try to create a new variable
            PyErr_Clear();
            origVar = NULL;
        }

    }

    // if no original variable used, create a new one
    if (!origVar) {

        // if the value is a variable object, bind it directly
        if (isValueVar) {
            Py_INCREF(value);
            *newVar = (udt_Variable*) value;
            (*newVar)->boundPos = 0;
            Py_XDECREF((*newVar)->boundName);
            (*newVar)->boundName = NULL;

        // otherwise, create a new variable
        } else {
            *newVar = Variable_NewByValue(self, value, numElements);
            if (!*newVar)
                return -1;
            if (Variable_SetValue(*newVar, arrayPos, value) < 0)
                return -1;
        }

    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_SetBindVariables()
//   Create or set bind variables.
//-----------------------------------------------------------------------------
static int Cursor_SetBindVariables(
    udt_Cursor *self,                   // cursor to perform binds on
    PyObject *parameters,               // parameters to bind
    unsigned numElements,               // number of elements to create
    unsigned arrayPos)                  // array position to set
{
    int i, origBoundByPos, origNumParams, boundByPos, numParams;
    PyObject *key, *value, *origVar;
    udt_Variable *newVar;
    Py_ssize_t pos;

    // make sure positional and named binds are not being intermixed
    numParams = 0;
    boundByPos = PySequence_Check(parameters);
    if (boundByPos) {
        numParams = PySequence_Size(parameters);
        if (numParams < 0)
            return -1;
    }
    if (self->bindVariables) {
        origBoundByPos = PyList_Check(self->bindVariables);
        if (boundByPos != origBoundByPos) {
            PyErr_SetString(g_ProgrammingErrorException,
                    "positional and named binds cannot be intermixed");
            return -1;
        }
        origNumParams = PyList_GET_SIZE(self->bindVariables);

    // otherwise, create the list or dictionary if needed
    } else {
        if (boundByPos)
            self->bindVariables = PyList_New(numParams);
        else self->bindVariables = PyDict_New();
        if (!self->bindVariables)
            return -1;
        origNumParams = 0;
    }

    // handle positional binds
    if (boundByPos) {
        for (i = 0; i < numParams; i++) {
            value = PySequence_GetItem(parameters, i);
            if (!value)
                return -1;
            Py_DECREF(value);
            if (i < origNumParams) {
                origVar = PyList_GET_ITEM(self->bindVariables, i);
                if (origVar == Py_None)
                    origVar = NULL;
            } else origVar = NULL;
            if (Cursor_SetBindVariableHelper(self, numElements, arrayPos,
                    value, (udt_Variable*) origVar, &newVar) < 0)
                return -1;
            if (newVar) {
                if (i < PyList_GET_SIZE(self->bindVariables)) {
                    if (PyList_SetItem(self->bindVariables, i,
                            (PyObject*) newVar) < 0) {
                        Py_DECREF(newVar);
                        return -1;
                    }
                } else {
                    if (PyList_Append(self->bindVariables,
                            (PyObject*) newVar) < 0) {
                        Py_DECREF(newVar);
                        return -1;
                    }
                    Py_DECREF(newVar);
                }
            }
        }

    // handle named binds
    } else {
        pos = 0;
        while (PyDict_Next(parameters, &pos, &key, &value)) {
            origVar = PyDict_GetItem(self->bindVariables, key);
            if (Cursor_SetBindVariableHelper(self, numElements, arrayPos,
                    value, (udt_Variable*) origVar, &newVar) < 0)
                return -1;
            if (newVar) {
                if (PyDict_SetItem(self->bindVariables, key,
                        (PyObject*) newVar) < 0) {
                    Py_DECREF(newVar);
                    return -1;
                }
                Py_DECREF(newVar);
            }
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_PerformBind()
//   Perform the binds on the cursor.
//-----------------------------------------------------------------------------
static int Cursor_PerformBind(
    udt_Cursor *self)                   // cursor to perform binds on
{
    PyObject *key, *var;
    Py_ssize_t pos;
    ub2 i;

    // set values and perform binds for all bind variables
    if (self->bindVariables) {
        if (PyDict_Check(self->bindVariables)) {
            pos = 0;
            while (PyDict_Next(self->bindVariables, &pos, &key, &var)) {
                if (Variable_Bind((udt_Variable*) var, self, key, 0) < 0)
                    return -1;
            }
        } else {
            for (i = 0; i < (ub2) PyList_GET_SIZE(self->bindVariables); i++) {
                var = PyList_GET_ITEM(self->bindVariables, i);
                if (var != Py_None) {
                    if (Variable_Bind((udt_Variable*) var, self, NULL,
                            i + 1) < 0)
                        return -1;
                }
            }
        }
    }

    // ensure that input sizes are reset
    self->setInputSizes = 0;

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_CreateRow()
//   Create an object for the row. The object created is a tuple unless a row
// factory function has been defined in which case it is the result of the
// row factory function called with the argument tuple that would otherwise be
// returned.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CreateRow(
    udt_Cursor *self)                   // cursor object
{
    PyObject *tuple, *item, *result;
    int numItems, pos;
    udt_Variable *var;

    // create a new tuple
    numItems = PyList_GET_SIZE(self->fetchVariables);
    tuple = PyTuple_New(numItems);
    if (!tuple)
        return NULL;

    // acquire the value for each item
    for (pos = 0; pos < numItems; pos++) {
        var = (udt_Variable*) PyList_GET_ITEM(self->fetchVariables, pos);
        item = Variable_GetValue(var, self->rowNum);
        if (!item) {
            Py_DECREF(tuple);
            return NULL;
        }
        PyTuple_SET_ITEM(tuple, pos, item);
    }

    // increment row counters
    self->rowNum++;
    self->rowCount++;

    // if a row factory is defined, call it
    if (self->rowFactory && self->rowFactory != Py_None) {
        result = PyObject_CallObject(self->rowFactory, tuple);
        Py_DECREF(tuple);
        return result;
    }

    return tuple;
}


//-----------------------------------------------------------------------------
// Cursor_InternalPrepare()
//   Internal method for preparing a statement for execution.
//-----------------------------------------------------------------------------
static int Cursor_InternalPrepare(
    udt_Cursor *self,                   // cursor to perform prepare on
    PyObject *statement)                // statement to prepare
{
    sword status;

    // make sure we don't get a situation where nothing is to be executed
    if (statement == Py_None && !self->statement) {
        PyErr_SetString(g_ProgrammingErrorException,
                "no statement specified and no prior statement prepared");
        return -1;
    }

    // nothing to do if the statement is identical to the one already stored
    if (statement == Py_None || statement == self->statement) {
        if (self->statementType != OCI_STMT_CREATE &&
                self->statementType != OCI_STMT_DROP &&
                self->statementType != OCI_STMT_ALTER)
            return 0;
        statement = self->statement;
    }

    // keep track of the statement
    Py_XDECREF(self->statement);
    Py_INCREF(statement);
    self->statement = statement;

    // release existing statement, if necessary
#ifdef ORACLE_9I
    if (Cursor_FreeHandle(self, 1) < 0)
        return -1;
#endif

    // prepare statement
    Py_BEGIN_ALLOW_THREADS
#ifdef ORACLE_9I
    self->isOwned = 0;
    status = OCIStmtPrepare2(self->connection->handle, &self->handle,
            self->environment->errorHandle,
            (text*) PyString_AS_STRING(statement),
            PyString_GET_SIZE(statement), NULL, 0, OCI_NTV_SYNTAX,
            OCI_DEFAULT);
#else
    status = OCIStmtPrepare(self->handle, self->environment->errorHandle,
            (text*) PyString_AS_STRING(statement),
            PyString_GET_SIZE(statement), OCI_NTV_SYNTAX, OCI_DEFAULT);
#endif
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(self->environment, status,
            "Cursor_InternalPrepare(): prepare") < 0) {
#ifdef ORACLE_9I
        // this is needed to avoid "invalid handle" errors since Oracle doesn't
        // seem to leave the pointer alone when an error is raised but the
        // resulting handle is still invalid
        self->handle = NULL;
#endif
        return -1;
    }

    // clear bind variables, if applicable
    if (!self->setInputSizes) {
        Py_XDECREF(self->bindVariables);
        self->bindVariables = NULL;
    }

    // clear row factory, if spplicable
    Py_XDECREF(self->rowFactory);
    self->rowFactory = NULL;

    // determine if statement is a query
    if (Cursor_GetStatementType(self) < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_Parse()
//   Parse the statement without executing it.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Parse(
    udt_Cursor *self,                   // cursor to perform prepare on
    PyObject *args)                     // arguments
{
    PyObject *statement;
    sword status;

    // statement text is expected
    if (!PyArg_ParseTuple(args, "S", &statement))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement
    if (Cursor_InternalPrepare(self, statement) < 0)
        return NULL;

    // parse the statement
    Py_BEGIN_ALLOW_THREADS
    status = OCIStmtExecute(self->connection->handle, self->handle,
            self->environment->errorHandle, 0, 0, 0, 0, OCI_PARSE_ONLY);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(self->environment, status,
            "Cursor_Parse()") < 0)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_Prepare()
//   Prepare the statement for execution.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Prepare(
    udt_Cursor *self,                   // cursor to perform prepare on
    PyObject *args)                     // arguments
{
    PyObject *statement;

    // statement text is expected
    if (!PyArg_ParseTuple(args, "S", &statement))
        return NULL;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement
    if (Cursor_InternalPrepare(self, statement) < 0)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_Call()
//   Call a stored procedure or function.
//-----------------------------------------------------------------------------
static int Cursor_Call(
    udt_Cursor *self,                   // cursor to call procedure/function
    udt_Variable *returnValue,          // return value variable (optional)
    const char *name,                   // name of procedure/function to call
    unsigned nameLength,                // length of name of procedure/function
    PyObject *listOfArguments)          // arguments to procedure/function
{
    PyObject *bindVariables, *results, *arguments;
    int numArguments, statementSize, i, offset;
    char *statement, *ptr;

    // determine the number of arguments passed
    if (listOfArguments) {
        if (!PySequence_Check(listOfArguments)) {
            PyErr_SetString(PyExc_TypeError, "arguments must be a sequence");
            return -1;
        }
        numArguments = PySequence_Size(listOfArguments);
        if (numArguments < 0)
            return -1;
    } else {
        numArguments = 0;
        listOfArguments = PyList_New(0);
        if (!listOfArguments)
            return -1;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return -1;

    // determine the statement size add the return value, if applicable
    offset = 0;
    statementSize = nameLength + numArguments * 9 + 15;
    if (returnValue) {
        offset = 1;
        statementSize += 10;
        bindVariables = PySequence_List(listOfArguments);
        if (!bindVariables)
            return -1;
        if (PyList_Insert(bindVariables, 0, (PyObject*) returnValue) < 0) {
            Py_DECREF(bindVariables);
            return -1;
        }
    } else {
        Py_INCREF(listOfArguments);
        bindVariables = listOfArguments;
    }

    // allocate a string for the statement
    statement = (char*) PyMem_Malloc(statementSize);
    if (!statement) {
        Py_DECREF(bindVariables);
        PyErr_NoMemory();
        return -1;
    }

    // build up the statement
    arguments = PySequence_Fast(listOfArguments,
            "expecting sequence of arguments");
    if (!arguments) {
        Py_DECREF(bindVariables);
        PyMem_Free(statement);
        return -1;
    }
    strcpy(statement, "begin ");
    if (returnValue)
        strcat(statement, ":1 := ");
    strcat(statement, name);
    ptr = statement + strlen(statement);
    *ptr++ = '(';
    for (i = 0; i < numArguments; i++) {
        if (i > 0)
            *ptr++ = ',';
        ptr += sprintf(ptr, ":%d", i + offset + 1);
#if (PY_VERSION_HEX >= 0x02030000)
        if (PyBool_Check(PySequence_Fast_GET_ITEM(arguments, i)))
            ptr += sprintf(ptr, " = 1");
#endif
    }
    strcpy(ptr, "); end;");
    Py_DECREF(arguments);

    // execute the statement on the cursor
    results = PyObject_CallMethod( (PyObject*) self, "execute", "sO",
            statement, bindVariables);
    PyMem_Free(statement);
    Py_DECREF(bindVariables);
    if (!results)
        return -1;
    Py_DECREF(results);

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_CallFunc()
//   Call a stored function and return the return value of the function.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CallFunc(
    udt_Cursor *self,                   // cursor to execute
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "name", "returnType", "parameters", NULL };
    PyObject *listOfArguments, *returnType, *results;
    udt_Variable *var;
    int nameLength;
    char *name;

    // expect stored function name, return type and optionally a list of
    // arguments
    listOfArguments = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "s#O|O", keywordList,
            &name, &nameLength, &returnType, &listOfArguments))
        return NULL;

    // create the return variable
    var = Variable_NewByType(self, returnType, 1);
    if (!var)
        return NULL;

    // call the function
    if (Cursor_Call(self, var, name, nameLength, listOfArguments) < 0)
        return NULL;

    // determine the results
    results = Variable_GetValue(var, 0);
    Py_DECREF(var);
    return results;
}


//-----------------------------------------------------------------------------
// Cursor_CallProc()
//   Call a stored procedure and return the (possibly modified) arguments.
//-----------------------------------------------------------------------------
static PyObject *Cursor_CallProc(
    udt_Cursor *self,                   // cursor to execute
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "name", "parameters", NULL };
    PyObject *listOfArguments, *results, *var, *temp;
    int nameLength, numArgs, i;
    char *name;

    // expect stored procedure name and optionally a list of arguments
    listOfArguments = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "s#|O", keywordList,
            &name, &nameLength, &listOfArguments))
        return NULL;

    // call the stored procedure
    if (Cursor_Call(self, NULL, name, nameLength, listOfArguments) < 0)
        return NULL;

    // create the return value
    numArgs = PyList_GET_SIZE(self->bindVariables);
    results = PyList_New(numArgs);
    if (!results)
        return NULL;
    for (i = 0; i < numArgs; i++) {
        var = PyList_GET_ITEM(self->bindVariables, i);
        temp = Variable_GetValue((udt_Variable*) var, 0);
        if (!temp) {
            Py_DECREF(results);
            return NULL;
        }
        PyList_SET_ITEM(results, i, temp);
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_Execute()
//   Execute the statement.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Execute(
    udt_Cursor *self,                   // cursor to execute
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keywords
{
    PyObject *statement, *executeArgs;
    int isQuery;

    executeArgs = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &statement, &executeArgs))
        return NULL;
    if (statement != Py_None && !PyString_Check(statement)) {
        PyErr_SetString(PyExc_TypeError, "expecting None or a string");
        return NULL;
    }
    if (executeArgs && keywordArgs) {
        if (PyDict_Size(keywordArgs) == 0)
            keywordArgs = NULL;
        else {
            PyErr_SetString(g_InterfaceErrorException,
                    "expecting argument or keyword arguments, not both");
            return NULL;
        }
    }
    if (keywordArgs)
        executeArgs = keywordArgs;
    if (executeArgs) {
        if (!PyDict_Check(executeArgs) && !PySequence_Check(executeArgs)) {
            PyErr_SetString(PyExc_TypeError,
                    "expecting a dictionary, sequence or keyword args");
            return NULL;
        }
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement, if applicable
    if (Cursor_InternalPrepare(self, statement) < 0)
        return NULL;

    // perform binds
    if (executeArgs && Cursor_SetBindVariables(self, executeArgs, 1, 0) < 0)
        return NULL;
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement
    isQuery = (self->statementType == OCI_STMT_SELECT);
    if (Cursor_InternalExecute(self, isQuery ? 0 : 1) < 0)
        return NULL;

    // perform defines, if necessary
    if (isQuery && !self->fetchVariables && Cursor_PerformDefine(self) < 0)
        return NULL;

    // reset the values of setoutputsize()
    self->outputSize = -1;
    self->outputSizeColumn = -1;

    // for queries, return the defined variables for possible use
    if (isQuery) {
        Py_INCREF(self->fetchVariables);
        return self->fetchVariables;
    }

    // for all other statements, simply return None
    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_ExecuteMany()
//   Execute the statement many times. The number of times is equivalent to the
// number of elements in the array of dictionaries.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ExecuteMany(
    udt_Cursor *self,                   // cursor to execute
    PyObject *args)                     // arguments
{
    PyObject *arguments, *listOfArguments, *statement;
    int i, numRows;

    // expect statement text (optional) plus list of mappings
    if (!PyArg_ParseTuple(args, "OO!", &statement, &PyList_Type,
            &listOfArguments))
        return NULL;
    if (statement != Py_None && !PyString_Check(statement)) {
        PyErr_SetString(PyExc_TypeError, "expecting None or a string");
        return NULL;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // prepare the statement
    if (Cursor_InternalPrepare(self, statement) < 0)
        return NULL;

    // queries are not supported as the result is undefined
    if (self->statementType == OCI_STMT_SELECT) {
        PyErr_SetString(g_NotSupportedErrorException,
                "queries not supported: results undefined");
        return NULL;
    }

    // perform binds
    numRows = PyList_GET_SIZE(listOfArguments);
    for (i = 0; i < numRows; i++) {
        arguments = PyList_GET_ITEM(listOfArguments, i);
        if (!PyDict_Check(arguments) && !PySequence_Check(arguments)) {
            PyErr_SetString(g_InterfaceErrorException,
                    "expecting a list of dictionaries or sequences");
            return NULL;
        }
        if (Cursor_SetBindVariables(self, arguments, numRows, i) < 0)
            return NULL;
    }
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement
    if (Cursor_InternalExecute(self, PyList_GET_SIZE(listOfArguments)) < 0)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_ExecuteManyPrepared()
//   Execute the prepared statement the number of times requested. At this
// point, the statement must have been already prepared and the bind variables
// must have their values set.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ExecuteManyPrepared(
    udt_Cursor *self,                   // cursor to execute
    PyObject *args)                     // arguments
{
    int numIters;

    // expect number of times to execute the statement
    if (!PyArg_ParseTuple(args, "i", &numIters))
        return NULL;
    if (numIters > self->bindArraySize) {
        PyErr_SetString(g_InterfaceErrorException,
                "iterations exceed bind array size");
        return NULL;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // queries are not supported as the result is undefined
    if (self->statementType == OCI_STMT_SELECT) {
        PyErr_SetString(g_NotSupportedErrorException,
                "queries not supported: results undefined");
        return NULL;
    }

    // perform binds
    if (Cursor_PerformBind(self) < 0)
        return NULL;

    // execute the statement
    if (Cursor_InternalExecute(self, numIters) < 0)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_VerifyFetch()
//   Verify that fetching may happen from this cursor.
//-----------------------------------------------------------------------------
static int Cursor_VerifyFetch(
    udt_Cursor *self)                   // cursor to fetch from
{
    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return -1;

    // fixup bound cursor, if necessary
    if (Cursor_FixupBoundCursor(self) < 0)
        return -1;

    // make sure the cursor is for a query
    if (self->statementType != OCI_STMT_SELECT) {
        PyErr_SetString(g_InterfaceErrorException, "not a query");
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_InternalFetch()
//   Performs the actual fetch from Oracle.
//-----------------------------------------------------------------------------
static int Cursor_InternalFetch(
    udt_Cursor *self,                   // cursor to fetch from
    int numRows)                        // number of rows to fetch
{
    udt_Variable *var;
    sword status;
    ub4 rowCount;
    int i;

    if (!self->fetchVariables) {
        PyErr_SetString(g_InterfaceErrorException, "query not executed");
        return -1;
    }
    Py_BEGIN_ALLOW_THREADS
    status = OCIStmtFetch(self->handle, self->environment->errorHandle,
            numRows, OCI_FETCH_NEXT, OCI_DEFAULT);
    Py_END_ALLOW_THREADS
    if (status != OCI_NO_DATA) {
        if (Environment_CheckForError(self->environment, status,
                "Cursor_InternalFetch(): fetch") < 0)
            return -1;
    }
    for (i = 0; i < PyList_GET_SIZE(self->fetchVariables); i++) {
        var = (udt_Variable*) PyList_GET_ITEM(self->fetchVariables, i);
        var->internalFetchNum++;
    }
    status = OCIAttrGet(self->handle, OCI_HTYPE_STMT, &rowCount, 0,
            OCI_ATTR_ROW_COUNT, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "Cursor_InternalFetch(): row count") < 0)
        return -1;
    self->actualRows = rowCount - self->rowCount;
    self->rowNum = 0;
    return 0;
}


//-----------------------------------------------------------------------------
// Cursor_MoreRows()
//   Returns an integer indicating if more rows can be retrieved from the
// cursor.
//-----------------------------------------------------------------------------
static int Cursor_MoreRows(
    udt_Cursor *self)                   // cursor to fetch from
{
    if (self->rowNum >= self->actualRows) {
        if (self->actualRows < 0 || self->actualRows == self->fetchArraySize) {
            if (Cursor_InternalFetch(self, self->fetchArraySize) < 0)
                return -1;
        }
        if (self->rowNum >= self->actualRows)
            return 0;
    }
    return 1;
}


//-----------------------------------------------------------------------------
// Cursor_MultiFetch()
//   Return a list consisting of the remaining rows up to the given row limit
// (if specified).
//-----------------------------------------------------------------------------
static PyObject *Cursor_MultiFetch(
    udt_Cursor *self,                   // cursor to fetch from
    int rowLimit)                       // row limit
{
    PyObject *results, *row;
    int rowNum, status;

    // create an empty list
    results = PyList_New(0);
    if (!results)
        return NULL;

    // fetch as many rows as possible
    for (rowNum = 0; rowLimit == 0 || rowNum < rowLimit; rowNum++) {
        status = Cursor_MoreRows(self);
        if (status < 0) {
            Py_DECREF(results);
            return NULL;
        } else if (status == 0) {
            break;
        } else {
            row = Cursor_CreateRow(self);
            if (!row) {
                Py_DECREF(results);
                return NULL;
            }
            if (PyList_Append(results, row) < 0) {
                Py_DECREF(row);
                Py_DECREF(results);
                return NULL;
            }
            Py_DECREF(row);
        }
    }

    return results;
}


//-----------------------------------------------------------------------------
// Cursor_FetchOne()
//   Fetch a single row from the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchOne(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    int status;

    // verify fetch can be performed
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;

    // setup return value
    status = Cursor_MoreRows(self);
    if (status < 0)
        return NULL;
    else if (status > 0)
        return Cursor_CreateRow(self);

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_FetchMany()
//   Fetch multiple rows from the cursor based on the arraysize.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchMany(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "numRows", NULL };
    int rowLimit;

    // parse arguments -- optional rowlimit expected
    rowLimit = self->arraySize;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &rowLimit))
        return NULL;

    // verify fetch can be performed
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;

    return Cursor_MultiFetch(self, rowLimit);
}


//-----------------------------------------------------------------------------
// Cursor_FetchAll()
//   Fetch all remaining rows from the cursor.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchAll(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    return Cursor_MultiFetch(self, 0);
}


//-----------------------------------------------------------------------------
// Cursor_FetchRaw()
//   Perform raw fetch on the cursor; return the actual number of rows fetched.
//-----------------------------------------------------------------------------
static PyObject *Cursor_FetchRaw(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "numRows", NULL };
    int numRowsToFetch, numRowsFetched;

    // expect an optional number of rows to retrieve
    numRowsToFetch = self->fetchArraySize;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &numRowsToFetch))
        return NULL;
    if (numRowsToFetch > self->fetchArraySize) {
        PyErr_SetString(g_InterfaceErrorException,
                "rows to fetch exceeds array size");
        return NULL;
    }

    // do not attempt to perform fetch if no more rows to fetch
    if (self->actualRows > 0 && self->actualRows < self->fetchArraySize)
        return PyInt_FromLong(0);

    // perform internal fetch
    if (Cursor_InternalFetch(self, numRowsToFetch) < 0)
        return NULL;

    self->rowCount += self->actualRows;
    numRowsFetched = self->actualRows;
    if (self->actualRows == numRowsToFetch)
        self->actualRows = -1;
    return PyInt_FromLong(numRowsFetched);
}


//-----------------------------------------------------------------------------
// Cursor_SetInputSizes()
//   Set the sizes of the bind variables.
//-----------------------------------------------------------------------------
static PyObject *Cursor_SetInputSizes(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    int numPositionalArgs;
    PyObject *key, *value;
    udt_Variable *var;
    Py_ssize_t i;

    // only expect keyword arguments or positional arguments, not both
    numPositionalArgs = PyTuple_Size(args);
    if (keywordArgs && numPositionalArgs > 0) {
        PyErr_SetString(g_InterfaceErrorException,
                "expecting arguments or keyword arguments, not both");
        return NULL;
    }

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // eliminate existing bind variables
    Py_XDECREF(self->bindVariables);
    if (keywordArgs)
        self->bindVariables = PyDict_New();
    else self->bindVariables = PyList_New(numPositionalArgs);
    if (!self->bindVariables)
        return NULL;
    self->setInputSizes = 1;

    // process each input
    if (keywordArgs) {
        i = 0;
        while (PyDict_Next(keywordArgs, &i, &key, &value)) {
            var = Variable_NewByType(self, value, self->bindArraySize);
            if (!var)
                return NULL;
            if (PyDict_SetItem(self->bindVariables, key,
                    (PyObject*) var) < 0) {
                Py_DECREF(var);
                return NULL;
            }
            Py_DECREF(var);
        }
    } else {
        for (i = 0; i < numPositionalArgs; i++) {
            value = PyTuple_GET_ITEM(args, i);
            if (value == Py_None) {
                Py_INCREF(Py_None);
                PyList_SET_ITEM(self->bindVariables, i, Py_None);
            } else {
                var = Variable_NewByType(self, value, self->bindArraySize);
                if (!var)
                    return NULL;
                PyList_SET_ITEM(self->bindVariables, i, (PyObject*) var);
            }
        }
    }

    Py_INCREF(self->bindVariables);
    return self->bindVariables;
}


//-----------------------------------------------------------------------------
// Cursor_SetOutputSize()
//   Set the size of all of the long columns or just one of them.
//-----------------------------------------------------------------------------
static PyObject *Cursor_SetOutputSize(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    self->outputSizeColumn = -1;
    if (!PyArg_ParseTuple(args, "i|i", &self->outputSize,
            &self->outputSizeColumn))
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// Cursor_Var()
//   Create a bind variable and return it.
//-----------------------------------------------------------------------------
static PyObject *Cursor_Var(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    udt_VariableType *varType;
    PyObject *type;
    int length;

    // parse arguments
    length = 0;
    if (!PyArg_ParseTuple(args, "O|i", &type, &length))
        return NULL;

    // determine the type of variable
    varType = Variable_TypeByPythonType(self, type);
    if (!varType)
        return NULL;
    if (varType->isVariableLength && length == 0)
        length = varType->elementLength;

    return (PyObject*) Variable_New(self, self->bindArraySize, varType,
            length);
}


//-----------------------------------------------------------------------------
// Cursor_ArrayVar()
//   Create an array bind variable and return it.
//-----------------------------------------------------------------------------
static PyObject *Cursor_ArrayVar(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    udt_VariableType *varType;
    int length, numElements;
    PyObject *type, *value;
    udt_Variable *var;

    // parse arguments
    length = 0;
    if (!PyArg_ParseTuple(args, "OO|i", &type, &value, &length))
        return NULL;

    // determine the type of variable
    varType = Variable_TypeByPythonType(self, type);
    if (!varType)
        return NULL;
    if (varType->isVariableLength && length == 0)
        length = varType->elementLength;

    // determine the number of elements to create
    if (PyList_Check(value))
        numElements = PyList_GET_SIZE(value);
    else if (PyInt_Check(value))
        numElements = PyInt_AS_LONG(value);
    else {
        PyErr_SetString(PyExc_TypeError,
                "expecting integer or list of values");
        return NULL;
    }

    // create the variable
    var = Variable_New(self, numElements, varType, length);
    if (!var)
        return NULL;
    if (Variable_MakeArray(var) < 0) {
        Py_DECREF(var);
        return NULL;
    }

    // set the value, if applicable
    if (PyList_Check(value)) {
        if (Variable_SetArrayValue(var, value) < 0)
            return NULL;
    }

    return (PyObject*) var;
}


//-----------------------------------------------------------------------------
// Cursor_BindNames()
//   Return a list of bind variable names.
//-----------------------------------------------------------------------------
static PyObject *Cursor_BindNames(
    udt_Cursor *self,                   // cursor to fetch from
    PyObject *args)                     // arguments
{
    PyObject *names;
    int result;

    // make sure the cursor is open
    if (Cursor_IsOpen(self) < 0)
        return NULL;

    // return result
    result = Cursor_GetBindNames(self, 8, &names);
    if (result < 0)
        return NULL;
    if (!names && Cursor_GetBindNames(self, result, &names) < 0)
        return NULL;
    return names;
}


//-----------------------------------------------------------------------------
// Cursor_GetIter()
//   Return a reference to the cursor which supports the iterator protocol.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetIter(
    udt_Cursor *self)                   // cursor
{
    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    Py_INCREF(self);
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Cursor_GetNext()
//   Return a reference to the cursor which supports the iterator protocol.
//-----------------------------------------------------------------------------
static PyObject *Cursor_GetNext(
    udt_Cursor *self)                   // cursor
{
    int status;

    if (Cursor_VerifyFetch(self) < 0)
        return NULL;
    status = Cursor_MoreRows(self);
    if (status < 0)
        return NULL;
    else if (status > 0)
        return Cursor_CreateRow(self);

    // no more rows, return NULL without setting an exception
    return NULL;
}

