#include <gtest/gtest.h>
#include "type_convert.hpp"
#include "ktype_traits.hpp"
#include "kpointer.hpp"
#include "kerror.hpp"

namespace q
{

    class TypeConvertTests : public ::testing::Test
    {
    protected:
        template<typename T, T q2Atom(::K)>
        void test_q2Decimal()
        {
            test_q2Atom<T, q2Atom>(::kb, true, "1b");
            test_q2Atom<T, q2Atom>(::kb, false, "0b");
            test_q2Atom_with_list<T, q2Atom>(kBoolean, "*b list");

            test_q2Atom<T, q2Atom>(::kg, 0xF3, "0xF3");
            test_q2Atom<T, q2Atom>(::kg, 0, "0x00");
            test_q2Atom_with_list<T, q2Atom>(kByte, "0x* list");

            test_q2Atom<T, q2Atom>(::kh, 0, "0h");
            test_q2Atom<T, q2Atom>(::kh, 27856, "27856h");
            test_q2Atom_with_list<T, q2Atom>(kShort, "*h list");

            test_q2Atom<T, q2Atom>(::ki, 0, "0i");
            test_q2Atom<T, q2Atom>(::ki, 476883888, "476883888i");
            test_q2Atom_with_list<T, q2Atom>(kInt, "*i list");

            test_q2Atom<T, q2Atom>(::kj, 0, "0j");
            test_q2Atom<T, q2Atom>(::kj, 7413760571230203088L, "7413760571230203088j");
            test_q2Atom_with_list<T, q2Atom>(kLong, "*j list");

            K_ptr k;
            k.reset(ks("hello"));
            EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "`hello";

            k.reset(ktn(kSymbol, 1));
            TypeTraits<kSymbol>::index(k.get())[0] = ::ss("hello");
            EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "1#`hello";

            k.reset(knk(0));
            EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "()";
        }

        template<typename T, T q2Atom(::K), typename createAtom, typename V>
        void test_q2Atom(createAtom const& create, V const& val, char const* qStr)
        {
            K_ptr k{ create(val) };
            ASSERT_NE(k.get(), nullptr);
            EXPECT_EQ(q2Atom(k.get()), val) << "q)" << qStr;
        }


        template<typename T, T q2Atom(::K)>
        void test_q2Atom_with_list(TypeId tid, char const* qStr)
        {
            K_ptr k{ ::ktn(tid, 1) };
            EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << qStr;
        }

    protected:
        template<typename T, std::vector<T> q2List(::K)>
        void test_q2Decimals()
        {
            test_q2List<T, q2List, kBoolean>(std::vector<bool>{
                true, false
            });
            test_q2List_with_atom<T, q2List>(::kb, true);

            test_q2List<T, q2List, kByte>(std::vector<unsigned char>{
                0x16, 0, 0xF3
            });
            test_q2List_with_atom<T, q2List>(::kg, 0xF3);

            test_q2List<T, q2List, kShort>(std::vector<::H>{
                -14733, 0, 27856, nh, wh
            });
            test_q2List_with_atom<T, q2List>(::kh, 27856);

            test_q2List<T, q2List, kInt>(std::vector<::I>{
                -773686186, 0, 476883888, ni, wi
            });
            test_q2List_with_atom<T, q2List>(::ki, 476883888);

            test_q2List<T, q2List, kLong>(std::vector<::J>{
                -26995309665792L, 0, 7413760571230203088L, nj, wj
            });
            test_q2List_with_atom<T, q2List>(::kj, 7413760571230203088L);

            K_ptr k;
            k.reset(ks("hello"));
            EXPECT_THROW(q2List(k.get()), K_error) << "q)" << "`hello";

            k.reset(ktn(KS, 1));
            TypeTraits<kSymbol>::index(k.get())[0] = ::ss("hello");
            EXPECT_THROW(q2List(k.get()), K_error) << "q)" << "1#`hello";
        }

