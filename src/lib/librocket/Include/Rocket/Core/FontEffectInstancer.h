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

#ifndef ROCKETCOREFONTEFFECTINSTANCER_H
#define ROCKETCOREFONTEFFECTINSTANCER_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/Header.h>
#include <Rocket/Core/PropertyDictionary.h>
#include <Rocket/Core/PropertySpecification.h>

namespace Rocket {
namespace Core {

class FontEffect;

/**
	A font effect instancer provides a method for allocating and deallocating font effects.

	It is important at the same instancer that allocated a font effect releases it. This ensures there are no issues
	with memory from different DLLs getting mixed up.

	@author Peter Curry
 */

class ROCKETCORE_API FontEffectInstancer : public ReferenceCountable
{
public:
	FontEffectInstancer();
	virtual ~FontEffectInstancer();

	/// Instances a font effect given the property tag and attributes from the RCSS file.
	/// @param[in] name The type of font effect desired. For example, "title-font-effect: outline;" is declared as type "outline".
	/// @param[in] properties All RCSS properties associated with the font effect.
	/// @return The font effect if it was instanced successfully, NULL if an error occured.
	virtual FontEffect* InstanceFontEffect(const String& name, const PropertyDictionary& properties) = 0;
	/// Releases the given font effect.
	/// @param[in] font_effect Font effect to release. This is guaranteed to have been constructed by this instancer.
	virtual void ReleaseFontEffect(FontEffect* font_effect) = 0;

	/// Releases the instancer.
	virtual void Release() = 0;

	/// Returns the property specification associated with the instancer.
	const PropertySpecification& GetPropertySpecification() const;

protected:
	/// Registers a property for the font effect.
	/// @param[in] property_name The name of the new property (how it is specified through RCSS).
	/// @param[in] default_value The default value to be used.
	/// @param[in] affects_generation True if this property affects the effect's texture data or glyph size, false if not.
	/// @return The new property definition, ready to have parsers attached.
	PropertyDefinition& RegisterProperty(const String& property_name, const String& default_value, bool affects_generation = true);
	/// Registers a shorthand property definition.
	/// @param[in] shorthand_name The name to register the new shorthand property under.
	/// @param[in] properties A comma-separated list of the properties this definition is shorthand for. The order in which they are specified here is the order in which the values will be processed.
	/// @param[in] type The type of shorthand to declare.
	/// @param True if all the property names exist, false otherwise.
	bool RegisterShorthand(const String& shorthand_name, const String& property_names, PropertySpecification::ShorthandType type = PropertySpecification::AUTO);

	// Releases the instancer.
	virtual void OnReferenceDeactivate();

private:
	PropertySpecification properties;

	// Properties that define the geometry.
	std::set< String > volatile_properties;

	friend class Factory;
};

}
}

#endif
