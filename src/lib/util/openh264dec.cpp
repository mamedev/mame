// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "openh264dec.h"

#include "modules/lib/osdlib.h"


namespace util::openh264 {

namespace {

class decoder_api_impl : public decoder_api
{
private:
	using create_function = long (*)(ISVCDecoder **);
	using destroy_function = void (*)(ISVCDecoder *);

	osd::dynamic_module::ptr m_lib;
	create_function m_create = nullptr;
	destroy_function m_destroy = nullptr;

public:
	decoder_api_impl()
	{
#if defined(_WIN32) && (defined(__x86_64__) || defined(_M_X64))
		m_lib = osd::dynamic_module::open({ "openh264-2.7.0-win64.dll", "openh264-win64.dll", "openh264.dll" });
#elif defined(_WIN32) && (defined(__aarch64__) || defined(_M_ARM64))
		m_lib = osd::dynamic_module::open({ "openh264-2.7.0-win-arm64.dll", "openh264-win-arm64.dll", "openh264.dll" });
#elif defined(_WIN32)
		m_lib = osd::dynamic_module::open({ "openh264-2.7.0.dll", "openh264.dll" });
#elif defined(__linux__) && defined(__x86_64__)
		m_lib = osd::dynamic_module::open({ "libopenh264-2.7.0-linux64.8.so", "libopenh264-linux64.8.so", "libopenh264.so.8", "libopenh264.so.7", "libopenh264.so.6", "libopenh264-linux64.so", "libopenh264.so" });
#elif defined(__linux__) && defined(__aarch64__)
		m_lib = osd::dynamic_module::open({ "libopenh264-2.7.0-linux-arm64.8.so", "libopenh264-linux-arm64.8.so", "libopenh264.so.8", "libopenh264.so.7", "libopenh264.so.6", "libopenh264-linux-arm64.so", "libopenh264.so" });
#elif defined(__APPLE__) && defined(__x64__)
		m_lib = osd::dynamic_module::open({ "libopenh264-2.7.0-mac-x64.dylib", "libopenh264-mac-x64.dylib", "libopenh264.dylib" });
#elif defined(__APPLE__) && defined(__aarch64__)
		m_lib = osd::dynamic_module::open({ "libopenh264-2.7.0-mac-arm64.dylib", "libopenh264-mac-arm64.dylib", "libopenh264.dylib" });
#endif
		if (m_lib)
		{
			m_create = m_lib->bind<create_function>("WelsCreateDecoder");
			m_destroy = m_lib->bind<destroy_function>("WelsDestroyDecoder");
		}
	}

	virtual long create_decoder(ISVCDecoder **decoder) override
	{
		if (m_create)
			return m_create(decoder);
		else
			return ERR_API_FAILED;
	}

	virtual void destroy_decoder(ISVCDecoder *decoder) override
	{
		if (m_destroy)
			return m_destroy(decoder);
	}

};

} // anonymous namespace

decoder_api::ptr decoder_api::initialize()
{
	return std::make_unique<decoder_api_impl>();
}

} // namespace util::openh264
