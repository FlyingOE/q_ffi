/ vim: set et noai ts=2 sw=2 syntax=q:
\d .ffi

DLL:`:q_ffi;

/// @brief Load a function from a DLL with the given signature.
/// @param dllSym   A file symbol pointing to the target DLL (sans the file extension).
/// @param fName    The target function name.
///		On Windows 32-bit environment, the function is invoked with <code>__cdecl</code> convention by default.
///     If the name is decorated as <code>_<i>xxxx</i>@<i>n</i></code>, the function is invoked with <code>__stdcall__</code> convention.
/// @param resType  Type of the function's result. Similar to that used in <code>0:</code> for CSV parsing.
/// @param parTypes Types of the function's parameters. Similar to that used in <code>0:</code> for CSV parsing.
/// @return         ??????????????
/// @code{.q}
///	.ffi.load[;;;]
/// @endcode
.ffi.load:{[dllSym;fName;resType;parTypes]
  };

/// @brief DLL version/build information.
/// @code{.q}
///	.ffi.version[]
/// @endcode
version:DLL 2:(`version;1);

\d .
\
__EOF__