// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_roms.cpp
 *
 */

#include "netlist/nl_base.h"
#include "netlist/nl_factory.h"

template <typename N, typename T>
constexpr bool TOR(N n, T &a)
{
	return (n == 0 ? false : TOR(n-1, a) || a[n-1]());
}

template <typename T>
constexpr bool TOR(T &a)
{
	return TOR(a.size(), a);
}

namespace netlist
{
	namespace devices
	{

		template <typename D>
		NETLIB_OBJECT(generic_prom)
		{
			NETLIB_CONSTRUCTOR(generic_prom)
			, m_enabled(*this, "m_enabled", true)
			, m_TE(*this, "FORCE_TRISTATE_LOGIC", 0)
			, m_A(*this, 0, "A{}", NETLIB_DELEGATE(addr))
			, m_CEQ(*this, 1,
				D::chip_enable_mask::value ^ static_cast<size_t>(0xffff), pstring("CE{}"),
				std::array<nldelegate, 3>{ NETLIB_DELEGATE(ce<0>),
				  NETLIB_DELEGATE(ce<1>),
				  NETLIB_DELEGATE(ce<2>)})
			, m_O(*this, D::data_name_offset::value, "O{}", m_TE())
			, m_ROM(*this, "ROM")
			, m_power_pins(*this)
			{
			}

			using data_type = typename plib::least_type_for_bits<D::data_width::value>::type;

		private:

			template <std::size_t N>
			inline NETLIB_HANDLERI(ce)
			{
				using cet = typename D::chip_enable_time;
				m_enabled = (m_CEQ() == D::chip_enable_mask::value);
				switch (D::output_id::value)
				{
					case 0: // logic
						{
							m_O.push(m_ROM[m_A()], D::access_time::value());
						}
						break;
					case 1: // tristate
						{
							m_O.set_tristate(!m_enabled, cet::value(N), cet::value(N));
							m_O.push(m_ROM[m_A()], D::access_time::value());
						}
						break;
					default: // 2, open collector
						{
							const auto delay = m_enabled ? D::access_time::value() : cet::value(N);
							const data_type o = m_enabled ? m_ROM[m_A()] :
								(1 << D::data_width::value) - 1;
							m_O.push(o, delay);
						}
						break;
				}

			}

			inline
			NETLIB_HANDLERI(addr)
			{
				if (m_enabled)
				{
					m_O.push(m_ROM[m_A()], D::access_time::value());
				}
			}

			state_var<bool> m_enabled;
			param_logic_t m_TE;
			object_array_t<logic_input_t, D::address_width::value> m_A;
			object_array_t<logic_input_t, D::chip_enable_inputs::value> m_CEQ;
			object_array_t<typename D::output_type, D::data_width::value> m_O;

			param_rom_t<uint8_t, D::address_width::value, D::data_width::value> m_ROM;
			nld_power_pins m_power_pins;
		};

		struct desc_82S126 : public desc_base
		{
			using address_width =      desc_const<8>;
			using data_width =         desc_const<4>;
			using data_name_offset =   desc_const<1>; // O1, O2, ..
			using chip_enable_inputs = desc_const<2>;
			// MATCH_MASK : all 0 ==> all bits inverted
			using chip_enable_mask =   desc_const<0x00>;

			using chip_enable_time =   times_ns2<25, 25>;
			using access_time =        time_ns<40>;

			using output_type =        tristate_output_t;
			using output_id   =        desc_const<1>; // 0: logic, 1: tristate, 2: open collector
		};

		struct desc_74S287 : public desc_82S126
		{
			using data_name_offset =   desc_const<0>; // O0, O1, ... according to National Semiconductor datasheet
			using chip_enable_time =   times_ns2<15, 15>;
			using access_time =        time_ns<35>;
		};

		struct desc_82S123 : public desc_base
		{
			// FIXME: tristate outputs, add 82S23 (open collector)
			using address_width =      desc_const<5>;
			using data_width =         desc_const<8>;
			using data_name_offset =   desc_const<1>; // O1, O2, ..
			using chip_enable_inputs = desc_const<1>;
			// MATCH_MASK : all 0 ==> all bits inverted
			using chip_enable_mask =   desc_const<0x00>;

			using chip_enable_time =   times_ns1<35>;
			using access_time =        time_ns<45>;

			using output_type =        tristate_output_t;
			using output_id   =        desc_const<1>; // 0: logic, 1: tristate, 2: open collector
		};

		struct desc_2716 : public desc_base
		{
			// FIXME: tristate outputs
			using address_width =      desc_const<11>;
			using data_width =         desc_const<8>;
			using data_name_offset =   desc_const<0>; // O0, O1, ..

			using chip_enable_inputs = desc_const<2>;
			// MATCH_MASK : all 0 ==> all bits inverted
			using chip_enable_mask =   desc_const<0x00>;

			using chip_enable_time = times_ns2<450, 100>; //CE, OE
			using access_time =        time_ns<450>;

			using output_type =        tristate_output_t;
			using output_id   =        desc_const<1>; // 0: logic, 1: tristate, 2: open collector
		};


	using NETLIB_NAME(82S123) = NETLIB_NAME(generic_prom)<desc_82S123>; // 256 bits, 32x8, used as 256x4
	using NETLIB_NAME(82S126) = NETLIB_NAME(generic_prom)<desc_82S126>; // 1024 bits, 32x32, used as 256x4
	using NETLIB_NAME(74S287) = NETLIB_NAME(generic_prom)<desc_74S287>; // 1024 bits, 32x32, used as 256x4
	using NETLIB_NAME(2716)   = NETLIB_NAME(generic_prom)<desc_2716>;   // CE2Q = OE, CE1Q = CE

	NETLIB_DEVICE_IMPL(82S126,     "PROM_82S126",     "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74S287,     "PROM_74S287",     "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,@VCC,@GND")
	NETLIB_DEVICE_IMPL(82S123,     "PROM_82S123",     "+CEQ,+A0,+A1,+A2,+A3,+A4,@VCC,@GND")
	NETLIB_DEVICE_IMPL(2716,       "EPROM_2716",      "+CE2Q,+CE1Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10,@VCC,@GND")

	} //namespace devices
} // namespace netlist
