// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PTESTS_H_
#define PTESTS_H_

///
/// \file ptests.h
///
/// google tests compatible (hopefully) test macros. This is work in progress!
///

#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
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
#define PRUN_ALL_TESTS(loglevel) plib::testing::run_all_tests(loglevel)

#define PINT_TEST(name, desc) PINT_TEST_F(name, desc, plib::testing::Test)

#define PINT_TESTNAME(name, desc) name ## _ ## desc
#define PINT_LOCATION() ::plib::testing::test_location(__FILE__, __LINE__)
#define PINT_SET_LAST(loc) this->m_parameters->m_last_source = loc

#define PINT_REGISTER(name, desc) \
	extern const plib::testing::reg_entry<PINT_TESTNAME(name, desc)> PINT_TESTNAME(name, desc  ## _reg); \
	const plib::testing::reg_entry<PINT_TESTNAME(name, desc)> PINT_TESTNAME(name, desc  ## _reg)(#name, #desc, PINT_LOCATION()); \

#define PINT_TEST_F(name, desc, base) \
	class PINT_TESTNAME(name, desc) : public base \
	{ public:\
		void desc (); \
		void run() override { desc (); } \
	}; \
	PINT_REGISTER(name, desc) \
	void PINT_TESTNAME(name, desc) :: desc ()

#define PINT_EXPECT(comp, exp1, exp2) \
	if (true) \
	{ \
		::plib::testing::test_location source = PINT_LOCATION(); PINT_SET_LAST(source); \
		m_parameters->m_num_tests++; \
		if (!this->internal_assert(plib::testing::comp_ ## comp (), # exp1, # exp2, exp1, exp2)) \
			this->test_error(source) << "test failed" << std::endl; \
	} else do {} while (0)

#define PINT_EXPECT_THROW(exp, excep) \
	if (true) \
	{ \
		::plib::testing::test_location source = PINT_LOCATION(); PINT_SET_LAST(source); \
		m_parameters->m_num_tests++; \
		try { exp; this->test_error(source) << "no " #excep " exception thrown" << std::endl;} \
		catch (excep &) { this->test_ok() << "got " #excep " for " # exp "" << std::endl;} \
		catch (std::exception &ptest_e) { this->test_error(source) << "unexpected exception thrown: " << ptest_e.what() << std::endl; } \
		catch (...) { this->test_error(source) << "unexpected exception thrown" << std::endl; } \
	} else do {} while (0)

#define PINT_EXPECT_NO_THROW(exp) \
	if (true) \
	{ \
		::plib::testing::test_location source = PINT_LOCATION(); PINT_SET_LAST(source); \
		m_parameters->m_num_tests++; \
		try { exp; this->test_ok() << "got no exception for " # exp << std::endl;} \
		catch (std::exception &test_exception) { this->test_error(source) << "unexpected exception thrown: " << test_exception.what() << std::endl; } \
		catch (...) { this->test_error(source) << "unexpected exception thrown" << std::endl; } \
	} else do {} while (0)

namespace plib::testing
{
	enum class loglevel
	{
		INFO,
		WARNING,
		ERROR
	};

	using test_location = std::pair<const char *, std::size_t>;

	struct test_parameters
	{
		loglevel m_loglevel = loglevel::INFO;
		std::size_t m_num_errors = 0;
		std::size_t m_num_tests = 0;
		test_location m_last_source = {"", 0 };
	};

	static std::ostream &stream_error(std::ostream &os, test_location loc)
	{
		return os << loc.first << ":" << loc.second << ":1: error: ";
	}

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

		void set_parameters(test_parameters *params)
		{
			m_parameters = params;
		}
	protected:
		std::ostream & test_ok() { return output(loglevel::INFO) << "\tOK: "; }
		std::ostream & test_fail() { return output(loglevel::WARNING) << "\tFAIL: "; }
		std::ostream & test_error(const test_location & loc)
		{
			return stream_error(output(loglevel::ERROR), loc);
		}

		template <typename C, typename T1, typename T2>
		bool internal_assert(C comp,
			const char* exp1, const char* exp2,
			const T1& val1, const T2& val2);
		test_parameters *m_parameters = nullptr;
	private:
		std::ostream &output(loglevel ll)
		{
			if (ll == loglevel::ERROR)
				m_parameters->m_num_errors++;
			return (ll >= m_parameters->m_loglevel) ? std::cout : m_nulstream;
		}
		std::ostringstream m_nulstream;
	};

	template <typename C, typename T1, typename T2>
	bool Test::internal_assert(C comp,
		const char* exp1, const char* exp2,
		const T1& val1, const T2& val2)
	{
		if (comp(val1, val2))
		{
			test_ok() << exp1 << " " << C::opstr() << " " << exp2 << std::endl;
			return true;
		}
		test_fail() << exp1 << " " << C::opstr() << " " << exp2
			<< " <" << val1 << ">,<" << val2 << ">" << std::endl;
		return false;
	}

	struct reg_entry_base
	{
		using list_t = std::vector<reg_entry_base *>;
		reg_entry_base(const char *n, const char *d, test_location l)
		: name(n), desc(d), location(l)
		{
			registry().push_back(this);
		}

		reg_entry_base(const reg_entry_base &) = delete;
		reg_entry_base &operator=(const reg_entry_base &) = delete;
		reg_entry_base(reg_entry_base &&) = delete;
		reg_entry_base &operator=(reg_entry_base &&) = delete;

		virtual ~reg_entry_base() = default;

		virtual Test *create() const { return nullptr; }

		const char *name;
		const char *desc;
		test_location location;
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

	template <typename L>
	std::pair<bool, std::string> catch_exception(L lambda)
	{
		try {
			lambda();
		}
		catch (std::exception &ptest_e)
		{
			return { true, ptest_e.what() };
		}
		catch (...)
		{
			return { true, "" };
		}
		return { false, "" };
	}

	static inline int run_all_tests(loglevel ll)
	{
		std::cout << "================================================" << std::endl;
		std::cout << "Running " << reg_entry_base::registry().size() << " test groups" << std::endl;
		std::cout << "================================================" << std::endl;

		auto &list = reg_entry_base::registry();

		std::sort(list.begin(), list.end(), [](reg_entry_base *a, reg_entry_base *b) {
			return (a->name < b->name);
		});

		std::size_t total_errors(0);
		std::size_t total_tests(0);

		for (auto &e : list)
		{
			test_parameters params;
			params.m_loglevel = ll;
			params.m_last_source = e->location;

			std::cout << e->name << "::" << e->desc << ":" << std::endl;
			Test *t = nullptr;

			std::pair<bool, std::string> r;
			if ((r = catch_exception([&]{
				t = e->create();
				t->set_parameters(&params);
			})).first)
			{
				stream_error(std::cout, e->location) << "unexpected exception thrown during instantiation" << (r.second != "" ? ": " + r.second : "") << std::endl;
				total_errors++;
			}
			else if ((r = catch_exception([&]{ t->SetUp(); })).first)
			{
				stream_error(std::cout, e->location) << "unexpected exception thrown during Setup" << (r.second != "" ? ": " + r.second : "") << std::endl;
				total_errors++;
			}
			else if ((r = catch_exception([&]{ t->run(); })).first)
			{
				stream_error(std::cout, params.m_last_source) << "unexpected exception thrown during run after this line" << (r.second != "" ? ": " + r.second : "") << std::endl;
				total_errors++;
			}
			else if ((r = catch_exception([&]{ t->TearDown(); })).first)
			{
				stream_error(std::cout, e->location) << "unexpected exception thrown during Teardown" << (r.second != "" ? ": " + r.second : "") << std::endl;
				total_errors++;
			}

			total_errors += params.m_num_errors;
			total_tests += params.m_num_tests;
			if (t != nullptr)
				delete t;
		}
		std::cout << "================================================" << std::endl;
		std::cout << "Found " << total_errors << " errors in " << total_tests << " tests from " << reg_entry_base::registry().size() << " test groups" << std::endl;
		std::cout << "================================================" << std::endl;
		return (total_errors ? 1 : 0);
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
