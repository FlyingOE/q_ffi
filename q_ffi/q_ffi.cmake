# vim: set et noai ts=4 sw=4 syntax=cmake:
#############################################################################
# Parameters:
#   q_script   - name of the q script to be processed
#   binary_dir - output directory for the generated scripts
#   dll_suffix - suffix of DLL filename
#############################################################################
configure_file(${q_script} ${binary_dir}/${q_script} @ONLY)
