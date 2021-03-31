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

        string makeCommand(string const& qbin)
        {
            ostringstream buffer;
            buffer << qbin << ' ' << TEST_SCRIPT << ' ' << EXTRA_OPTS
                << " 2>&1 > " << TEST_DUMP;
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

        void runTest()
        {
            bool success = false;
            for (auto const& qbin : QBINs) {
                if (success)
                    continue;
                auto const status = runCommand(makeCommand(qbin));
                switch (static_cast<errc>(status)) {
                case errc{ 0 }:
                    success = true;
                    break;
                case errc::operation_not_permitted:
                    cerr << "# Failed to start test script with"
                        << " `" << qbin << "', retrying..." << endl;
                    continue;
                default:
                    ASSERT_EQ(status, 0) << strerror(status);
                }
            }
            EXPECT_TRUE(success) << "Failed to start test script"
                << " `" << TEST_SCRIPT << "'";
        }
    };

    TEST_F(FFCallTests, cdeclAdd)
    {
        runTest();
    }

}//namespace q_ffi