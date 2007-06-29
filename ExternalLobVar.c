//-----------------------------------------------------------------------------
// ExternalLobVar.c
//   Defines the routines for handling LOB variables external to this module.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// external LOB type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    udt_LobVar *lobVar;
    unsigned pos;
    unsigned internalFetchNum;
} udt_ExternalLobVar;


//-----------------------------------------------------------------------------
// Declaration of external LOB variable functions.
//-----------------------------------------------------------------------------
static void ExternalLobVar_Free(udt_ExternalLobVar*);
static PyObject *ExternalLobVar_Str(udt_ExternalLobVar*);
static PyObject *ExternalLobVar_GetAttr(udt_ExternalLobVar*, PyObject*);
static PyObject *ExternalLobVar_Size(udt_ExternalLobVar*, PyObject*);
static PyObject *ExternalLobVar_Read(udt_ExternalLobVar*, PyObject*,
        PyObject*);
static PyObject *ExternalLobVar_Write(udt_ExternalLobVar*, PyObject*,
        PyObject*);
static PyObject *ExternalLobVar_Trim(udt_ExternalLobVar*, PyObject*,
        PyObject*);
static PyObject *ExternalLobVar_GetFileName(udt_ExternalLobVar*, PyObject*);
static PyObject *ExternalLobVar_SetFileName(udt_ExternalLobVar*, PyObject*);
static PyObject *ExternalLobVar_FileExists(udt_ExternalLobVar*, PyObject*);
static PyObject *ExternalLobVar_Reduce(udt_ExternalLobVar*);


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
static PyTypeObject g_ExternalLobVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  // ob_size
    "cx_Oracle.LOB",                    // tp_name
    sizeof(udt_ExternalLobVar),         // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) ExternalLobVar_Free,   // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    (reprfunc) ExternalLobVar_Str,      // tp_str
    (getattrofunc) ExternalLobVar_GetAttr,
                                        // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0                                   // tp_doc
};


