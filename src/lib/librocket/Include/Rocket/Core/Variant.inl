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

// Constructs a variant with internal data.
template< typename T >
Variant::Variant(const T& t) : type(NONE)
{
	Set( t );
}

// Templatised data accessor.
template< typename T >
bool Variant::GetInto(T& value) const
{	
	switch (type)
	{
		case BYTE:
			return TypeConverter< byte, T >::Convert(*(byte*)data, value);
		break;

		case CHAR:
			return TypeConverter< char, T >::Convert(*(char*)data, value);
		break;

		case FLOAT:
			return TypeConverter< float, T >::Convert(*(float*)data, value);
		break;

		case INT:
			return TypeConverter< int, T >::Convert(*(int*)data, value);
		break;

		case STRING:
			return TypeConverter< String, T >::Convert(*(String*)data, value);
		break;

		case WORD:
			return TypeConverter< word, T >::Convert(*(word*)data, value);
		break;

		case VECTOR2:
			return TypeConverter< Vector2f, T >::Convert(*(Vector2f*)data, value);
		break;

		case COLOURF:
			return TypeConverter< Colourf, T >::Convert(*(Colourf*)data, value);
		break;

		case COLOURB:
			return TypeConverter< Colourb, T >::Convert(*(Colourb*)data, value);
		break;

		case SCRIPTINTERFACE:
			return TypeConverter< ScriptInterface*, T >::Convert((ScriptInterface*)data, value);			
		break;

		case VOIDPTR:
			return TypeConverter< void*, T >::Convert((void*)data, value);
		break;
		
		default:
			return false;
	}

	return false;
}

// Templatised data accessor.
template< typename T >
T Variant::Get() const
{
#pragma GCC diagnostic ignored "-Wuninitialized"	
	T value;
	GetInto(value);
	return value;
}
