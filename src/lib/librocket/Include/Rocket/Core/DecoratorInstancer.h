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

#ifndef ROCKETCOREDECORATORINSTANCER_H
#define ROCKETCOREDECORATORINSTANCER_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/Header.h>
#include <Rocket/Core/PropertyDictionary.h>
#include <Rocket/Core/PropertySpecification.h>

namespace Rocket {
namespace Core {

class Decorator;

/**
	An element instancer provides a method for allocating and deallocating decorators.

	It is important at the same instancer that allocated a decorator releases it. This ensures there are no issues with
	memory from different DLLs getting mixed up.

	@author Peter Curry
 */

class ROCKETCORE_API DecoratorInstancer : public ReferenceCountable
{
public:
	DecoratorInstancer();
	virtual ~DecoratorInstancer();

	/// Instances a decorator given the property tag and attributes from the RCSS file.
	/// @param[in] name The type of decorator desired. For example, "background-decorator: simple;" is declared as type "simple".
	/// @param[in] properties All RCSS properties associated with the decorator.
	/// @return The decorator if it was instanced successfully, NULL if an error occured.
	virtual Decorator* InstanceDecorator(const String& name, const PropertyDictionary& properties) = 0;
	/// Releases the given decorator.
	/// @param[in] decorator Decorator to release. This is guaranteed to have been constructed by this instancer.
	virtual void ReleaseDecorator(Decorator* decorator) = 0;

	/// Releases the instancer.
	virtual void Release() = 0;

	/// Returns the property specification associated with the instancer.
	const PropertySpecification& GetPropertySpecification() const;

protected:
	/// Registers a property for the decorator.
	/// @param[in] property_name The name of the new property (how it is specified through RCSS).
	/// @param[in] default_value The default value to be used.
	/// @return The new property definition, ready to have parsers attached.
	PropertyDefinition& RegisterProperty(const String& property_name, const String& default_value);
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
};

}
}

#endif