//-----------------------------------------------------------------------------
// declaration of methods for Python type "ExternalLOBVar"
//-----------------------------------------------------------------------------
static PyMethodDef g_ExternalLobVarMethods[] = {
    { "size", (PyCFunction) ExternalLobVar_Size, METH_NOARGS },
    { "read", (PyCFunction) ExternalLobVar_Read,
              METH_VARARGS  | METH_KEYWORDS },
    { "write", (PyCFunction) ExternalLobVar_Write,
              METH_VARARGS  | METH_KEYWORDS },
    { "trim", (PyCFunction) ExternalLobVar_Trim,
              METH_VARARGS  | METH_KEYWORDS },
    { "getfilename", (PyCFunction) ExternalLobVar_GetFileName, METH_NOARGS },
    { "setfilename", (PyCFunction) ExternalLobVar_SetFileName, METH_VARARGS },
    { "fileexists", (PyCFunction) ExternalLobVar_FileExists, METH_NOARGS },
    { "__reduce__", (PyCFunction) ExternalLobVar_Reduce, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// ExternalLobVar_New()
//   Create a new external LOB variable.
//-----------------------------------------------------------------------------
PyObject *ExternalLobVar_New(
    udt_LobVar *var,                    // variable to encapsulate
    unsigned pos)                       // position in array to encapsulate
{
    udt_ExternalLobVar *newVar;

    newVar = PyObject_NEW(udt_ExternalLobVar, &g_ExternalLobVarType);
    if (!newVar)
        return NULL;
    newVar->pos = pos;
    newVar->internalFetchNum = var->internalFetchNum;
    Py_INCREF(var);
    newVar->lobVar = var;

    return (PyObject*) newVar;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Free()
//   Free an external LOB variable.
//-----------------------------------------------------------------------------
static void ExternalLobVar_Free(
    udt_ExternalLobVar *var)            // variable to free
{
    Py_DECREF(var->lobVar);
    PyObject_DEL(var);
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Verify()
//   Verify that the external LOB var is still valid.
//-----------------------------------------------------------------------------
static int ExternalLobVar_Verify(
    udt_ExternalLobVar *var)            // variable to verify
{
    if (var->internalFetchNum != var->lobVar->internalFetchNum) {
        PyErr_SetString(g_ProgrammingErrorException,
                "LOB variable no longer valid after subsequent fetch");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_GetAttr()
//   Retrieve an attribute on the external LOB variable object.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_GetAttr(
    udt_ExternalLobVar *var,            // cursor object
    PyObject *name)                     // name of attribute
{
    return Py_FindMethod(g_ExternalLobVarMethods, (PyObject*) var,
            PyString_AS_STRING(name));
}


//-----------------------------------------------------------------------------
// ExternalLobVar_InternalRead()
//   Return the size of the LOB variable for internal comsumption.
//-----------------------------------------------------------------------------
static int ExternalLobVar_InternalRead(
    udt_ExternalLobVar *var,            // variable to return the size of
    char *buffer,                       // buffer in which to put data
    ub4 bufferSize,                     // size of buffer
    ub4 *length,                        // length of data (IN/OUT)
    int offset)                         // offset
{
    sword status;

    if (var->lobVar->isFile) {
        status = OCILobFileOpen(var->lobVar->connection->handle,
                var->lobVar->environment->errorHandle,
                var->lobVar->data[var->pos], OCI_FILE_READONLY);
        if (Environment_CheckForError(var->lobVar->environment, status,
                "ExternalLobVar_FileOpen()") < 0)
            return -1;
    }

    status = OCILobRead(var->lobVar->connection->handle,
            var->lobVar->environment->errorHandle,
            var->lobVar->data[var->pos], length, offset, buffer,
            bufferSize, NULL, NULL, 0, var->lobVar->type->charsetForm);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_LobRead()") < 0) {
        OCILobFileClose(var->lobVar->connection->handle,
                var->lobVar->environment->errorHandle,
                var->lobVar->data[var->pos]);
        return -1;
    }

    if (var->lobVar->isFile) {
        status = OCILobFileClose(var->lobVar->connection->handle,
                var->lobVar->environment->errorHandle,
                var->lobVar->data[var->pos]);
        if (Environment_CheckForError(var->lobVar->environment, status,
                "ExternalLobVar_FileClose()") < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_InternalSize()
//   Return the size of the LOB variable for internal comsumption.
//-----------------------------------------------------------------------------
static int ExternalLobVar_InternalSize(
    udt_ExternalLobVar *var)            // variable to return the size of
{
    sword status;
    ub4 length;

    status = OCILobGetLength(var->lobVar->connection->handle,
            var->lobVar->environment->errorHandle,
            var->lobVar->data[var->pos], &length);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_InternalSize()") < 0)
        return -1;

    return length;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Value()
//   Return a portion (or all) of the data in the external LOB variable.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Value(
    udt_ExternalLobVar *var,            // variable to return the size of
    int offset,                         // offset into LOB
    int amount)                         // amount to read from LOB
{
    ub4 length, bufferSize;
    PyObject *result;
    char *buffer;

    // modify the arguments
    if (offset < 0)
        offset = 1;
    if (amount < 0) {
        amount = ExternalLobVar_InternalSize(var);
        if (amount < 0)
            return NULL;
        amount = amount - offset + 1;
        if (amount <= 0)
            amount = 1;
    }
    length = amount;
    bufferSize = amount * var->lobVar->environment->maxBytesPerCharacter;

    // create a string for retrieving the value
    buffer = (char*) PyMem_Malloc(bufferSize);
    if (!buffer)
        return PyErr_NoMemory();
    if (ExternalLobVar_InternalRead(var, buffer, bufferSize, &length,
            offset) < 0) {
        PyMem_Free(buffer);
        return NULL;
    }

    // return the result
    if (var->lobVar->environment->fixedWidth)
        length = length * var->lobVar->environment->maxBytesPerCharacter;
    result = PyString_FromStringAndSize(buffer, length);
    PyMem_Free(buffer);
    return result;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Size()
//   Return the size of the data in the LOB variable.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Size(
    udt_ExternalLobVar *var,            // variable to return the size of
    PyObject *args)                     // arguments
{
    int length;

    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    length = ExternalLobVar_InternalSize(var);
    if (length < 0)
        return NULL;
    return PyInt_FromLong(length);
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Read()
//   Return a portion (or all) of the data in the external LOB variable.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Read(
    udt_ExternalLobVar *var,            // variable to return the size of
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "offset", "amount", NULL };
    int offset, amount;

    // offset and amount are expected, both optional
    offset = amount = -1;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|ii", keywordList,
            &offset, &amount))
        return NULL;

    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    return ExternalLobVar_Value(var, offset, amount);
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Str()
//   Return all of the data in the external LOB variable.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Str(
    udt_ExternalLobVar *var)            // variable to return the string for
{
    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    return ExternalLobVar_Value(var, 1, -1);
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Write()
//   Write a value to the LOB variable; return the number of bytes written.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Write(
    udt_ExternalLobVar *var,            // variable to perform write against
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "data", "offset", NULL };
    int offset, bufferLength, length;
    char *buffer;
    sword status;

    // buffer and offset are expected, offset is optional
    offset = -1;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "s#|i", keywordList,
            &buffer, &bufferLength, &offset))
        return NULL;
    if (offset < 0)
        offset = 1;

    // create a string for retrieving the value
    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    length = bufferLength;
    status = OCILobWrite(var->lobVar->connection->handle,
            var->lobVar->environment->errorHandle, var->lobVar->data[var->pos],
            (unsigned int*) &length, offset, buffer, bufferLength,
            OCI_ONE_PIECE, NULL, NULL, 0, var->lobVar->type->charsetForm);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_Write()") < 0)
        return NULL;

    // return the result
    return PyInt_FromLong(length);
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Trim()
//   Trim the LOB variable to the specified length.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Trim(
    udt_ExternalLobVar *var,            // variable to perform write against
    PyObject *args,                     // arguments
    PyObject *keywordArgs)              // keyword arguments
{
    static char *keywordList[] = { "newSize", NULL };
    sword status;
    ub4 newSize;

    // buffer and offset are expected, offset is optional
    newSize = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &newSize))
        return NULL;

    // create a string for retrieving the value
    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    status = OCILobTrim(var->lobVar->connection->handle,
            var->lobVar->environment->errorHandle, var->lobVar->data[var->pos],
            newSize);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_Trim()") < 0)
        return NULL;

    // return the result
    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_Reduce()
