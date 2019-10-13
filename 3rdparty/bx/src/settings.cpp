/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/settings.h>

namespace
{
#define INI_MALLOC(_ctx, _size)        (BX_ALLOC(reinterpret_cast<bx::AllocatorI*>(_ctx), _size) )
#define INI_FREE(_ctx, _ptr)           (BX_FREE(reinterpret_cast<bx::AllocatorI*>(_ctx), _ptr) )
#define INI_MEMCPY(_dst, _src, _count) (bx::memCopy(_dst, _src, _count) )
#define INI_STRLEN(_str)               (bx::strLen(_str) )
#define INI_STRNICMP(_s1, _s2, _len)   (bx::strCmpI(_s1, _s2, _len) )

#define INI_IMPLEMENTATION

BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function");

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wsign-compare");
#include <ini/ini.h>
BX_PRAGMA_DIAGNOSTIC_POP();
}

namespace bx
{

Settings::Settings(AllocatorI* _allocator, const void* _data, uint32_t _len)
	: m_allocator(_allocator)
	, m_ini(NULL)
{
	load(_data, _len);
}

#define INI_T(_ptr) reinterpret_cast<ini_t*>(_ptr)

Settings::~Settings()
{
	ini_destroy(INI_T(m_ini) );
}

void Settings::clear()
{
	load(NULL, 0);
}

void Settings::load(const void* _data, uint32_t _len)
{
	if (NULL != m_ini)
	{
		ini_destroy(INI_T(m_ini) );
	}

	if (NULL == _data)
	{
		m_ini = ini_create(m_allocator);
	}
	else
	{
		m_ini = ini_load( (const char*)_data, _len, m_allocator);
	}
}

StringView Settings::get(const StringView& _name) const
{
	ini_t* ini = INI_T(m_ini);

	FilePath uri(_name);
	const StringView  path(strTrim(uri.getPath(), "/") );
	const StringView& fileName(uri.getFileName() );
	int32_t section = INI_GLOBAL_SECTION;

	if (!path.isEmpty() )
	{
		section = ini_find_section(ini, path.getPtr(), path.getLength() );
		if (INI_NOT_FOUND == section)
		{
			section = INI_GLOBAL_SECTION;
		}
	}

	int32_t property = ini_find_property(ini, section, fileName.getPtr(), fileName.getLength() );
	if (INI_NOT_FOUND == property)
	{
		return StringView();
	}

	return ini_property_value(ini, section, property);
}

void Settings::set(const StringView& _name, const StringView& _value)
{
	ini_t* ini = INI_T(m_ini);

	FilePath uri(_name);
	const StringView  path(strTrim(uri.getPath(), "/") );
	const StringView& fileName(uri.getFileName() );

	int32_t section = INI_GLOBAL_SECTION;

	if (!path.isEmpty() )
	{
		section = ini_find_section(ini, path.getPtr(), path.getLength() );
		if (INI_NOT_FOUND == section)
		{
			section = ini_section_add(ini, path.getPtr(), path.getLength() );
		}
	}

	int32_t property = ini_find_property(ini, section, fileName.getPtr(), fileName.getLength() );
	if (INI_NOT_FOUND == property)
	{
		ini_property_add(
			  ini
			, section
			, fileName.getPtr()
			, fileName.getLength()
			, _value.getPtr()
			, _value.getLength()
			);
	}
	else
	{
		ini_property_value_set(
			  ini
			, section
			, property
			, _value.getPtr()
			, _value.getLength()
			);
	}
}

void Settings::remove(const StringView& _name) const
{
	ini_t* ini = INI_T(m_ini);

	FilePath uri(_name);
	const StringView  path     = strTrim(uri.getPath(), "/");
	const StringView& fileName = uri.getFileName();

	int32_t section = INI_GLOBAL_SECTION;

	if (!path.isEmpty() )
	{
		section = ini_find_section(ini, path.getPtr(), path.getLength() );
		if (INI_NOT_FOUND == section)
		{
			section = INI_GLOBAL_SECTION;
		}
	}

	int32_t property = ini_find_property(ini, section, fileName.getPtr(), fileName.getLength() );
	if (INI_NOT_FOUND == property)
	{
		return;
	}

	ini_property_remove(ini, section, property);

	if (INI_GLOBAL_SECTION != section
	&&  0 == ini_property_count(ini, section) )
	{
		ini_section_remove(ini, section);
	}
}

int32_t Settings::read(ReaderSeekerI* _reader, Error* _err)
{
	int32_t size = int32_t(getRemain(_reader) );

	void* data = BX_ALLOC(m_allocator, size);

	int32_t total = bx::read(_reader, data, size, _err);
	load(data, size);

	BX_FREE(m_allocator, data);

	return total;
}

int32_t Settings::write(WriterI* _writer, Error* _err) const
{
	ini_t* ini = INI_T(m_ini);

	int32_t size = ini_save(ini, NULL, 0);
	void* data = BX_ALLOC(m_allocator, size);

	ini_save(ini, (char*)data, size);
	int32_t total = bx::write(_writer, data, size-1, _err);

	BX_FREE(m_allocator, data);

	return total;
}

#undef INI_T

int32_t read(ReaderSeekerI* _reader, Settings& _settings, Error* _err)
{
	BX_ERROR_SCOPE(_err);
	return _settings.read(_reader, _err);
}

int32_t write(WriterI* _writer, const Settings& _settings, Error* _err)
{
	BX_ERROR_SCOPE(_err);
	return _settings.write(_writer, _err);
}

} // namespace bx
