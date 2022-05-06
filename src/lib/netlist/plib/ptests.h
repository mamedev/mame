// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PTESTS_H_
#define PTESTS_H_

///
/// \file ptests.h
///
/// google tests compatible (hopefully) test macros. This is work in progress!
///

#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#define PEXPECT_EQ(exp1, exp2) PINT_EXPECT(eq, exp1, exp2)
#define PEXPECT_NE(exp1, exp2) PINT_EXPECT(ne, exp1, exp2)
#define PEXPECT_GT(exp1, exp2) PINT_EXPECT(gt, exp1, exp2)
#define PEXPECT_LT(exp1, exp2) PINT_EXPECT(lt, exp1, exp2)
#define PEXPECT_GE(exp1, exp2) PINT_EXPECT(ge, exp1, exp2)
#define PEXPECT_LE(exp1, exp2) PINT_EXPECT(le, exp1, exp2)

#define PEXPECT_TRUE(exp1) PINT_EXPECT(eq, exp1, true)
#define PEXPECT_FALSE(exp1) PINT_EXPECT(eq, exp1, false)

#define PEXPECT_THROW(exp, excep) PINT_EXPECT_THROW(exp, excep)
#define PEXPECT_NO_THROW(exp) PINT_EXPECT_NO_THROW(exp)

#define PTEST(name, desc) PINT_TEST(name, desc)
#define PTEST_F(name, desc) PINT_TEST_F(name, desc, name)
#define PRUN_ALL_TESTS() plib::testing::run_all_tests()

#define PINT_TEST(name, desc) PINT_TEST_F(name, desc, plib::testing::Test)

#define PINT_TEST_F(name, desc, base) \
	class name ## _ ## desc : public base \
	{ public:\
		void desc (); \
		void run() override { desc (); } \
	}; \
	extern const plib::testing::reg_entry<name ## _ ## desc> name ## _ ## desc ## _reg; \
	const plib::testing::reg_entry<name ## _ ## desc> name ## _ ## desc ## _reg(#name, #desc); \
	void name ## _ ## desc :: desc ()

#define PINT_EXPECT(comp, exp1, exp2) \
	if (!plib::testing::internal_assert(plib::testing::comp_ ## comp (), # exp1, # exp2, exp1, exp2)) \
		std::cout << __FILE__ << ":" << __LINE__ << ":1: error: test failed\n"

#define PINT_EXPECT_THROW(exp, excep) \
	if (const char *ptest_f = __FILE__) \
	{ \
		try { exp; std::cout << ptest_f << ":" << __LINE__ << ":1: error: no " #excep " exception thrown\n";} \
		catch (excep &) { std::cout << "\tOK: got " #excep " for " # exp "\n";} \
		catch (std::exception &ptest_e) { std::cout << ptest_f << ":" << __LINE__ << ":1: error: unexpected exception thrown: " << ptest_e.what() << "\n"; } \
		catch (...) { std::cout << ptest_f << ":" << __LINE__ << ":1: error: unexpected exception thrown\n"; } \
	} else do {} while (0)

#define PINT_EXPECT_NO_THROW(exp) \
	if (const char *ptest_f = __FILE__) \
	{ \
		try { exp; std::cout << "\tOK: got no exception for " # exp "\n";} \
		catch (std::exception &ptest_e) { std::cout << ptest_f << ":" << __LINE__ << ":1: error: unexpected exception thrown: " << ptest_e.what() << "\n"; } \
		catch (...) { std::cout << ptest_f << ":" << __LINE__ << ":1: error: unexpected exception thrown\n"; } \
	} else do {} while (0)

namespace plib::testing
{

	class Test
	{
	public:
		Test() = default;
		virtual ~Test() = default;

		Test(const Test &) = delete;
		Test &operator=(const Test &) = delete;
		Test(Test &&) = delete;
		Test &operator=(Test &&) = delete;

		virtual void run() {}
		virtual void SetUp() {}
		virtual void TearDown() {}
	};

	struct reg_entry_base
	{
		using list_t = std::vector<reg_entry_base *>;
		reg_entry_base(const std::string &n, const std::string &d)
		: name(n), desc(d)
		{
			registry().push_back(this);
		}

		reg_entry_base(const reg_entry_base &) = delete;
		reg_entry_base &operator=(const reg_entry_base &) = delete;
		reg_entry_base(reg_entry_base &&) = delete;
		reg_entry_base &operator=(reg_entry_base &&) = delete;

		virtual ~reg_entry_base() = default;

		virtual Test *create() const { return nullptr; }

		std::string name;
		std::string desc;
	public:
		static list_t & registry()
		{
			static list_t prlist;
			return prlist;
		}
	};

	template <typename T>
	struct reg_entry : public reg_entry_base
	{
		using reg_entry_base::reg_entry_base;

		Test *create() const override { return new T(); } // NOLINT
	};

	template <typename C, typename T1, typename T2>
	bool internal_assert(C comp,
		const char* exp1, const char* exp2,
		const T1& val1, const T2& val2)
	{
		if (comp(val1, val2))
		{
			std::cout << "\tOK: " << exp1 << " " << C::opstr() << " " << exp2 << "\n";
			return true;
		}
		std::cout << "\tFAIL: " << exp1 << " " << C::opstr() << " " << exp2
			<< " <" << val1 << ">,<" << val2 << ">\n";
		return false;
	}

	static inline int run_all_tests()
	{
		std::cout << "======================================\n";
		std::cout << "Running " << reg_entry_base::registry().size() << " tests\n";
		std::cout << "======================================\n";
		for (auto &e : reg_entry_base::registry())
		{
			std::cout << e->name << "::" << e->desc << ":\n";
			Test *t = e->create();
			t->SetUp();
			t->run();
			t->TearDown();
			delete t;
		}
		return 0;
	}

#define DEF_COMP(name, op) \
	struct comp_ ## name \
	{ \
		static const char * opstr() { return #op ; } \
		template <typename T1, typename T2> \
		bool operator()(const T1 &v1, const T2 &v2) { return v1 op v2; } \
	}; \

	DEF_COMP(eq, ==)
	DEF_COMP(ne, !=)
	DEF_COMP(gt, >)
	DEF_COMP(lt, <)
	DEF_COMP(ge, >=)
	DEF_COMP(le, <=)

#undef DEF_COMP

} // namespace plib::testing


#endif // PTESTS_H_
