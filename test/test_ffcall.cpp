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
        static vector<TestInfo> ABI_TEST_CASES;

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
                if (p.empty() || string::npos != p.find(' '))
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

        void runAbiTest(string const& func,
            string const& abi, string const& retType, string const& argTypes)
        {
            auto const testDump = outputName();
            static constexpr auto testScript = "test_ffcall_add.q";
            int status = -1;
            for (auto const& qbin : QBINs) {
                auto const cmd = makeCommand(qbin, testDump, testScript, {
                    func, abi, retType, argTypes
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

#pragma region Tests for different calling conventions (ABIs)

    vector<FFCallTests::TestInfo>
    FFCallTests::ABI_TEST_CASES = {
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
        for (auto const& testCase : ABI_TEST_CASES) {
            if (testCase.abi_ != "")
                continue;

            SCOPED_TRACE(testCase.name());
            runAbiTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

    TEST_F(FFCallTests, addCdeclQ)
    {
        for (auto const& testCase : ABI_TEST_CASES) {
            if (testCase.abi_ != "cdecl")
                continue;

            SCOPED_TRACE(testCase.name());
            runAbiTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

    TEST_F(FFCallTests, addStdcallQ)
    {
        for (auto const& testCase : ABI_TEST_CASES) {
            if (testCase.abi_ != "stdcall")
                continue;

            SCOPED_TRACE(testCase.name());
            runAbiTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

    TEST_F(FFCallTests, addFastcallQ)
    {
        for (auto const& testCase : ABI_TEST_CASES) {
            if (testCase.abi_ != "fastcall")
                continue;

            SCOPED_TRACE(testCase.name());
            runAbiTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

#pragma endregion

#pragma region Tests for different argument/type combinations

    TEST_F(FFCallTests, argsQ)
    {
        SCOPED_TRACE("foreign-function argument combos");
        runGenericTest("test_ffcall_args.q");
    }

#pragma endregion

#pragma region Tests for different variable types

    TEST_F(FFCallTests, varsQ)
    {
        SCOPED_TRACE("foreign variables");
        runGenericTest("test_ffcall_vars.q");
    }

#pragma endregion
}//namespace q_ffi
