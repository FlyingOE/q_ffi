#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <sstream>
#include <fstream>

using namespace std;

#if defined(_WIN32)
#   define PLATFORM_WINDOWS
#   ifdef _WIN64
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#   endif
#elif defined(__linux__) && !defined(__ANDROID__)
#   define PLATFORM_LINUX
#   ifdef __x86_64__
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#   endif
#   include <k_compat.h>
    extern "C"
    {   
        ::K nyi() { throw logic_error("'nyi: use of dll-only function outside of q"); }
        /**
         * Following functions are only available in q, but not in dll.
         * In order to use libq_ffi.so outside of q, we need dummy implementations for them!
         */
        ::K dl(::V*, ::J) { return nyi(); }
        ::K dot(::K, ::K) { return nyi(); }
    }
#else
#   error FIXME: add unit test for this platform...
#endif

namespace q_ffi
{
    class FFCallTests : public ::testing::Test
    {
    protected:
        string const QBINs
#           if defined(PLATFORM_X86)
                [2]{ "q32", "q" };
#           elif defined(PLATFORM_X86_64)
                [2]{ "q64", "q" };
#           else
#               error FIXME
#           endif
        string const EXTRA_OPTS{ "-s 8" };

    private:
        struct TestInfo
        {
            string func_;
            string abi_;
            string retType_;
            string argTypes_;
            bool noThrow_;

            string name() const
            {
                ostringstream buffer;
                buffer << retType_ << ' ' << func_ << '[';
                bool first = true;
                for (char a : argTypes_) {
                    if (first)
                        first = false;
                    else
                        buffer << ';';
                    buffer << a;
                }
                buffer << ']';
                return buffer.str();
            }
        };

    protected:
        static vector<TestInfo> ABI_ATOM_TESTS;
        static vector<TestInfo> ABI_LIST_TESTS;

        static bool exists(string const& filename)
        {
            ifstream f{ filename };
            return f.good();
        }

        static void cleanUp(string const& filename, bool ignoreError = false)
        {
            if (exists(filename)) {
                auto const status = remove(filename.c_str());
                if (!ignoreError)
                    ASSERT_EQ(status, 0) << strerror(status);
            }
        }

        string outputName() const
        {
            auto const testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
            ostringstream testDump;
            testDump << testInfo->test_suite_name() << '.' << testInfo->name() << ".out";
            return testDump.str();
        }

        string makeCommand(string const& qbin, string const& testDump,
            string const& testScript, vector<string> const& params) const
        {
            ostringstream buffer;
            buffer << qbin << ' ' << testScript << " -q " << EXTRA_OPTS;
            for (auto const& p : params) {
                buffer << ' ';
                if (p.empty() || string::npos != p.find(' ') || string::npos != p.find('&'))
                    buffer << '"' << p << '"';
                else
                    buffer << p;
            }
            buffer << " 2>&1 > " << testDump;
            return buffer.str();
        }

        int runCommand(string const& cmd, string const& testDump)
        {
            cleanUp(testDump);
            cout.flush();
            auto const status = system(cmd.c_str());
            cout << ifstream(testDump).rdbuf() << endl;
            return status;
        }

        void SetUp() override
        {
#       if defined(PLATFORM_WINDOWS)
#       elif defined(PLATFORM_LINUX)
#       else
#           error FIXME
#       endif
        }

        void runAbiTestCase(char const* testScript, string const& func,
            string const& abi, string const& retType, string const& argTypes)
        {
            auto const testDump = outputName();
            int status = -1;
            for (auto const& qbin : QBINs) {
                auto const cmd = makeCommand(qbin, testDump, testScript, {
                    func, abi, retType
                });
#           ifndef NDEBUG
                cout << "RUNNING: " << cmd << endl;
#           endif
                status = runCommand(cmd, testDump);
                if (0 == status)
                    break;
            }
            ASSERT_EQ(status, 0) << "Test script `" << testScript << "' failed";
        }

