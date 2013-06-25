/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETVARIANT_H
#define ROCKETVARIANT_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/TypeConverter.h>
#include <list>

namespace Rocket {
namespace Core {

/**
	Variant is a container that can store a selection of basic types. The variant will store the
	value in the native form corresponding to the version of Set that was called.

	Get is templated to convert from the stored form to the requested form by using a TypeConverter.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Variant
{
public:
	Variant();
	/// Templatised constructors don't work for the copy constructor, so we have to define it
	/// explicitly.
	Variant(const Variant&);
	/// Constructs a variant with internal data.
	/// @param[in] t Data of a supported type to set on the variant.
	template< typename T >
	Variant(const T& t);

	~Variant();

	/// Type of data stored in the variant.
	enum Type
	{
		NONE = '-',
		BYTE = 'b',
		CHAR = 'c',
		FLOAT = 'f',
		INT = 'i', 
		STRING = 's',
		WORD = 'w',
		VECTOR2 = '2',
		COLOURF = 'g',
		COLOURB = 'h',
		SCRIPTINTERFACE = 'p',
		VOIDPTR = '*',			
	};

	/// Clears the data structure stored by the variant.
	void Clear();

	/// Gets the current internal representation type.
	/// @return The type of data stored in the variant internally.
	Type GetType() const;

	/// Shares another variant's data with this variant.
	/// @param[in] copy Variant to share data.
	void Set(const Variant& copy);
	/// Sets a byte value on this variant.
	/// @param[in] value New value to set.
	void Set(const byte value);
	/// Sets a signed char value on this variant.
	/// @param[in] value New value to set.
	void Set(const char value);
	/// Sets a float value on this variant.
	/// @param[in] value New value to set.
	void Set(const float value);
	/// Sets a signed int value on this variant.
	/// @param[in] value New value to set.
	void Set(const int value);
	/// Sets a word value on this variant.
	/// @param[in] value New value to set.
	void Set(const word value);
	/// Sets a constant C string value on this variant.
	/// @param[in] value New value to set.
	void Set(const char* value);
	/// Sets a generic void* value on this variant.
	/// @param[in] value New value to set.
	void Set(void* value);
	/// Sets an EMP string value on this variant.
	/// @param[in] value New value to set.
	void Set(const String& value);
	/// Sets a Vector2f value on this variant.
	/// @param[in] value New value to set.
	void Set(const Vector2f& value);
	/// Sets a Colourf value on this variant.
	/// @param[in] value New value to set.
	void Set(const Colourf& value);
	/// Sets a Colourb value on this variant.
	/// @param[in] value New value to set.
	void Set(const Colourb& value);
	/// Sets a script object value on this variant.
	/// @param[in] value New value to set.
	void Set(ScriptInterface* value);

	/// Templatised data accessor. TypeConverters will be used to attempt to convert from the
	/// internal representation to the requested representation.
	/// @return Data in the requested type.
	template< typename T >
	T Get() const;

	/// Templatised data accessor. TypeConverters will be used to attempt to convert from the
	/// internal representation to the requested representation.
	/// @param[out] value Data in the requested type.
	/// @return True if the value was converted and returned, false if no data was stored in the variant.
	template< typename T >
	bool GetInto(T& value) const;

	/// Assigns another variant's internal data to this variant.
	/// @param[in] copy Variant to share data.
	Variant& operator=(const Variant& copy);

private:
	
#ifdef ROCKET_ARCH_64
		static const int LOCAL_DATA_SIZE = 32; // Required for Strings
#else
		static const int LOCAL_DATA_SIZE = 24;
#endif
	Type type;
	char data[LOCAL_DATA_SIZE];
};

#include <Rocket/Core/Variant.inl>

}
}

#endif
