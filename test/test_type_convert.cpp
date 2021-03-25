#include <gtest/gtest.h>
#include "type_convert.hpp"
#include "ktype_traits.hpp"
#include "kpointer.hpp"
#include "kerror.hpp"

using namespace std;

namespace q
{
    namespace tester
    {
        inline long long q2Decimal(::K k)
        { return q::q2Decimal(k); }
        inline vector<long long> q2Decimals(::K k)
        { return q::q2Decimals(k); }

        inline double q2Real(::K k)
        { return q::q2Real(k); }
        inline vector<double> q2Reals(::K k)
        { return q::q2Reals(k); }

        inline string q2String(::K k)
        { return q::q2String(k); }
        inline vector<string> q2Strings(::K k)
        { return q::q2Strings(k); }
    }

    class TypeConvertTests : public ::testing::Test
    {
    protected:
        template<typename T, T q2Atom(::K)>
        void test_q2Decimal()
        {
            {
                SCOPED_TRACE("q2* on q boolean(s)");
                this->test_q2Atom<T, q2Atom>(::kb, true, "1b");
                this->test_q2Atom<T, q2Atom>(::kb, false, "0b");
                this->test_q2Atom_with_list<T, q2Atom>(kBoolean, "*b list");
            } {
                SCOPED_TRACE("q2* on q byte(s)");
                this->test_q2Atom<T, q2Atom>(::kg, 0xF3, "0xF3");
                this->test_q2Atom<T, q2Atom>(::kg, 0, "0x00");
                this->test_q2Atom_with_list<T, q2Atom>(kByte, "0x* list");
            } {
                SCOPED_TRACE("q2* on q short(s)");
                this->test_q2Atom<T, q2Atom>(::kh, 0, "0h");
                this->test_q2Atom<T, q2Atom>(::kh, 27856, "27856h");
                this->test_q2Atom_with_list<T, q2Atom>(kShort, "*h list");
            } {
                SCOPED_TRACE("q2* on q int(s)");
                this->test_q2Atom<T, q2Atom>(::ki, 0, "0i");
                this->test_q2Atom<T, q2Atom>(::ki, 476883888, "476883888i");
                this->test_q2Atom_with_list<T, q2Atom>(kInt, "*i list");
            } {
                SCOPED_TRACE("q2* on q long(s)");
                this->test_q2Atom<T, q2Atom>(::kj, 0, "0j");
                this->test_q2Atom<T, q2Atom>(::kj, 7413760571230203088L, "7413760571230203088j");
                this->test_q2Atom_with_list<T, q2Atom>(kLong, "*j list");
            } {
                SCOPED_TRACE("q2* on non-numeric type(s)");

                K_ptr k;
                k.reset(ks(const_cast<::S>("hello")));
                EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "`hello";

                k.reset(ktn(kSymbol, 1));
                TypeTraits<kSymbol>::index(k.get())[0] = ::ss(const_cast<::S>("hello"));
                EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "1#`hello";

                k.reset(knk(0));
                EXPECT_THROW(q2Atom(k.get()), K_error) << "q)" << "()";
            }
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
        template<typename T, vector<T> q2List(::K)>
        void test_q2Decimals()
        {
            {
                SCOPED_TRACE("q2*s on q boolean(s)");
                this->test_q2List<T, q2List, kBoolean>(vector<bool>{
                    true, false
                });
                this->test_q2List_with_atom<T, q2List>(::kb, true);
            } {
                SCOPED_TRACE("q2*s on q byte(s)");
                this->test_q2List<T, q2List, kByte>(vector<unsigned char>{
                    0x16, 0, 0xF3
                });
                this->test_q2List_with_atom<T, q2List>(::kg, 0xF3);
            } {
                SCOPED_TRACE("q2*s on q short(s)");
                this->test_q2List<T, q2List, kShort>(vector<::H>{
                    -14733, 0, 27856, nh, wh
                });
                this->test_q2List_with_atom<T, q2List>(::kh, 27856);
            } {
                SCOPED_TRACE("q2*s on q int(s)");
                this->test_q2List<T, q2List, kInt>(vector<::I>{
                    -773686186, 0, 476883888, ni, wi
                });
                this->test_q2List_with_atom<T, q2List>(::ki, 476883888);
            } {
                SCOPED_TRACE("q2*s on q long(s)");
                this->test_q2List<T, q2List, kLong>(vector<::J>{
                    -26995309665792L, 0, 7413760571230203088L, nj, wj
                });
                this->test_q2List_with_atom<T, q2List>(::kj, 7413760571230203088L);
            } {
                SCOPED_TRACE("q2*s on non-numeric type(s)");

                K_ptr k;
                k.reset(ks(const_cast<::S>("hello")));
                EXPECT_THROW(q2List(k.get()), K_error) << "q)" << "`hello";

                k.reset(ktn(KS, 1));
                TypeTraits<kSymbol>::index(k.get())[0] = ::ss(const_cast<::S>("hello"));
                EXPECT_THROW(q2List(k.get()), K_error) << "q)" << "1#`hello";
            }
        }

        template<typename T, vector<T> q2List(::K), TypeId tid, typename S>
        void test_q2List(S const& samples)
        {
            K_ptr k;
            this->init_qList<tid>(k, samples);
            ASSERT_NE(k.get(), nullptr);
            this->verify_qList(tid, q2List(k.get()), samples);
        }

        template<typename T, vector<T> q2List(::K), typename createAtom, typename V>
        void test_q2List_with_atom(createAtom const& create, V const& val)
        {
            K_ptr k{ create(val) };
            ASSERT_NE(k.get(), nullptr);
            EXPECT_THROW(q2List(k.get()), K_error);
        }

        template<TypeId tid, typename T>
        void init_qList(K_ptr& pk, vector<T> const& samples)
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
        void verify_qList(TypeId tid, vector<T0> const& expected, vector<T1> const& values)
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
            this->test_q2Decimal<long long, tester::q2Decimal>();
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
            this->test_q2Decimals<long long, tester::q2Decimals>();
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
            this->test_q2Decimal<double, tester::q2Real>();
        } {
            SCOPED_TRACE("q::q2Real on q real");
            this->test_q2Atom<double, tester::q2Real>(::ke, 967.23981f, "967.23981e");
            this->test_q2Atom<double, tester::q2Real>(::ke, 0.f, "0e");
            this->test_q2Atom<double, tester::q2Real>(::ke, -967.23981f, "-967.23981e");
            this->test_q2Atom<double, tester::q2Real>(::ke, ::E(wf), "0We");
            this->test_q2Atom<double, tester::q2Real>(::ke, ::E(-wf), "-0We");
        } {
            SCOPED_TRACE("q::q2Real on q reals");
            this->test_q2Atom_with_list<double, tester::q2Real>(kReal, "*e list");
        } {
            SCOPED_TRACE("q::q2Real on q float");
            this->test_q2Atom<double, tester::q2Real>(::kf, 7413760.571230203088, "7413760.571230203088");
            this->test_q2Atom<double, tester::q2Real>(::kf, 0., "0f");
            this->test_q2Atom<double, tester::q2Real>(::kf, -7413760.571230203088, "-7413760.571230203088");
            this->test_q2Atom<double, tester::q2Real>(::kf, wf, "0w");
            this->test_q2Atom<double, tester::q2Real>(::kf, -wf, "-0w");
        } {
            SCOPED_TRACE("q::q2Real on q floats");
            this->test_q2Atom_with_list<double, tester::q2Real>(kFloat, "*f list");
        }
    }