        void runAbiTests(vector<TestInfo> const& tests, char const* abi, bool isAtom)
        {
            for (auto const& testCase : tests) {
                if (testCase.abi_ == abi) {
                    SCOPED_TRACE(testCase.name());
                    runAbiTestCase(isAtom ? "test_ffcall_add.q" : "test_ffcall_adds.q",
                        testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
                }
            }
        }

        void runGenericTest(string const& testScript)
        {
            auto const testDump = outputName();
            int status = -1;
            for (auto const& qbin : QBINs) {
                auto const cmd = makeCommand(qbin, testDump, testScript, {});
#           ifndef NDEBUG
                cout << "RUNNING: " << cmd << endl;
#           endif
                status = runCommand(cmd, testDump);
                if (0 == status)
                    break;
            }
            ASSERT_EQ(status, 0) << "Test script `" << testScript << "' failed";
        }
    };

#pragma region Tests for various calling conventions (ABIs) for atoms

    vector<FFCallTests::TestInfo>
    FFCallTests::ABI_ATOM_TESTS = {
        { "add_char_cdecl"   , "", "x", "xx", true },
        { "add_int16_t_cdecl", "", "h", "hh", true },
        { "add_int32_t_cdecl", "", "i", "ii", true },
        { "add_int64_t_cdecl", "", "j", "jj", true },
        { "add_float_cdecl"  , "", "e", "ee", true },
        { "add_double_cdecl" , "", "f", "ff", true },
#   if defined(PLATFORM_X86)
        { "add_char_cdecl"   , "cdecl", "x", "xx", true },
        { "add_int16_t_cdecl", "cdecl", "h", "hh", true },
        { "add_int32_t_cdecl", "cdecl", "i", "ii", true },
        { "add_int64_t_cdecl", "cdecl", "j", "jj", true },
        { "add_float_cdecl"  , "cdecl", "e", "ee", true },
        { "add_double_cdecl" , "cdecl", "f", "ff", true },
        { "_add_char_stdcall@8"    , "stdcall", "x", "xx", true },
        { "_add_int16_t_stdcall@8" , "stdcall", "h", "hh", true },
        { "_add_int32_t_stdcall@8" , "stdcall", "i", "ii", true },
        { "_add_int64_t_stdcall@16", "stdcall", "j", "jj", true },
        { "_add_float_stdcall@8"   , "stdcall", "e", "ee", true },
        { "_add_double_stdcall@16" , "stdcall", "f", "ff", true },
        { "@add_char_fastcall@8"    , "fastcall", "x", "xx", true },
        { "@add_int16_t_fastcall@8" , "fastcall", "h", "hh", true },
        { "@add_int32_t_fastcall@8" , "fastcall", "i", "ii", true },
        { "@add_int64_t_fastcall@16", "fastcall", "j", "jj", true },
        { "@add_float_fastcall@8"   , "fastcall", "e", "ee", true },
        { "@add_double_fastcall@16" , "fastcall", "f", "ff", true },
#   elif defined(PLATFORM_X86_64)
        { "add_char_stdcall"   , "", "x", "xx", true },
        { "add_int16_t_stdcall", "", "h", "hh", true },
        { "add_int32_t_stdcall", "", "i", "ii", true },
        { "add_int64_t_stdcall", "", "j", "jj", true },
        { "add_float_stdcall"  , "", "e", "ee", true },
        { "add_double_stdcall" , "", "f", "ff", true },
        { "add_char_fastcall"   , "", "x", "xx", true },
        { "add_int16_t_fastcall", "", "h", "hh", true },
        { "add_int32_t_fastcall", "", "i", "ii", true },
        { "add_int64_t_fastcall", "", "j", "jj", true },
        { "add_float_fastcall"  , "", "e", "ee", true },
        { "add_double_fastcall" , "", "f", "ff", true },
#   endif
    };

    TEST_F(FFCallTests, addQ)
    {
        constexpr bool is_atom = true;
        runAbiTests(ABI_ATOM_TESTS, "", is_atom);
    }

    TEST_F(FFCallTests, addCdeclQ)
    {
        constexpr bool is_atom = true;
        runAbiTests(ABI_ATOM_TESTS, "cdecl", is_atom);
    }

    TEST_F(FFCallTests, addStdcallQ)
    {
        constexpr bool is_atom = true;
        runAbiTests(ABI_ATOM_TESTS, "stdcall", is_atom);
    }

    TEST_F(FFCallTests, addFastcallQ)
    {
        constexpr bool is_atom = true;
        runAbiTests(ABI_ATOM_TESTS, "fastcall", is_atom);
    }

#pragma endregion

#pragma region Tests for various calling conventions (ABIs) for lists