        template<typename T, std::vector<T> q2List(::K), TypeId tid, typename S>
        void test_q2List(S const& samples)
        {
            K_ptr k;
            init_qList<tid>(k, samples);
            ASSERT_NE(k.get(), nullptr);
            verify_qList(tid, q2List(k.get()), samples);
        }

        template<typename T, std::vector<T> q2List(::K), typename createAtom, typename V>
        void test_q2List_with_atom(createAtom const& create, V const& val)
        {
            K_ptr k{ create(val) };
            ASSERT_NE(k.get(), nullptr);
            EXPECT_THROW(q2List(k.get()), K_error);
        }

        template<TypeId tid, typename T>
        void init_qList(K_ptr& pk, std::vector<T> const& samples)
        {
            ASSERT_FALSE(samples.empty());
            K_ptr k{ ::ktn(tid, samples.size()) };
            ASSERT_EQ(count(k.get()), samples.size());
            auto p = TypeTraits<tid>::index(k.get());
            ASSERT_NE(p, nullptr);
            for (auto const& s : samples)
                *(p++) = s;
            pk.reset(k.release());
        }

        template<typename T0, typename T1>
        void verify_qList(TypeId tid, std::vector<T0> const& expected, std::vector<T1> const& values)
        {
            ASSERT_EQ(expected.size(), values.size()) << tid << " list lengths mismatch";
            auto e = expected.cbegin();
            auto v = values.cbegin();
            for (; e != expected.cend(); ++e, ++v)
                EXPECT_EQ(*e, *v) << tid << " list mismatch @ [" << (e - expected.cbegin()) << ']';
        }

    protected:
        template<typename T, T q2Atom(::K)>
        void test_temporal(::K kt, T const& t, char const* qStr)
        {
            ASSERT_NE(kt, nullptr);
            K_ptr k{ kt };
            EXPECT_EQ(q2Atom(k.get()), t) << "q)" << qStr;
        }

    };//class TypeConvertTests

#   pragma region q2Decimal|q2Decimals

    TEST_F(TypeConvertTests, q2Decimal)
    {
        {
            SCOPED_TRACE("q::q2Decimal on integral atoms");
            this->test_q2Decimal<long long, q2Decimal>();
        }

        K_ptr k;
        k.reset(kf(-123.456));
        EXPECT_THROW(q2Decimal(k.get()), K_error) << "q)" << "-123.456";

        k.reset(ktn(KF, 1));
        TypeTraits<kFloat>::index(k.get())[0] = -123.456;
        EXPECT_THROW(q2Decimal(k.get()), K_error) << "q)" << "1#-123.456";
    }

    TEST_F(TypeConvertTests, q2Decimals)
    {
        {
            SCOPED_TRACE("q::q2Decimals on integral lists");
            this->test_q2Decimals<long long, q2Decimals>();
        }

        K_ptr k;
        k.reset(kf(-123.456));
        EXPECT_THROW(q2Decimals(k.get()), K_error) << "q)" << "-123.456";

        k.reset(ktn(KF, 1));
        TypeTraits<kFloat>::index(k.get())[0] = -123.456;
        EXPECT_THROW(q2Decimals(k.get()), K_error) << "q)" << "1#-123.456";
    }

#   pragma endregion

#   pragma region q2Real|q2Reals
    TEST_F(TypeConvertTests, q2Real)
    {
        {
            SCOPED_TRACE("q::q2Real on integral atoms");
            this->test_q2Decimal<double, q2Real>();
        } {
            SCOPED_TRACE("q::q2Real on q real");
            this->test_q2Atom<double, q2Real>(::ke, 967.23981f, "967.23981e");
            this->test_q2Atom<double, q2Real>(::ke, 0.f, "0e");
            this->test_q2Atom<double, q2Real>(::ke, -967.23981f, "-967.23981e");
            this->test_q2Atom<double, q2Real>(::ke, ::E(wf), "0We");
            this->test_q2Atom<double, q2Real>(::ke, ::E(-wf), "-0We");
        } {
            SCOPED_TRACE("q::q2Real on q reals");
            this->test_q2Atom_with_list<double, q2Real>(kReal, "*e list");
        } {
            SCOPED_TRACE("q::q2Real on q float");
            this->test_q2Atom<double, q2Real>(::kf, 7413760.571230203088, "7413760.571230203088");
            this->test_q2Atom<double, q2Real>(::kf, 0., "0f");
            this->test_q2Atom<double, q2Real>(::kf, -7413760.571230203088, "-7413760.571230203088");
            this->test_q2Atom<double, q2Real>(::kf, wf, "0w");
            this->test_q2Atom<double, q2Real>(::kf, -wf, "-0w");
        } {
            SCOPED_TRACE("q::q2Real on q floats");
            this->test_q2Atom_with_list<double, q2Real>(kFloat, "*f list");
        }
    }

