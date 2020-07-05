// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PTESTS_H_
#define PTESTS_H_

///
/// \file ptests.h
///
/// google tests compatible (hopefully) test macros. This is work in progress!
///

#include <string>
#include <vector>
#include <iostream>
#include <functional>

#define EXPECT_EQ(exp1, exp2) PINT_EXPECT(eq, exp1, exp2)
#define EXPECT_NE(exp1, exp2) PINT_EXPECT(ne, exp1, exp2)
#define EXPECT_GT(exp1, exp2) PINT_EXPECT(gt, exp1, exp2)
#define EXPECT_LT(exp1, exp2) PINT_EXPECT(lt, exp1, exp2)
#define EXPECT_GE(exp1, exp2) PINT_EXPECT(ge, exp1, exp2)
#define EXPECT_LE(exp1, exp2) PINT_EXPECT(le, exp1, exp2)

#define EXPECT_TRUE(exp1) PINT_EXPECT(eq, exp1, true)
#define EXPECT_FALSE(exp1) PINT_EXPECT(eq, exp1, false)

#define TEST(name, desc) PINT_TEST(name, desc)
#define TEST_F(name, desc) PINT_TEST_F(name, desc, name)
#define RUN_ALL_TESTS() plib::testing::run_all_tests()

#define PINT_TEST(name, desc) PINT_TEST_F(name, desc, plib::testing::Test)

#define PINT_TEST_F(name, desc, base) \
	class name ## _ ## desc : public base \
	{ public:\
		void desc (); \
		void run() override { desc (); } \
	}; \
	plib::testing::reg_entry<name ## _ ## desc> name ## _ ## desc ## _reg(#name, #desc); \
	void name ## _ ## desc :: desc ()

#define PINT_EXPECT(comp, exp1, exp2) \
	if (!plib::testing::internal_assert(plib::testing::comp_ ## comp (), # exp1, # exp2, exp1, exp2)) \
		std::cout << __FILE__ << ":" << __LINE__ << ":1: error: test failed\n";

namespace plib
{
namespace testing
{

	class Test
	{
	public:
		virtual ~Test() {}
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
		virtual ~reg_entry_base() = default;
		virtual Test *create() { return nullptr; }

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

		virtual Test *create() override { return new T(); }
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

} // namespace testing
} // namespace plib


#endif // PTESTS_H_
