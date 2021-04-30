# vim: set et noai ts=4 sw=4 syntax=cmake:
#############################################################################
# Parameters:
#   test_scripts - list of script files to be generated
#   binary_dir   - output directory for the generated scripts
#   test_dll     - name (less filext) of the test DLL
#   dll_suffix   - suffix of DLL filename
#############################################################################
foreach(script ${test_scripts})
    if(${CMAKE_VERSION} VERSION_LESS "3.20")
        get_filename_component(script_file ${script} NAME)
    else()
        cmake_path(GET ${script} FILENAME script_file)
    endif()

    configure_file(${script} ${binary_dir}/${script_file} @ONLY)

endforeach()
