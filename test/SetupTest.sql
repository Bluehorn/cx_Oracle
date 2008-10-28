/*-----------------------------------------------------------------------------
 * SetupTest.sql
 *   Creates a user named "cx_Oracle" and populates its schema with the tables
 * and packages necessary for performing the test suite.
 *---------------------------------------------------------------------------*/

whenever sqlerror exit failure

-- drop existing users, if present
begin
  for r in
      ( select username
        from dba_users
        where username in ('CX_ORACLE', 'CX_ORACLE_PROXY')
      ) loop
    execute immediate 'drop user ' || r.username || ' cascade';
  end loop;
end;
/

alter session set nls_date_format = 'YYYY-MM-DD HH24:MI:SS';
alter session set nls_numeric_characters='.,';

create user cx_Oracle identified by dev
quota unlimited on users
default tablespace users;

grant
  create session,
  create table,
  create procedure,
  create type
to cx_Oracle;

-- create types
create type cx_Oracle.udt_Object as object (
  NumberValue           number,
  StringValue           varchar2(60),
  FixedCharValue        char(10),
  DateValue             date
);
/

create type cx_Oracle.udt_Array as varray(10) of number;
/

-- create tables
create table cx_Oracle.TestNumbers (
  IntCol                number(9) not null,
  NumberCol             number(9, 2) not null,
  FloatCol              float not null,
  UnconstrainedCol      number not null,
  NullableCol           number(38)
) tablespace users;

create table cx_Oracle.TestStrings (
  IntCol                number(9) not null,
  StringCol             varchar2(20) not null,
  RawCol                raw(30) not null,
  FixedCharCol          char(40) not null,
  NullableCol           varchar2(50)
) tablespace users;

create table cx_Oracle.TestDates (
  IntCol                number(9) not null,
  DateCol               date not null,
  NullableCol           date
) tablespace users;

create table cx_Oracle.TestCLOBs (
  IntCol                number(9) not null,
  CLOBCol               clob not null
) tablespace users;

create table cx_Oracle.TestBLOBs (
  IntCol                number(9) not null,
  BLOBCol               blob not null
) tablespace users;

create table cx_Oracle.TestLongs (
  IntCol                number(9) not null,
  LongCol               long not null
) tablespace users;

create table cx_Oracle.TestLongRaws (
  IntCol                number(9) not null,
  LongRawCol            long raw not null
) tablespace users;

create table cx_Oracle.TestExecuteMany (
  IntCol                number(9) not null
) tablespace users;

create table cx_Oracle.TestObjects (
  IntCol                number(9) not null,
  ObjectCol             cx_Oracle.udt_Object,
  ArrayCol              cx_Oracle.udt_Array
);

alter table cx_Oracle.testexecutemany
add constraint testexecutemany_pk
primary key (
  intcol
) using index tablespace users;

-- populate tables
begin
  for i in 1..10 loop
    insert into cx_Oracle.TestNumbers
    values (i, i + i * 0.25, i + i * .75, i * i * i + i *.5,
        decode(mod(i, 2), 0, null, power(143, i)));
  end loop;
end;
/

declare

  t_RawValue			raw(30);

  function ConvertHexDigit(a_Value number) return varchar2 is
  begin
    if a_Value between 0 and 9 then
      return to_char(a_Value);
    end if;
    return chr(ascii('A') + a_Value - 10);
  end;

  function ConvertToHex(a_Value varchar2) return varchar2 is
    t_HexValue			varchar2(60);
    t_Digit			number;
  begin
    for i in 1..length(a_Value) loop
      t_Digit := ascii(substr(a_Value, i, 1));
      t_HexValue := t_HexValue ||
          ConvertHexDigit(trunc(t_Digit / 16)) ||
          ConvertHexDigit(mod(t_Digit, 16));
    end loop;
    return t_HexValue;
  end;

begin
  for i in 1..10 loop
    t_RawValue := hextoraw(ConvertToHex('Raw ' || to_char(i)));
    insert into cx_Oracle.TestStrings
    values (i, 'String ' || to_char(i), t_RawValue,
        'Fixed Char ' || to_char(i),
        decode(mod(i, 2), 0, null, 'Nullable ' || to_char(i)));
  end loop;
end;
/

begin
  for i in 1..10 loop
    insert into cx_Oracle.TestDates
    values (i, to_date(20021209, 'YYYYMMDD') + i + i * .1,
        decode(mod(i, 2), 0, null,
        to_date(20021209, 'YYYYMMDD') + i + i + i * .15));
  end loop;
end;
/

insert into cx_Oracle.TestObjects values (1,
    cx_Oracle.udt_Object(1, 'First row', 'First',
        to_date(20070306, 'YYYYMMDD')),
    cx_Oracle.udt_Array(5, 10, null, 20));

