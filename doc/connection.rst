.. _connobj:

*****************
Connection Object
*****************

.. note::

   Any outstanding changes will be rolled back when the connection object
   is destroyed or closed.



.. method:: Connection.__enter__()

   The entry point for the connection as a context manager, a feature available
   in Python 2.5 and higher. It returns itself.

   .. note::

      This method is an extension to the DB API definition.


.. method:: Connection.__exit__()

   The exit point for the connection as a context manager, a feature available
   in Python 2.5 and higher. In the event of an exception, the transaction is
   rolled back; otherwise, the transaction is committed.

   .. note::

      This method is an extension to the DB API definition.


.. attribute:: Connection.action

   This write-only attribute sets the action column in the v$session table and
   is only available in Oracle 10g.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.autocommit

   This read-write attribute determines whether autocommit mode is on or off.

   .. note::

      This attribute is an extension to the DB API definition.


.. method:: Connection.begin()
              Connection.begin([formatId, transactionId, branchId])

   Explicitly begin a new transaction. Without parameters, this explicitly
   begins a local transaction; otherwise, this explicitly begins a distributed
   (global) transaction with the given parameters. See the Oracle documentation
   for more details.

   Note that in order to make use of global (distributed) transactions, the
   twophase argument to the Connection constructor must be a true value. See
   the comments on the Connection constructor for more information
   (:ref:`module`).

   .. note::

      This method is an extension to the DB API definition.


.. method:: Connection.cancel()

   Cancel a long-running transaction. This is only effective on non-Windows
   platforms.


.. attribute:: Connection.clientinfo

   This write-only attribute sets the client_info column in the v$session table
   and is only available in Oracle 10g.

   .. note::

      This attribute is an extension to the DB API definition.


.. method:: Connection.close()

   Close the connection now, rather than whenever __del__ is called. The
   connection will be unusable from this point forward; an Error exception will
   be raised if any operation is attempted with the connection. The same
   applies to any cursor objects trying to use the connection.


.. method:: Connection.commit()

   Commit any pending transactions to the database.


.. method:: Connection.cursor()

   Return a new Cursor object (:ref:`cursorobj`) using the connection.


.. attribute:: Connection.dsn

   This read-only attribute returns the TNS entry of the database to which a
   connection has been established.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.encoding

   This read-only attribute returns the IANA character set name of the
   character set in use by the Oracle client.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.maxBytesPerCharacter

   This read-only attribute returns the maximum number of bytes each character
   can use for the client character set.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.module

   This write-only attribute sets the module column in the v$session table and
   is only available in Oracle 10g.

   .. note:

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.nencoding

   This read-only attribute returns the IANA character set name of the national
   character set in use by the Oracle client.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.password

   This read-write attribute initially contains the password of the user which
   established the connection to the database.

   .. note::

      This attribute is an extension to the DB API definition.


.. method:: Connection.ping()

   Ping the server which can be used to test if the connection is still active.

   .. note::

         This method is an extension to the DB API definition and is only
         available in Oracle 10g R2 and higher.


.. method:: Connection.prepare()

   Prepare the distributed (global) transaction for commit.

   .. note::

         This method is an extension to the DB API definition.


.. method:: Connection.register(code, when, function)

   Register the function as an OCI callback. The code is one of the function
   codes defined in the Oracle documentation of which the most common ones are
   defined as constants in this module. The when parameter is one of
   :data:`UCBTYPE_ENTRY`, :data:`UCBTYPE_EXIT` or :data:`UCBTYPE_REPLACE`. The
   function is a Python function which will accept the parameters that the OCI
   function accepts, modified as needed to return Python objects that are of
   some use. Note that this is a highly experimental method and can cause
   cx_Oracle to crash if not used properly. In particular, the OCI does not
   provide sizing information to the callback so attempts to access a variable
   beyond the allocated size will crash cx_Oracle.  Use with caution.

   .. note::

      This method is an extension to the DB API definition.


.. method:: Connection.rollback()

   Rollback any pending transactions.


.. method:: Connection.shutdown([mode])

   Shutdown the database. In order to do this the connection must connected as
   :data:`SYSDBA` or :data:`SYSOPER`. First shutdown using one of the
   DBSHUTDOWN constants defined in the constants (:ref:`constants`) section.
   Next issue the SQL statements required to close the database ("alter
   database close normal") and dismount the database ("alter database
   dismount") followed by a second call to this method with the
   :data:`DBSHUTDOWN_FINAL` mode.

   .. note::

      This method is an extension to the DB API definition and is only
      available in Oracle 10g R2 and higher.


.. method:: Connection.startup(force=False, restrict=False)

   Startup the database. This is equivalent to the SQL\*Plus command "startup
   nomount". The connection must be connected as :data:`SYSDBA` or
   :data:`SYSOPER` with the :data:`PRELIM_AUTH` option specified for this to
   work. Once this method has completed, connect again without the
   :data:`PRELIM_AUTH` option and issue the statements required to mount
   ("alter database mount") and open ("alter database open") the database.

   .. note::

      This method is an extension to the DB API definition and is only
      available in Oracle 10g R2 and higher.


.. attribute:: Connection.stmtcachesize

   This read-write attribute specifies the size of the statement cache. This
   value can make a significant difference in performance (up to 100x) if you
   have a small number of statements that you execute repeatedly.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.tnsentry

   This read-only attribute returns the TNS entry of the database to which a
   connection has been established.

   .. note::

      This attribute is an extension to the DB API definition.


.. method:: Connection.unregister(code, when)

   Unregister the function as an OCI callback. The code is one of the function
   codes defined in the Oracle documentation of which the most common ones are
   defined as constants in this module. The when parameter is one of
   :data:`UCBTYPE_ENTRY`, :data:`UCBTYPE_EXIT` or :data:`UCBTYPE_REPLACE`.

   .. note::

      This method is an extension to the DB API definition.


.. attribute:: Connection.username

   This read-only attribute returns the name of the user which established the
   connection to the database.

   .. note::

      This attribute is an extension to the DB API definition.


.. attribute:: Connection.version

   This read-only attribute returns the version of the database to which a
   connection has been established.

   .. note::

      This attribute is an extension to the DB API definition.