    TEST_F(TypeConvertTests, q2Reals)
    {
        {
            SCOPED_TRACE("q::q2Reals on integral lists");
            this->test_q2Decimals<double, q2Reals>();
        }
        {
            SCOPED_TRACE("q::q2Reals on q reals");
            this->test_q2List<double, q2Reals, kReal>(std::vector<::E>{
                967.23981f, 0.f, -967.23981f, float(wf), float(-wf)
            });
        }
        K_ptr k;
        k.reset(::ke(0.f));
        EXPECT_THROW(q2Reals(k.get()), K_error) << "q)" << "0e";

        {
            SCOPED_TRACE("q::q2Reals on q floats");
            this->test_q2List<double, q2Reals, kFloat>(std::vector<::F>{
                7413760.571230203088, 0., -7413760.571230203088, wf, -wf
            });
        }
        k.reset(::kf(0.));
        EXPECT_THROW(q2Reals(k.get()), K_error) << "q)" << "0.";
    }

#   pragma endregion

#   pragma region q2String|q2Strings

    TEST_F(TypeConvertTests, q2String)
    {
        {
            SCOPED_TRACE("q::q2String on q symbol");
            this->test_q2Atom<std::string, q2String>(::ks,
                const_cast<::S>("hello"), "`hello");
            this->test_q2Atom<std::string, q2String>(::ks,
                const_cast<::S>(""), "`");
            this->test_q2Atom<std::string, q2String>(::ks,
                const_cast<::S>("Hello world!"), "`$\"Hello world!\"");
        } {
            SCOPED_TRACE("q::q2String on q symbols");
            this->test_q2Atom_with_list<std::string, q2String>(kSymbol, "`* list");
        }
        {
            SCOPED_TRACE("q::q2String on q char list");
            this->test_q2Atom<std::string, q2String>(::kp,
                const_cast<::S>("hello"), "\"hello\"");
            this->test_q2Atom<std::string, q2String>(::kp,
                const_cast<::S>("A"), "1#\"A\"");
            this->test_q2Atom<std::string, q2String>(::kp,
                const_cast<::S>(""), "\"\"");
            this->test_q2Atom<std::string, q2String>(::kp,
                const_cast<::S>("Hello world!"), "\"Hello world!\"");
        } {
            SCOPED_TRACE("q::q2String on q char lists");
            this->test_q2Atom_with_list<std::string, q2String>(kMixed, "\"*\" list");
        }
        {
            // Non-string
            K_ptr k{ ::kf(-123.456) };
            EXPECT_THROW(q2String(k.get()), K_error) << "q)" << "-123.456";
        }
    }