insert into cx_Oracle.TestObjects values (2, null,
    cx_Oracle.udt_Array(3, null, 9, 12, 15));

insert into cx_Oracle.TestObjects values (3,
    cx_Oracle.udt_Object(3, 'Third row', 'Third',
        to_date(20070621, 'YYYYMMDD')), null);

commit;

-- create procedures for testing callproc()
create procedure cx_Oracle.proc_Test (
  a_InValue             varchar2,
  a_InOutValue          in out number,
  a_OutValue            out number
) as
begin
  a_InOutValue := a_InOutValue * length(a_InValue);
  a_OutValue := length(a_InValue);
end;
/

create procedure cx_Oracle.proc_TestNoArgs as
begin
  null;
end;
/

-- create functions for testing callfunc()
create function cx_Oracle.func_Test (
  a_String              varchar2,
  a_ExtraAmount         number
) return number as
begin
  return length(a_String) + a_ExtraAmount;
end;
/

create function cx_Oracle.func_TestNoArgs
return number as
begin
  return 712;
end;
/

-- create packages
create or replace package cx_Oracle.pkg_TestStringArrays as

  type udt_StringList is table of varchar2(100) index by binary_integer;

  function TestInArrays (
    a_StartingLength    number,
    a_Array             udt_StringList
  ) return number;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out nocopy udt_StringList
  );

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out nocopy udt_StringList
  );

end;
/

create or replace package body cx_Oracle.pkg_TestStringArrays as

  function TestInArrays (
    a_StartingLength    number,
    a_Array             udt_StringList
  ) return number is
    t_Length            number;
  begin
    t_Length := a_StartingLength;
    for i in 1..a_Array.count loop
      t_Length := t_Length + length(a_Array(i));
    end loop;
    return t_Length;
  end;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out udt_StringList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := 'Converted element # ' ||
          to_char(i) || ' originally had length ' ||
          to_char(length(a_Array(i)));
    end loop;
  end;

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out udt_StringList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := 'Test out element # ' || to_char(i);
    end loop;
  end;

end;
/

create or replace package cx_Oracle.pkg_TestNumberArrays as

  type udt_NumberList is table of number index by binary_integer;

  function TestInArrays (
    a_StartingValue     number,
    a_Array             udt_NumberList
  ) return number;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out nocopy udt_NumberList
  );

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out nocopy udt_NumberList
  );

end;
/

create or replace package body cx_Oracle.pkg_TestNumberArrays as

  function TestInArrays (
    a_StartingValue     number,
    a_Array             udt_NumberList
  ) return number is
    t_Value             number;
  begin
    t_Value := a_StartingValue;
    for i in 1..a_Array.count loop
      t_Value := t_Value + a_Array(i);
    end loop;
    return t_Value;
  end;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out udt_NumberList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := a_Array(i) * 10;
    end loop;
  end;

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out udt_NumberList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := i * 100;
    end loop;
  end;

end;
/

create or replace package cx_Oracle.pkg_TestDateArrays as

  type udt_DateList is table of date index by binary_integer;

  function TestInArrays (
    a_StartingValue     number,
    a_BaseDate          date,
    a_Array             udt_DateList
  ) return number;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out nocopy udt_DateList
  );

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out nocopy udt_DateList
  );

end;
/

create or replace package body cx_Oracle.pkg_TestDateArrays as

  function TestInArrays (
    a_StartingValue     number,
    a_BaseDate          date,
    a_Array             udt_DateList
  ) return number is
    t_Value             number;
  begin
    t_Value := a_StartingValue;
    for i in 1..a_Array.count loop
      t_Value := t_Value + a_Array(i) - a_BaseDate;
    end loop;
    return t_Value;
  end;

  procedure TestInOutArrays (
    a_NumElems          number,
    a_Array             in out udt_DateList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := a_Array(i) + 7;
    end loop;
  end;

  procedure TestOutArrays (
    a_NumElems          number,
    a_Array             out udt_DateList
  ) is
  begin
    for i in 1..a_NumElems loop
      a_Array(i) := to_date(20021212, 'YYYYMMDD') + i * 1.2;
    end loop;
  end;

end;
/

create or replace package cx_Oracle.pkg_TestOutCursors as

  type udt_RefCursor is ref cursor;

  procedure TestOutCursor (
    a_MaxIntValue       number,
    a_Cursor            out udt_RefCursor
  );

end;
/

create or replace package body cx_Oracle.pkg_TestOutCursors as

  procedure TestOutCursor (
    a_MaxIntValue       number,
    a_Cursor            out udt_RefCursor
  ) is
  begin
    open a_Cursor for
      select
        IntCol,
        StringCol
      from TestStrings
      where IntCol <= a_MaxIntValue
      order by IntCol;
  end;

end;
/

exit