    TEST_F(TypeConvertTests, q2Reals)
    {
        {
            SCOPED_TRACE("q::q2Reals on integral lists");
            this->test_q2Decimals<double, tester::q2Reals>();
        }
        {
            SCOPED_TRACE("q::q2Reals on q reals");
            this->test_q2List<double, tester::q2Reals, kReal>(vector<::E>{
                967.23981f, 0.f, -967.23981f, float(wf), float(-wf)
            });
        }
        K_ptr k;
        k.reset(::ke(0.f));
        EXPECT_THROW(q2Reals(k.get()), K_error) << "q)" << "0e";

        {
            SCOPED_TRACE("q::q2Reals on q floats");
            this->test_q2List<double, tester::q2Reals, kFloat>(vector<::F>{
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
            this->test_q2Atom<string, tester::q2String>(::ks,
                const_cast<::S>("hello"), "`hello");
            this->test_q2Atom<string, tester::q2String>(::ks,
                const_cast<::S>(""), "`");
            this->test_q2Atom<string, tester::q2String>(::ks,
                const_cast<::S>("Hello world!"), "`$\"Hello world!\"");
        } {
            SCOPED_TRACE("q::q2String on q symbols");
            this->test_q2Atom_with_list<string, tester::q2String>(kSymbol, "`* list");
        }
        {
            SCOPED_TRACE("q::q2String on q char list");
            this->test_q2Atom<string, tester::q2String>(::kp,
                const_cast<::S>("hello"), "\"hello\"");
            this->test_q2Atom<string, tester::q2String>(::kp,
                const_cast<::S>("A"), "1#\"A\"");
            this->test_q2Atom<string, tester::q2String>(::kp,
                const_cast<::S>(""), "\"\"");
            this->test_q2Atom<string, tester::q2String>(::kp,
                const_cast<::S>("Hello world!"), "\"Hello world!\"");
        } {
            SCOPED_TRACE("q::q2String on q char lists");
            this->test_q2Atom_with_list<string, tester::q2String>(kMixed, "\"*\" list");
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
        vector<::S> const samples{
            const_cast<::S>("hello"), const_cast<::S>(""), const_cast<::S>("Hello world!")
        };
        {
            SCOPED_TRACE("q::q2Strings on q symbols");
            k.reset(TypeTraits<kSymbol>::list(samples.begin(), samples.end()));
            this->verify_qList(kSymbol, q2Strings(k.get()), samples);
        } {
            SCOPED_TRACE("q::q2Strings on q symbol");
            this->test_q2List_with_atom<string, tester::q2Strings>(::ks, const_cast<S>("hello"));
        }
        {
            SCOPED_TRACE("q::q2Strings on q char lists");
            this->test_q2List_with_atom<string, tester::q2Strings>(::ks, const_cast<S>("hello"));
            k.reset(knk(3, kp(samples[0]), kp(samples[1]), kp(samples[2])));
            this->verify_qList(kMixed, q2Strings(k.get()), samples);
        } {
            SCOPED_TRACE("q::q2Strings on q char list");
            this->test_q2List_with_atom<string, tester::q2Strings>(::kp, const_cast<S>("hello"));
        }
        {
            // Mixed list
            K_ptr k;
            k.reset(knk(2, ks(const_cast<::S>("hello")), kf(-123.456)));
            EXPECT_THROW(q2Strings(k.get()), K_error) << "q)" << "(\"hello\";-123.456)";

            k.reset(::kf(-123.456));
            EXPECT_THROW(q2Strings(k.get()), K_error) << "q)" << "-123.456";
        }
    }

#   pragma endregion

    using namespace std::chrono;

#   pragma region q2TimePoint|q2TimePoints

    TEST_F(TypeConvertTests, q2TimePoint)
    {
        {
            SCOPED_TRACE("q2TimePoint on q timestamp(s)");

            K_ptr k{ ::ktj(-KP, "2020.09.10D15:07:01.012345678"_qp) };

            EXPECT_EQ(q2TimePoint<Timestamp>(k.get()),
                Timestamp{ Date{ date::year(2020) / 9 / 10 } }
                    + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'345'678LL });
            EXPECT_EQ(q2TimePoint<DateTime>(k.get()),
                DateTime{ Date{ date::year(2020) / 9 / 10 } }
                    + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 });
            EXPECT_EQ(q2TimePoint<Date>(k.get()),
                Date{ date::year(2020) / 9 / 10 });

            k.reset(::ktj(-KN, 100));
            EXPECT_THROW(q2TimePoint<Timestamp>(k.get()), K_error);

            k.reset(::kj(100));
            EXPECT_THROW(q2TimePoint<Timestamp>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoint on q month(s)");

            K_ptr k{ TypeTraits<kMonth>::atom("2020.09"_qm) };

            EXPECT_EQ(q2TimePoint<Timestamp>(k.get()),
                Timestamp{ Date{ date::year(2020) / 9 / 1 } });
            EXPECT_EQ(q2TimePoint<DateTime>(k.get()),
                DateTime{ Date{ date::year(2020) / 9 / 1 } });
            EXPECT_EQ(q2TimePoint<Date>(k.get()),
                Date{ date::year(2020) / 9 / 1 });

            k.reset(::kt(100));
            EXPECT_THROW(q2TimePoint<Date>(k.get()), K_error);

            k.reset(::ki(100));
            EXPECT_THROW(q2TimePoint<Date>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoint on q datetime(s)");

            K_ptr k{ ::kd("2020.09.10"_qd) };

            EXPECT_EQ(q2TimePoint<Timestamp>(k.get()),
                Timestamp{ Date{ date::year(2020) / 9 / 10 } });
            EXPECT_EQ(q2TimePoint<DateTime>(k.get()),
                DateTime{ Date{ date::year(2020) / 9 / 10 } });
            EXPECT_EQ(q2TimePoint<Date>(k.get()),
                Date{ date::year(2020) / 9 / 10 });

            k.reset(::kt(100));
            EXPECT_THROW(q2TimePoint<Date>(k.get()), K_error);

            k.reset(::ki(100));
            EXPECT_THROW(q2TimePoint<Date>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoint on q datetime(s)");

            K_ptr k{ ::kz("2020.09.10T15:07:01.012"_qz) };

            EXPECT_EQ(q2TimePoint<Timestamp>(k.get()),
                Timestamp{ Date{ date::year(2020) / 9 / 10 } }
                    + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'000'000LL });
            EXPECT_EQ(q2TimePoint<DateTime>(k.get()),
                DateTime{ Date{ date::year(2020) / 9 / 10 } }
                    + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 });
            EXPECT_EQ(q2TimePoint<Date>(k.get()),
                Date{ date::year(2020) / 9 / 10 });

            k.reset(::kt(100));
            EXPECT_THROW(q2TimePoint<DateTime>(k.get()), K_error);

            k.reset(::kf(100.));
            EXPECT_THROW(q2TimePoint<DateTime>(k.get()), K_error);
        }
    }

    TEST_F(TypeConvertTests, q2TimePoints)
    {
        {
            SCOPED_TRACE("q2TimePoints on q timestamp(s)");

            K_ptr k{ ::ktn(KP, 3) };
            kJ(k.get())[0] = "2020.09.10D15:07:01.012345678"_qp;
            kJ(k.get())[1] = "2000.01.01D00:00:00"_qp;
            kJ(k.get())[2] = "1969.12.31D23:59:59.999999999"_qp;

            EXPECT_EQ(q2TimePoints<Timestamp>(k.get()),
                (vector<Timestamp>{
                    Timestamp{ Date{ date::year(2020) / 9 / 10 } }
                        + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'345'678LL },
                    Timestamp{ Date{ Epoch } },
                    Timestamp{ Date{ date::year(1969) / 12 / 31 } }
                        + hours{ 23 } + minutes{ 59 } + seconds{ 59 } + nanoseconds{ 999'999'999LL },
                }));
            EXPECT_EQ(q2TimePoints<DateTime>(k.get()),
                (vector<DateTime>{
                    DateTime{ Date{ date::year(2020) / 9 / 10 } }
                        + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 },
                    DateTime{ Date{ Epoch } },
                    DateTime{ Date{ date::year(1970) / 1 / 1 } },   //rounded up to 1970.01.01
                }));
            EXPECT_EQ(q2TimePoints<Date>(k.get()),
                (vector<Date>{
                    Date{ date::year(2020) / 9 / 10 },
                    Date{ Epoch },
                    Date{ date::year(1970) / 1 / 1 },               //rounded up to 1970.01.01
                }));

            k.reset(::ktn(KN, 3));
            EXPECT_THROW(q2TimePoints<Timestamp>(k.get()), K_error);

            k.reset(::ktn(KJ, 3));
            EXPECT_THROW(q2TimePoints<Timestamp>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoints on q month(s)");

            K_ptr k{ ::ktn(KM, 3) };
            kI(k.get())[0] = "2020.09"_qm;
            kI(k.get())[1] = "2000.01"_qm;
            kI(k.get())[2] = "1969.12"_qm;

            EXPECT_EQ(q2TimePoints<Timestamp>(k.get()),
                (vector<Timestamp>{
                    Timestamp{ Date{ date::year(2020) / 9 / 1 } },
                    Timestamp{ Date{ Epoch } },
                    Timestamp{ Date{ date::year(1969) / 12 / 1 } },
                }));
            EXPECT_EQ(q2TimePoints<DateTime>(k.get()),
                (vector<DateTime>{
                    DateTime{ Date{ date::year(2020) / 9 / 1 } },
                    DateTime{ Date{ Epoch } },
                    DateTime{ Date{ date::year(1969) / 12 / 1 } },
                }));
            EXPECT_EQ(q2TimePoints<Date>(k.get()),
                (vector<Date>{
                    Date{ date::year(2020) / 9 / 1 },
                    Date{ Epoch },
                    Date{ date::year(1969) / 12 / 1 },
                }));

            k.reset(::ktn(KT, 3));
            EXPECT_THROW(q2TimePoints<Date>(k.get()), K_error);

            k.reset(::ktn(KI, 3));
            EXPECT_THROW(q2TimePoints<Date>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoints on q date(s)");

            K_ptr k{ ::ktn(KD, 3) };
            kI(k.get())[0] = "2020.09.10"_qd;
            kI(k.get())[1] = "2000.01.01"_qd;
            kI(k.get())[2] = "1969.12.31"_qd;

            EXPECT_EQ(q2TimePoints<Timestamp>(k.get()),
                (vector<Timestamp>{
                    Timestamp{ Date{ date::year(2020) / 9 / 10 } },
                    Timestamp{ Date{ Epoch } },
                    Timestamp{ Date{ date::year(1969) / 12 / 31 } },
                }));
            EXPECT_EQ(q2TimePoints<DateTime>(k.get()),
                (vector<DateTime>{
                    DateTime{ Date{ date::year(2020) / 9 / 10 } },
                        DateTime{ Date{ Epoch } },
                        DateTime{ Date{ date::year(1969) / 12 / 31 } },
                }));
            EXPECT_EQ(q2TimePoints<Date>(k.get()),
                (vector<Date>{
                    Date{ date::year(2020) / 9 / 10 },
                        Date{ Epoch },
                        Date{ date::year(1969) / 12 / 31 },
                }));

            k.reset(::ktn(KT, 3));
            EXPECT_THROW(q2TimePoints<Date>(k.get()), K_error);

            k.reset(::ktn(KI, 3));
            EXPECT_THROW(q2TimePoints<Date>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimePoints on q datetime(s)");

            K_ptr k{ ::ktn(KZ, 3) };
            kF(k.get())[0] = "2020.09.10T15:07:01.012"_qz;
            kF(k.get())[1] = "2000.01.01T00:00:00"_qz;
            kF(k.get())[2] = "1969.12.31T23:59:59.999"_qz;

            EXPECT_EQ(q2TimePoints<Timestamp>(k.get()),
                (vector<Timestamp>{
                    Timestamp{ Date{ date::year(2020) / 9 / 10 } }
                        + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'000'000LL },
                    Timestamp{ Date{ Epoch } },
                    Timestamp{ Date{ date::year(1969) / 12 / 31 } }
                        + hours{ 23 } + minutes{ 59 } + seconds{ 59 } + nanoseconds{ 999'000'000LL },
                }));
            EXPECT_EQ(q2TimePoints<DateTime>(k.get()),
                (vector<DateTime>{
                    DateTime{ Date{ date::year(2020) / 9 / 10 } }
                        + hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 },
                    DateTime{ Date{ Epoch } },
                    DateTime{ Date{ date::year(1969) / 12 / 31 } }
                        + hours{ 23 } + minutes{ 59 } + seconds{ 59 } + milliseconds{ 999 },
                }));
            EXPECT_EQ(q2TimePoints<Date>(k.get()),
                (vector<Date>{
                    Date{ date::year(2020) / 9 / 10 },
                    Date{ Epoch },
                    Date{ date::year(1970) / 1 / 1 },   //rounded up to 1970.01.01
                }));

            k.reset(::ktn(KT, 3));
            EXPECT_THROW(q2TimePoints<DateTime>(k.get()), K_error);

            k.reset(::ktn(KF, 3));
            EXPECT_THROW(q2TimePoints<DateTime>(k.get()), K_error);
        }
    }

#   pragma endregion

#   pragma region q2TimeSpan|q2TimeSpans

    TEST_F(TypeConvertTests, q2TimeSpan)
    {
        {
            SCOPED_TRACE("q2TimeSpan on q timespan(s)");

            K_ptr k{ ::ktj(-KN, "10D15:07:01.012345678"_qn) };

            EXPECT_EQ(q2TimeSpan<Nanoseconds>(k.get()),
                date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 }
                    + nanoseconds{ 12'345'678LL });
            EXPECT_EQ(q2TimeSpan<Milliseconds>(k.get()),
                date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 }
                    + milliseconds{ 12 });
            EXPECT_EQ(q2TimeSpan<Seconds>(k.get()),
                date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 });
            EXPECT_EQ(q2TimeSpan<Minutes>(k.get()),
                date::days{ 10 } + hours{ 15 } + minutes{ 7 });

