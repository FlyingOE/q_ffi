# vim: set et noai ts=4 sw=4 syntax=cmake:
#############################################################################
# Parameters:
#   test_scripts - list of script files to be generated
#   binary_dir   - output directory for the generated scripts
#############################################################################
file(READ test.lib.q TEST_LIB)

foreach(script ${test_scripts})
    configure_file(${script}.in ${binary_dir}/${script} @ONLY)
endforeach()
