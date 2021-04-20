// vim: set et noai ts=2 sw=2 syntax=q:
\d .ffi
/////////////////////////////////////////////////////////////////////////////
/// Enhanced FFI (foreign function interface) support for k4.
/////////////////////////////////////////////////////////////////////////////
DLL:@[value;`.ffi.DLL;{
  .Q.dd[hsym .z.o;]
    $[.z.o like"w*"; `q_ffi@dll_suffix@; `libq_ffi@dll_suffix@]
 }];

/// @brief DLL version/build information.
version:DLL 2:(`version;1);

/// @brief Return a q type code for a @c size_t in the current platform.
size_t:DLL 2:(`size_type;1);
/// @brief Return the size of a @c size_t in the current platform.
size_n:DLL 2:(`size_size;1);

/// @brief Return a q type code for a pointer in the current platform.
ptr_t:DLL 2:(`ptr_type;1);
/// @brief Return the size of a pointer in the current platform.
ptr_n:DLL 2:(`ptr_size;1);

/// @brief Load a function from a DLL with the given signature and calling convention.
///     Parameters are similar to those in @ref .ffi.load
/// @param abi (Symbol;CharList) Calling convention to use.
///     For Windows 32-bit environment, this can be <code>`cdecl`stdcall`fastcall</code>,
///     with @c `, calling convention is detected according to function name's mangling rule.
load0:DLL 2:(`load_fun;5);

/// @brief Load a function from a DLL with the given signature.
/// @param dllSym (Symbol)
/// @param funName (Symbol|CharList)
///     On Windows 32-bit environment, the function is invoked with @c __cdecl convention by default.
///     If the name is decorated as <code>_<i>xxxx</i>@<i>n</i></code>, it is invoked with @c __stdcall__ convention.
///     If the name is decorated as <code>@<i>xxxx</i>@<i>n</i></code>, it is invoked with @c __fastcall__ convention.
/// @param resType (Char) Type of the function's result. Similar to that used in <code>0:</code> for CSV parsing.
///     @c void return is declared as <code>" "</code>.
///     Any pointer type is declared as <code>"&"</code>.
/// @param argTypes (CharList|Symbol) Types of the function's arguments.
///     Similar to that used in resType.
/// @return (Projection) Target function as a q projection.
.ffi.load:load0[;;`;;];

/// @brief Load a variable from a DLL.
/// @param dllSym (Symbol)
/// @param varName (Symbol|CharList)
/// @param varType (Char) Type of the variable. Similar to that used in <code>0:</code> for CSV parsing.
///     Any pointer type is declared as <code>"&"</code>.
/// @return Target variable's value.
.ffi.get:DLL 2:(`get_var;3);

/// @brief Changes a variable from a DLL.
///   CAUTION: DLL variables are prone to being overwritten due to DLL reloads.
///       Use this function at your own risk!
/// @param dllSym (Symbol)
/// @param varName (Symbol|CharList)
/// @param val The value to set.
.ffi.set:DLL 2:(`set_var;3);

/// @brief Get the pointer to the values in a q list.
/// @return The memory address of the value in @ref ptr_t type.
.ffi.addr:DLL 2:(`addr_of;1);

/// @brief Calls @c free on a given address.
free:DLL 2:(`free_addr;1);

/// @brief Get the value at the address provided by a pointer.
/// @param addr The memory address. The data type is platform-dependent, given by @ref ptr_t
/// @param varType (Char) Type of the value at the address. Similar to that used in <code>0:</code>.
/// @return The memory address of the value in @ref ptr_t type.
ptr_get:DLL 2:(`get_from_addr;2);

/// @brief Set the value at the address provided by a pointer.
/// @param addr The memory address. The data type is platform-dependent, given by @ref ptr_t
/// @param value The value to set.
/// @return The memory address of the value in @ref ptr_t type.
ptr_set:DLL 2:(`set_to_addr;2);

\d .
\
__EOF__
