#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>
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
#   include <unistd.h>
#else
#   error FIXME: add unit test for this platform...
#endif

namespace q_ffi
{
#ifdef PLATFORM_LINUX
    static string getTestDllPath()
    {
        char path[PATH_MAX + 1]{};
        auto n = readlink("/proc/self/exe", path, PATH_MAX);
        assert(n <= PATH_MAX);

        char* p = strrchr(path, '/');
        assert(p != nullptr && p <= path + PATH_MAX);

        auto const DLL_NAME = "libtest_q_ffi_dll.so";
        strncpy(p + 1, DLL_NAME, PATH_MAX - strlen(path));
        return path;
    }
#endif

    class FFCallTests : public ::testing::Test
    {
    protected:
        string const QBINs
#           ifdef PLATFORM_X86
                [2]{ "q32", "q" };
#           elif PLATFORM_X86_64
                [2]{ "q64", "q" };
#           else
#               error FIXME
#           endif
        string const EXTRA_OPTS{ "-s 8" };
        string const TEST_SCRIPT{ "test_ffcall.q" };
        string const TEST_DUMP{ "test_ffcall.out" };

        struct TestInfo
        {
            string func_;
            string abi_;
            string retType_;
            string argTypes_;
            bool noThrow_;
        };

        static vector<TestInfo> TEST_CASES;

    private:
        static bool exists(std::string const& filename)
        {
            ifstream f{ filename };
            return f.good();
        }

        void cleanUp(bool permissive = false)
        {
            if (exists(TEST_DUMP)) {
                auto const status = remove(TEST_DUMP.c_str());
                if (!permissive)
                    ASSERT_EQ(status, 0) << strerror(status);
            }
        }

        string makeCommand(string const& qbin, vector<string> const& params)
        {
            ostringstream buffer;
            buffer << qbin << ' ' << TEST_SCRIPT << " -q " << EXTRA_OPTS;
            for (auto const& p : params) {
                buffer << ' ';
                if (p.empty() || string::npos != p.find(' '))
                    buffer << '"' << p << '"';
                else
                    buffer << p;
            }
            buffer << " 2>&1 > " << TEST_DUMP;
            return buffer.str();
        }

        int runCommand(string const& cmd)
        {
            cleanUp();
            cout.flush();
            auto const status = system(cmd.c_str());
            cout << ifstream(TEST_DUMP).rdbuf() << endl;
            return status;
        }

    protected:
        void SetUp() override
        {
#       if defined(PLATFORM_WINDOWS)
#       elif defined(PLATFORM_LINUX)
#       else
#           error FIXME
#       endif
        }

        void TearDown() override
        {
            cleanUp(true);
        }

        void runTest(string const& func,
            string const& abi, string const& retType, string const& argTypes)
        {
            int status = -1;
            for (auto const& qbin : QBINs) {
                auto const cmd = makeCommand(qbin, { func, abi, retType, argTypes });
#           ifndef NDEBUG
                cout << "RUNNING: " << cmd << endl;
#           endif
                status = runCommand(cmd);
                if (0 == status)
                    break;
            }
            EXPECT_EQ(status, 0) << "Test script `" << TEST_SCRIPT << "' failed";
        }
    };

    vector<FFCallTests::TestInfo>
    FFCallTests::TEST_CASES = {
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

    TEST_F(FFCallTests, cdeclAdd)
    {
        for (auto const& testCase : TEST_CASES) {
            if (testCase.abi_ != "" && testCase.abi_ != "cdecl")
                continue;
            ostringstream buffer;
            buffer << testCase.func_ << '[' << testCase.argTypes_ << ']';
            SCOPED_TRACE(buffer.str());
            runTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

    TEST_F(FFCallTests, stdcallAdd)
    {
        for (auto const& testCase : TEST_CASES) {
            if (testCase.abi_ != "stdcall")
                continue;
            ostringstream buffer;
            buffer << testCase.func_ << '[' << testCase.argTypes_ << ']';
            SCOPED_TRACE(buffer.str());
            runTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

    TEST_F(FFCallTests, fastcallAdd)
    {
        for (auto const& testCase : TEST_CASES) {
            if (testCase.abi_ != "fastcall")
                continue;
            ostringstream buffer;
            buffer << testCase.func_ << '[' << testCase.argTypes_ << ']';
            SCOPED_TRACE(buffer.str());
            runTest(testCase.func_, testCase.abi_, testCase.retType_, testCase.argTypes_);
        }
    }

}//namespace q_ffi