            k.reset(::ktj(-KP, 100));
            EXPECT_THROW(q2TimeSpan<Nanoseconds>(k.get()), K_error);

            k.reset(::kj(100));
            EXPECT_THROW(q2TimeSpan<Nanoseconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpan on q time(s)");

            K_ptr k{ ::kt("15:07:01.012"_qt) };

            EXPECT_EQ(q2TimeSpan<Nanoseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'000'000LL });
            EXPECT_EQ(q2TimeSpan<Milliseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 });
            EXPECT_EQ(q2TimeSpan<Seconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 });
            EXPECT_EQ(q2TimeSpan<Minutes>(k.get()),
                hours{ 15 } + minutes{ 7 });

            k.reset(::ktj(-KP, 100));
            EXPECT_THROW(q2TimeSpan<Milliseconds>(k.get()), K_error);

            k.reset(::ki(100));
            EXPECT_THROW(q2TimeSpan<Milliseconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpan on q second(s)");

            K_ptr k{ TypeTraits<kSecond>::atom("15:07:01"_qv) };

            EXPECT_EQ(q2TimeSpan<Nanoseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 0 });
            EXPECT_EQ(q2TimeSpan<Milliseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 0 });
            EXPECT_EQ(q2TimeSpan<Seconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 1 });
            EXPECT_EQ(q2TimeSpan<Minutes>(k.get()),
                hours{ 15 } + minutes{ 7 });

            k.reset(::ktj(-KP, 100));
            EXPECT_THROW(q2TimeSpan<Seconds>(k.get()), K_error);

            k.reset(::ki(100));
            EXPECT_THROW(q2TimeSpan<Seconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpan on q minute(s)");

            K_ptr k{ TypeTraits<kMinute>::atom("15:07"_qu) };

            EXPECT_EQ(q2TimeSpan<Nanoseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + nanoseconds{ 0 });
            EXPECT_EQ(q2TimeSpan<Milliseconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + milliseconds{ 0 });
            EXPECT_EQ(q2TimeSpan<Seconds>(k.get()),
                hours{ 15 } + minutes{ 7 } + seconds{ 0 });
            EXPECT_EQ(q2TimeSpan<Minutes>(k.get()),
                hours{ 15 } + minutes{ 7 });

            k.reset(::ktj(-KP, 100));
            EXPECT_THROW(q2TimeSpan<Minutes>(k.get()), K_error);

            k.reset(::ki(100));
            EXPECT_THROW(q2TimeSpan<Minutes>(k.get()), K_error);
        }
    }

    TEST_F(TypeConvertTests, q2TimeSpans)
    {
        {
            SCOPED_TRACE("q2TimeSpans on q timespan(s)");

            K_ptr k{ ::ktn(KN, 3) };
            kJ(k.get())[0] = "10D15:07:01.012345678"_qn;
            kJ(k.get())[1] =  "0D00:00:00.000000000"_qn;
            kJ(k.get())[2] = "-1D09:59:59.999999999"_qn;

            EXPECT_EQ(q2TimeSpans<Nanoseconds>(k.get()),
                (vector<Nanoseconds>{
                    date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 }
                        + nanoseconds{ 12'345'678LL },
                    nanoseconds{ 0 },
                    date::days{ -1 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 }
                        - nanoseconds{ 999'999'999LL },
                }));
            EXPECT_EQ(q2TimeSpans<Milliseconds>(k.get()),
                (vector<Milliseconds>{
                    date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 }
                        + milliseconds{ 12 },
                    milliseconds{ 0 },
                    date::days{ -1 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 }
                        - milliseconds{ 999 },
                }));
            EXPECT_EQ(q2TimeSpans<Seconds>(k.get()),
                (vector<Seconds>{
                    date::days{ 10 } + hours{ 15 } + minutes{ 7 } + seconds{ 1 },
                    seconds{ 0 },
                    date::days{ -1 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Minutes>(k.get()),
                (vector<Minutes>{
                    date::days{ 10 } + hours{ 15 } + minutes{ 7 },
                    minutes{ 0 },
                    date::days{ -1 } - hours{ 9 } - minutes{ 59 },
                }));

            k.reset(::ktn(KP, 3));
            EXPECT_THROW(q2TimeSpans<Nanoseconds>(k.get()), K_error);

            k.reset(::ktn(KJ, 3));
            EXPECT_THROW(q2TimeSpans<Nanoseconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpans on q time(s)");

            K_ptr k{ ::ktn(KT, 3) };
            kI(k.get())[0] =  "15:07:01.012"_qt;
            kI(k.get())[1] =  "00:00:00.000"_qt;
            kI(k.get())[2] = "-09:59:59.999"_qt;

            EXPECT_EQ(q2TimeSpans<Nanoseconds>(k.get()),
                (vector<Nanoseconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 } + nanoseconds{ 12'000'000LL },
                    nanoseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 }
                        - nanoseconds{ 999'000'000LL },
                }));
            EXPECT_EQ(q2TimeSpans<Milliseconds>(k.get()),
                (vector<Milliseconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 } + milliseconds{ 12 },
                    milliseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 }
                        - milliseconds{ 999 },
                }));
            EXPECT_EQ(q2TimeSpans<Seconds>(k.get()),
                (vector<Seconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 },
                    seconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Minutes>(k.get()),
                (vector<Minutes>{
                    hours{ 15 } + minutes{ 7 },
                    minutes{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));

            k.reset(::ktn(KP, 3));
            EXPECT_THROW(q2TimeSpans<Milliseconds>(k.get()), K_error);

            k.reset(::ktn(KI, 3));
            EXPECT_THROW(q2TimeSpans<Milliseconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpans on q second(s)");

            K_ptr k{ ::ktn(KV, 3) };
            kI(k.get())[0] =  "15:07:01"_qv;
            kI(k.get())[1] =  "00:00:00"_qv;
            kI(k.get())[2] = "-09:59:59"_qv;

            EXPECT_EQ(q2TimeSpans<Nanoseconds>(k.get()),
                (vector<Nanoseconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 },
                    nanoseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Milliseconds>(k.get()),
                (vector<Milliseconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 },
                    milliseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Seconds>(k.get()),
                (vector<Seconds>{
                    hours{ 15 } + minutes{ 7 } + seconds{ 1 },
                    seconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 } - seconds{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Minutes>(k.get()),
                (vector<Minutes>{
                    hours{ 15 } + minutes{ 7 },
                    minutes{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));

            k.reset(::ktn(KP, 3));
            EXPECT_THROW(q2TimeSpans<Seconds>(k.get()), K_error);

            k.reset(::ktn(KI, 3));
            EXPECT_THROW(q2TimeSpans<Seconds>(k.get()), K_error);
        }
        {
            SCOPED_TRACE("q2TimeSpans on q minute(s)");

            K_ptr k{ ::ktn(KU, 3) };
            kI(k.get())[0] =  "15:07"_qu;
            kI(k.get())[1] =  "00:00"_qu;
            kI(k.get())[2] = "-09:59"_qu;

            EXPECT_EQ(q2TimeSpans<Nanoseconds>(k.get()),
                (vector<Nanoseconds>{
                    hours{ 15 } + minutes{ 7 },
                    nanoseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Milliseconds>(k.get()),
                (vector<Milliseconds>{
                    hours{ 15 } + minutes{ 7 },
                    milliseconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Seconds>(k.get()),
                (vector<Seconds>{
                    hours{ 15 } + minutes{ 7 },
                    seconds{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));
            EXPECT_EQ(q2TimeSpans<Minutes>(k.get()),
                (vector<Minutes>{
                    hours{ 15 } + minutes{ 7 },
                    minutes{ 0 },
                    date::days{ 0 } - hours{ 9 } - minutes{ 59 },
                }));

            k.reset(::ktn(KP, 3));
            EXPECT_THROW(q2TimeSpans<Minutes>(k.get()), K_error);

            k.reset(::ktn(KI, 3));
            EXPECT_THROW(q2TimeSpans<Minutes>(k.get()), K_error);
        }
    }

}//namespace q
