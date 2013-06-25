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

#include "precompiled.h"
#include <Rocket/Core/Variant.h>

namespace Rocket {
namespace Core {

Variant::Variant() : type(NONE)
{
	// Make sure our object size assumptions fit inside the static buffer
	ROCKET_STATIC_ASSERT(sizeof(Colourb) <= LOCAL_DATA_SIZE, LOCAL_DATA_TOO_SMALL_FOR_Colourb);
	ROCKET_STATIC_ASSERT(sizeof(Colourf) <= LOCAL_DATA_SIZE, LOCAL_DATA_TOO_SMALL_FOR_Colourf);
	ROCKET_STATIC_ASSERT(sizeof(String) <= LOCAL_DATA_SIZE, LOCAL_DATA_TOO_SMALL_FOR_String);
	(void)LOCAL_DATA_TOO_SMALL_FOR_Colourb;
	(void)LOCAL_DATA_TOO_SMALL_FOR_Colourf;
	(void)LOCAL_DATA_TOO_SMALL_FOR_String;	
}

Variant::Variant( const Variant& copy ) : type(NONE)
{
	Set(copy);
}

Variant::~Variant() 
{
	Clear();
}

void Variant::Clear()
{
	// Free any allocated types.
	switch (type) 
	{      
		case STRING:
		{
			// Clean up the string.
			String* string = (String*)data;
			string->~String();
		}
		break;
			
		default:
		break;
	}
	type = NONE;
}

Variant::Type Variant::GetType() const
{
	return type;
}

//////////////////////////////////////////////////
// Set methods
//////////////////////////////////////////////////

#define SET_VARIANT(type) *((type*)data) = value;

void Variant::Set(const Variant& copy)
{
	switch (copy.type)
	{
		case STRING:
		{
			// Create the string
			Set(*(String*)copy.data);
		}
		break;
			
		default:
			Clear();
			memcpy(data, copy.data, LOCAL_DATA_SIZE);
		break;	
	}
	type = copy.type;
}

void Variant::Set(const byte value)
{
	type = BYTE;
	SET_VARIANT(byte);
}

void Variant::Set(const char value)
{
	type = CHAR;
	SET_VARIANT(char);
}

void Variant::Set(const float value)
{
	type = FLOAT;
	SET_VARIANT(float);
}

void Variant::Set(const int value)
{
	type = INT;
	SET_VARIANT(int);
}

void Variant::Set(const String& value) 
{
	if (type == STRING)
	{
		((String*)data)->Assign(value);
	}
	else
	{
		type = STRING;
		new(data) String(value);
	}
}

void Variant::Set(const word value)
{
	type = WORD;
	SET_VARIANT(word);  
}

void Variant::Set(const char* value) 
{
	Set(String(value));
}

void Variant::Set(void* voidptr) 
{
	type = VOIDPTR;
	memcpy(data, &voidptr, sizeof(void*));
}

void Variant::Set(const Vector2f& value)
{
	type = VECTOR2;
	SET_VARIANT(Vector2f);
}

void Variant::Set(const Colourf& value)
{
	type = COLOURF;
	SET_VARIANT(Colourf);
}

void Variant::Set(const Colourb& value)
{
	type = COLOURB;
	SET_VARIANT(Colourb);
}

void Variant::Set(ScriptInterface* value) 
{
	type = SCRIPTINTERFACE;
	memcpy(data, &value, sizeof(ScriptInterface*));
}

Variant& Variant::operator=(const Variant& copy)
{
	Set(copy);
	return *this;
}

}
}