//   Method provided for pickling/unpickling of LOB variables.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_Reduce(
    udt_ExternalLobVar *self)           // variable to dump
{
    PyObject *result, *value;

    value = ExternalLobVar_Str(self);
    if (!value)
        return NULL;
    result = Py_BuildValue("(O(O))", &PyString_Type, value);
    Py_DECREF(value);
    return result;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_GetFileName()
//   Return the directory alias and file name for the BFILE lob.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_GetFileName(
    udt_ExternalLobVar *var,            // variable to perform write against
    PyObject *args)                     // arguments
{
    char dirAlias[30], name[255];
    ub2 dirAliasLength, nameLength;
    PyObject *result, *temp;
    sword status;

    // determine the directory alias and name
    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    dirAliasLength = sizeof(dirAlias);
    nameLength = sizeof(name);
    status = OCILobFileGetName(var->lobVar->environment->handle,
            var->lobVar->environment->errorHandle, var->lobVar->data[var->pos],
            (text*) dirAlias, &dirAliasLength, (text*) name, &nameLength);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_GetFileName()") < 0)
        return NULL;

    // create the two-tuple for returning
    result = PyTuple_New(2);
    if (!result)
        return NULL;
    temp = PyString_FromStringAndSize(dirAlias, dirAliasLength);
    if (!temp) {
        Py_DECREF(result);
        return NULL;
    }
    PyTuple_SET_ITEM(result, 0, temp);
    temp = PyString_FromStringAndSize(name, nameLength);
    if (!temp) {
        Py_DECREF(result);
        return NULL;
    }
    PyTuple_SET_ITEM(result, 1, temp);

    return result;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_SetFileName()
//   Set the directory alias and file name for the BFILE lob.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_SetFileName(
    udt_ExternalLobVar *var,            // variable to perform write against
    PyObject *args)                     // arguments
{
    int dirAliasLength, nameLength;
    char *dirAlias, *name;
    sword status;

    // get the directory alias and name as strings
    if (!PyArg_ParseTuple(args, "s#s#", &dirAlias, &dirAliasLength, &name,
            &nameLength))
        return NULL;

    // create a string for retrieving the value
    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    status = OCILobFileSetName(var->lobVar->environment->handle,
            var->lobVar->environment->errorHandle,
            &var->lobVar->data[var->pos], (text*) dirAlias,
            (ub2) dirAliasLength, (text*) name, (ub2) nameLength);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_SetFileName()") < 0)
        return NULL;

    // return the result
    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// ExternalLobVar_FileExists()
//   Return a boolean indicating if the BFIILE lob exists.
//-----------------------------------------------------------------------------
static PyObject *ExternalLobVar_FileExists(
    udt_ExternalLobVar *var,            // variable to perform write against
    PyObject *args)                     // arguments
{
    PyObject *result;
    sword status;
    boolean flag;

    if (ExternalLobVar_Verify(var) < 0)
        return NULL;
    status = OCILobFileExists(var->lobVar->connection->handle,
            var->lobVar->environment->errorHandle, var->lobVar->data[var->pos],
            &flag);
    if (Environment_CheckForError(var->lobVar->environment, status,
            "ExternalLobVar_FileExists()") < 0)
        return NULL;

    // return the result
    if (flag)
        result = Py_True;
    else result = Py_False;
    Py_INCREF(result);
    return result;
}

