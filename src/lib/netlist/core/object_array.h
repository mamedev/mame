// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file param.h
///

#ifndef NL_CORE_OBJECT_ARRAY_H_
#define NL_CORE_OBJECT_ARRAY_H_

#include "base_objects.h"
#include "logic.h"

#include "../nltypes.h"

#include "../plib/pfmtlog.h"
#include "../plib/plists.h"
#include "../plib/pstring.h"

#include <array>
#include <utility>

namespace netlist
{
	template<class C, std::size_t N>
	class object_array_base_t : public plib::static_vector<C, N>
	{
	public:
		template<class D, typename... Args>
		//object_array_base_t(D &dev, const std::initializer_list<const char *> &names, Args&&... args)
		object_array_base_t(D &dev, std::array<const char *, N> &&names, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, pstring(names[i]), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, const pstring &fmt)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i));
		}

		template<class D, typename... Args>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i+offset), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, nl_delegate delegate)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i+offset), delegate);
		}

		template<class D>
		object_array_base_t(D &dev, std::size_t offset, std::size_t output_mask, const pstring &fmt)
		{
			for (std::size_t i = 0; i<N; i++)
			{
				pstring name(formatted(fmt, i+offset));
				if ((output_mask >> i) & 1)
					name += "Q";
				this->emplace(i, dev, name);
			}
		}
	protected:
		object_array_base_t() = default;

		static pstring formatted(const pstring &fmt, std::size_t n)
		{
			if (N != 1)
				return plib::pfmt(fmt)(n);
			return plib::pfmt(fmt)("");
		}
	};


	template<class C, std::size_t N>
	class object_array_t : public object_array_base_t<C, N>
	{
	public:
		using base_type = object_array_base_t<C, N>;
		using base_type::base_type;
	};

	template<std::size_t N>
	class object_array_t<logic_input_t,N> : public object_array_base_t<logic_input_t, N>
	{
	public:
		using base_type = object_array_base_t<logic_input_t, N>;
		using base_type::base_type;

		template<class D, std::size_t ND>
		object_array_t(D &dev, std::size_t offset, std::size_t output_mask,
			const pstring &fmt, std::array<nl_delegate, ND> &&delegates)
		{
			static_assert(N <= ND, "initializer_list size mismatch");
			std::size_t i = 0;
			for (auto &e : delegates)
			{
				if (i < N)
				{
					pstring name(this->formatted(fmt, i+offset));
					if ((output_mask >> i) & 1)
						name += "Q";
					this->emplace_back(dev, name, e);
				}
				i++;
			}
		}

		//using value_type = typename plib::fast_type_for_bits<N>::type;
		using value_type = std::uint32_t;
		value_type operator ()()
		{
			if (N == 1) return e<0>() ;
			if (N == 2) return e<0>() | (e<1>() << 1);
			if (N == 3) return e<0>() | (e<1>() << 1) | (e<2>() << 2);
			if (N == 4) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3);
			if (N == 5) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4);
			if (N == 6) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5);
			if (N == 7) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5) | (e<6>() << 6);
			if (N == 8) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5) | (e<6>() << 6) | (e<7>() << 7);

			value_type r(0);
			for (std::size_t i = 0; i < N; i++)
				r = static_cast<value_type>((*this)[i]() << (N-1)) | (r >> 1);
			return r;
		}

	private:
		template <std::size_t P>
		constexpr value_type e() const { return (*this)[P](); }
	};

	template<std::size_t N>
	class object_array_t<logic_output_t,N> : public object_array_base_t<logic_output_t, N>
	{
	public:
		using base_type = object_array_base_t<logic_output_t, N>;
		using base_type::base_type;

		template <typename T>
		void push(const T &v, const netlist_time &t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t);
		}

		template<typename T>
		void push(const T &v, const netlist_time * t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t[0]);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t[1]);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t[2]);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t[3]);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t[4]);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t[5]);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t[6]);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t[7]);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t[i]);
		}

		template<typename T, std::size_t NT>
		void push(const T &v, const std::array<const netlist_time, NT> &t)
		{
			static_assert(NT >= N, "Not enough timing entries provided");

			push(v, t.data());
		}

		void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off) noexcept
		{
			for (std::size_t i = 0; i < N; i++)
				(*this)[i].set_tristate(v, ts_off_on, ts_on_off);
		}
	};

	template<std::size_t N>
	class object_array_t<tristate_output_t, N> : public object_array_base_t<tristate_output_t, N>
	{
	public:
		using base_type = object_array_base_t<tristate_output_t, N>;
		using base_type::base_type;

		template <typename T>
		void push(const T &v, const netlist_time &t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t);
		}

		void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off) noexcept
		{
			for (std::size_t i = 0; i < N; i++)
				(*this)[i].set_tristate(v, ts_off_on, ts_on_off);
		}
	};

	// -----------------------------------------------------------------------------
	// Externals
	// -----------------------------------------------------------------------------

	extern template class object_array_t<logic_input_t, 1>;
	extern template class object_array_t<logic_input_t, 2>;
	extern template class object_array_t<logic_input_t, 3>;
	extern template class object_array_t<logic_input_t, 4>;
	extern template class object_array_t<logic_input_t, 5>;
	extern template class object_array_t<logic_input_t, 6>;
	extern template class object_array_t<logic_input_t, 7>;
	extern template class object_array_t<logic_input_t, 8>;

	extern template class object_array_t<logic_output_t, 1>;
	extern template class object_array_t<logic_output_t, 2>;
	extern template class object_array_t<logic_output_t, 3>;
	extern template class object_array_t<logic_output_t, 4>;
	extern template class object_array_t<logic_output_t, 5>;
	extern template class object_array_t<logic_output_t, 6>;
	extern template class object_array_t<logic_output_t, 7>;
	extern template class object_array_t<logic_output_t, 8>;

} // namespace netlist


#endif // NL_CORE_OBJECT_ARRAY_H_