    TEST_F(TypeConvertTests, q2Strings)
    {
        K_ptr k;
        std::vector<::S> const samples{ "hello", "", "Hello world!" };
        {
            SCOPED_TRACE("q::q2Strings on q symbols");
            k.reset(TypeTraits<kSymbol>::list(samples.begin(), samples.end()));
            this->verify_qList(kSymbol, q2Strings(k.get()), samples);
        } {
            SCOPED_TRACE("q::q2Strings on q symbol");
            this->test_q2List_with_atom<std::string, q2Strings>(::ks, const_cast<S>("hello"));
        }
        {
            SCOPED_TRACE("q::q2Strings on q char lists");
            this->test_q2List_with_atom<std::string, q2Strings>(::ks, const_cast<S>("hello"));
            k.reset(knk(3, kp(samples[0]), kp(samples[1]), kp(samples[2])));
            this->verify_qList(kMixed, q2Strings(k.get()), samples);
        } {
            SCOPED_TRACE("q::q2Strings on q char list");
            this->test_q2List_with_atom<std::string, q2Strings>(::kp, const_cast<S>("hello"));
        }
        {
            // Mixed list
            K_ptr k;
            k.reset(knk(2, ks("hello"), kf(-123.456)));
            EXPECT_THROW(q2Strings(k.get()), K_error) << "q)" << "(\"hello\";-123.456)";

            k.reset(::kf(-123.456));
            EXPECT_THROW(q2Strings(k.get()), K_error) << "q)" << "-123.456";
        }
    }

#   pragma endregion

#   pragma region q2DateTime|q2DateTimes

    TEST_F(TypeConvertTests, q2DateTime)
    {
        K_ptr k;
/*
        tm_ext tm{};

#   define INIT_Q2DATETIME(kx, YYYY, mm, dd, HH, MM, SS, nanos) \
        k.reset((kx));  \
        std::memset(&tm, 0, sizeof(tm));    \
        tm.tm_year = (YYYY) - 1900; \
        tm.tm_mon = (mm) - 1;   \
        tm.tm_mday = (dd);  \
        tm.tm_hour = (HH);  \
        tm.tm_min = (MM);   \
        tm.tm_sec = (SS);   \
        tm.tm_nanos = (nanos);  \
        tm.mktime()
#   define TEST_Q2DATETIME(kx, YYYY, mm, dd, HH, MM, SS, nanos, qStr) \
        INIT_Q2DATETIME(kx, YYYY, mm, dd, HH, MM, SS, nanos);   \
        EXPECT_EQ(q2DateTime(k.get()), tm) << "q)" << (qStr)

#   define TEST_Q2DATETIME_VALUE(kx, qStr) \
        k.reset((kx));  \
        EXPECT_THROW(q2DateTime(k.get()), K_error) << "q)" << (qStr)
*/
/*
        TEST_Q2DATETIME(kd(     0), 2000,  1,  1, 0, 0, 0, 0, "2000.01.01");
        TEST_Q2DATETIME(kd(  5703), 2015,  8, 13, 0, 0, 0, 0, "2015.08.13");
        TEST_Q2DATETIME(kd(-10957), 1970,  1,  1, 0, 0, 0, 0, "1970.01.01");
        TEST_Q2DATETIME(kd( 13898), 2038,  1, 19, 0, 0, 0, 0, "2038.01.19");
        TEST_Q2DATETIME(kd(-10958), 1969, 12, 31, 0, 0, 0, 0, "1969.12.31");
        TEST_Q2DATETIME(kd( 13899), 2038,  1, 20, 0, 0, 0, 0, "2038.01.20");
        INIT_Q2DATETIME(kd(-10958), 1969, 12, 31, 0, 0, 0, 0);
        EXPECT_THROW(q2DateTime(k.get()), K_error) << "q)" << "1969.12.31";
        INIT_Q2DATETIME(kd(13899), 2038, 1, 20, 0, 0, 0, 0);
        EXPECT_THROW(q2DateTime(k.get()), K_error) << "q)" << "2038.01.20";
        TEST_Q2DATETIME_VALUE(kd(wi), "0Wd");
        TEST_Q2DATETIME_VALUE(kd(-wi), "-0Wd");
//        EXPECT_EQ(q2DateTime(kd(0)), q2DateTime(kd(ni))) << "q)" << "0Nd";

//        TEST_Q2DATETIME(kt(0), 0, 0, 0, 0, 0, 0, 0, "0t");
 */
    }

    TEST_F(TypeConvertTests, q2DateTimes)
    {
        K_ptr k;

    }

#   pragma endregion

}//namespace q