    vector<FFCallTests::TestInfo>
    FFCallTests::ABI_LIST_TESTS = {
        { "adds_char_cdecl"   , "", "X", "XX", true },
        { "adds_int16_t_cdecl", "", "H", "HH", true },
        { "adds_int32_t_cdecl", "", "I", "II", true },
        { "adds_int64_t_cdecl", "", "J", "JJ", true },
        { "adds_float_cdecl"  , "", "E", "EE", true },
        { "adds_double_cdecl" , "", "F", "FF", true },
#   if defined(PLATFORM_X86)
        { "adds_char_cdecl"   , "cdecl", "X", "XX", true },
        { "adds_int16_t_cdecl", "cdecl", "H", "HH", true },
        { "adds_int32_t_cdecl", "cdecl", "I", "II", true },
        { "adds_int64_t_cdecl", "cdecl", "J", "JJ", true },
        { "adds_float_cdecl"  , "cdecl", "E", "EE", true },
        { "adds_double_cdecl" , "cdecl", "F", "FF", true },
        { "_adds_char_stdcall@12"    , "stdcall", "X", "XX", true },
        { "_adds_int16_t_stdcall@12" , "stdcall", "H", "HH", true },
        { "_adds_int32_t_stdcall@12" , "stdcall", "I", "II", true },
        { "_adds_int64_t_stdcall@12", "stdcall", "J", "JJ", true },
        { "_adds_float_stdcall@12"   , "stdcall", "E", "EE", true },
        { "_adds_double_stdcall@12" , "stdcall", "F", "FF", true },
        { "@adds_char_fastcall@12"    , "fastcall", "X", "XX", true },
        { "@adds_int16_t_fastcall@12" , "fastcall", "H", "HH", true },
        { "@adds_int32_t_fastcall@12" , "fastcall", "I", "II", true },
        { "@adds_int64_t_fastcall@12", "fastcall", "J", "JJ", true },
        { "@adds_float_fastcall@12"   , "fastcall", "E", "EE", true },
        { "@adds_double_fastcall@12" , "fastcall", "F", "FF", true },
#   elif defined(PLATFORM_X86_64)
        { "adds_char_stdcall"   , "", "X", "XX", true },
        { "adds_int16_t_stdcall", "", "H", "HH", true },
        { "adds_int32_t_stdcall", "", "I", "II", true },
        { "adds_int64_t_stdcall", "", "J", "JJ", true },
        { "adds_float_stdcall"  , "", "E", "EE", true },
        { "adds_double_stdcall" , "", "F", "FF", true },
        { "adds_char_fastcall"   , "", "X", "XX", true },
        { "adds_int16_t_fastcall", "", "H", "HH", true },
        { "adds_int32_t_fastcall", "", "I", "II", true },
        { "adds_int64_t_fastcall", "", "J", "JJ", true },
        { "adds_float_fastcall"  , "", "E", "EE", true },
        { "adds_double_fastcall" , "", "F", "FF", true },
#   endif
    };

    TEST_F(FFCallTests, addsQ)
    {
        constexpr bool is_atom = false;
        runAbiTests(ABI_LIST_TESTS, "", is_atom);
    }

    TEST_F(FFCallTests, addsCdeclQ)
    {
        constexpr bool is_atom = false;
        runAbiTests(ABI_LIST_TESTS, "cdecl", is_atom);
    }

    TEST_F(FFCallTests, addsStdcallQ)
    {
        constexpr bool is_atom = false;
        runAbiTests(ABI_LIST_TESTS, "stdcall", is_atom);
    }

    TEST_F(FFCallTests, addsFastcallQ)
    {
        constexpr bool is_atom = false;
        runAbiTests(ABI_LIST_TESTS, "fastcall", is_atom);
    }

#pragma endregion

#pragma region Tests for various argument/type combinations

    TEST_F(FFCallTests, argumentsQ)
    {
        SCOPED_TRACE("foreign-function argument combos");
        runGenericTest("test_ffcall_args.q");
    }

#pragma endregion

#pragma region Tests for various variable types

    TEST_F(FFCallTests, variablesQ)
    {
        SCOPED_TRACE("foreign variables");
        runGenericTest("test_ffcall_vars.q");
    }

#pragma endregion

#pragma region Tests for various pointer types

    TEST_F(FFCallTests, pointersQ)
    {
        SCOPED_TRACE("pointer arguments");
        runGenericTest("test_ffcall_neg.q");
    }

#pragma endregion
}//namespace q_ffi
