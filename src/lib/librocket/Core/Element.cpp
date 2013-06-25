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
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Dictionary.h>
#include <algorithm>
#include "ElementBackground.h"
#include "ElementBorder.h"
#include "ElementDefinition.h"
#include "ElementStyle.h"
#include "EventDispatcher.h"
#include "ElementDecoration.h"
#include "FontFaceHandle.h"
#include "LayoutEngine.h"
#include "PluginRegistry.h"
#include "StyleSheetParser.h"
#include "XMLParseTools.h"
#include <Rocket/Core/Core.h>

namespace Rocket {
namespace Core {

/**
	STL function object for sorting elements by z-type (ie, float-types before general types, etc).
	@author Peter Curry
 */
class ElementSortZOrder
{
public:
	bool operator()(const std::pair< Element*, float >& lhs, const std::pair< Element*, float >& rhs)
	{
		return lhs.second < rhs.second;
	}
};

/**
	STL function object for sorting elements by z-index property.
	@author Peter Curry
 */
class ElementSortZIndex
{
public:
	bool operator()(const Element* lhs, const Element* rhs)
	{
		// Check the z-index.
		return lhs->GetZIndex() < rhs->GetZIndex();
	}
};

/// Constructs a new libRocket element.
Element::Element(const String& _tag) : relative_offset_base(0, 0), relative_offset_position(0, 0), absolute_offset(0, 0), scroll_offset(0, 0),  boxes(1), content_offset(0, 0), content_box(0, 0)
{
	tag = _tag.ToLower();
	parent = NULL;
	focus = NULL;
	instancer = NULL;
	owner_document = NULL;

	offset_fixed = false;
	offset_parent = NULL;
	offset_dirty = true;

	client_area = Box::PADDING;

	num_non_dom_children = 0;

	visible = true;

	z_index = 0;

	local_stacking_context = false;
	local_stacking_context_forced = false;
	stacking_context_dirty = false;

	font_face_handle = NULL;
	
	clipping_ignore_depth = 0;
	clipping_enabled = false;
	clipping_state_dirty = true;

	event_dispatcher = new EventDispatcher(this);
	style = new ElementStyle(this);
	background = new ElementBackground(this);
	border = new ElementBorder(this);
	decoration = new ElementDecoration(this);
	scroll = new ElementScroll(this);
}

Element::~Element()
{
	ROCKET_ASSERT(parent == NULL);	

	PluginRegistry::NotifyElementDestroy(this);

	// Delete the scroll funtionality before we delete the children!
	delete scroll;

	while (!children.empty())
	{
		// A simplified version of RemoveChild() for destruction.
		Element* child = children.front();
		child->OnChildRemove(child);

		if (num_non_dom_children > 0)
			num_non_dom_children--;

		deleted_children.push_back(child);
		children.erase(children.begin());
	}

	// Release all deleted children.
	ReleaseElements(deleted_children);

	delete decoration;
	delete border;
	delete background;
	delete style;
	delete event_dispatcher;

	if (font_face_handle != NULL)
		font_face_handle->RemoveReference();

	if (instancer)
		instancer->RemoveReference();
}

void Element::Update()
{
	ReleaseElements(deleted_children);
	active_children = children;
	for (size_t i = 0; i < active_children.size(); i++)
		active_children[i]->Update();

	// Force a definition reload, if necessary.
	style->GetDefinition();

	scroll->Update();
	OnUpdate();
}

void Element::Render()
{
	// Rebuild our stacking context if necessary.
	if (stacking_context_dirty)
		BuildLocalStackingContext();

	// Render all elements in our local stacking context that have a z-index beneath our local index of 0.
	size_t i = 0;
	for (; i < stacking_context.size() && stacking_context[i]->z_index < 0; ++i)
		stacking_context[i]->Render();

	// Set up the clipping region for this element.
	if (ElementUtilities::SetClippingRegion(this))
	{
		background->RenderBackground();
		border->RenderBorder();
		decoration->RenderDecorators();

		OnRender();
	}

	// Render the rest of the elements in the stacking context.
	for (; i < stacking_context.size(); ++i)
		stacking_context[i]->Render();
}

// Clones this element, returning a new, unparented element.
Element* Element::Clone() const
{
	Element* clone = NULL;

	if (instancer != NULL)
	{
		clone = instancer->InstanceElement(NULL, GetTagName(), attributes);
		if (clone != NULL)
			clone->SetInstancer(instancer);
	}
	else
		clone = Factory::InstanceElement(NULL, GetTagName(), GetTagName(), attributes);

	if (clone != NULL)
	{
		String inner_rml;
		GetInnerRML(inner_rml);

		clone->SetInnerRML(inner_rml);
	}

	return clone;
}

// Sets or removes a class on the element.
void Element::SetClass(const String& class_name, bool activate)
{
	style->SetClass(class_name, activate);
}

// Checks if a class is set on the element.
bool Element::IsClassSet(const String& class_name) const
{
	return style->IsClassSet(class_name);
}

// Specifies the entire list of classes for this element. This will replace any others specified.
void Element::SetClassNames(const String& class_names)
{
	SetAttribute("class", class_names);
}

/// Return the active class list
String Element::GetClassNames() const
{
	return style->GetClassNames();
}

// Returns the active style sheet for this element. This may be NULL.
StyleSheet* Element::GetStyleSheet() const
{
	return style->GetStyleSheet();
}

// Returns the element's definition, updating if necessary.
const ElementDefinition* Element::GetDefinition()
{
	return style->GetDefinition();
}

// Fills an String with the full address of this element.
String Element::GetAddress(bool include_pseudo_classes) const
{
	// Add the tag name onto the address.
	String address(tag);

	// Add the ID if we have one.
	if (!id.Empty())
	{
		address += "#";
		address += id;
	}

	String classes = style->GetClassNames();
	if (!classes.Empty())
	{
		classes = classes.Replace(".", " ");
		address += ".";
		address += classes;
	}

	if (include_pseudo_classes)
	{
		const PseudoClassList& pseudo_classes = style->GetActivePseudoClasses();		
		for (PseudoClassList::const_iterator i = pseudo_classes.begin(); i != pseudo_classes.end(); ++i)
		{
			address += ":";
			address += (*i);
		}
	}

	if (parent)
	{
		address.Append(" < ");
		return address + parent->GetAddress(true);
	}
	else
		return address;
}

// Sets the position of this element, as a two-dimensional offset from another element.
void Element::SetOffset(const Vector2f& offset, Element* _offset_parent, bool _offset_fixed)
{
	_offset_fixed |= GetProperty< int >(POSITION) == POSITION_FIXED;

	// If our offset has definitely changed, or any of our parenting has, then these are set and
	// updated based on our left / right / top / bottom properties.
	if (relative_offset_base != offset ||
		offset_parent != _offset_parent ||
		offset_fixed != _offset_fixed)
	{
		relative_offset_base = offset;
		offset_fixed = _offset_fixed;
		offset_parent = _offset_parent;
		UpdateOffset();
		DirtyOffset();
	}

	// Otherwise, our offset is updated in case left / right / top / bottom will have an impact on
	// our final position, and our children are dirtied if they do.
	else
	{
		Vector2f& old_base = relative_offset_base;
		Vector2f& old_position = relative_offset_position;

		UpdateOffset();

		if (old_base != relative_offset_base ||
			old_position != relative_offset_position)
			DirtyOffset();
	}
}

// Returns the position of the top-left corner of one of the areas of this element's primary box.
Vector2f Element::GetRelativeOffset(Box::Area area)
{
	UpdateLayout();
	return relative_offset_base + relative_offset_position + GetBox().GetPosition(area);
}

// Returns the position of the top-left corner of one of the areas of this element's primary box.
Vector2f Element::GetAbsoluteOffset(Box::Area area)
{
	UpdateLayout();
	if (offset_dirty)
	{
		offset_dirty = false;

		if (offset_parent != NULL)
			absolute_offset = offset_parent->GetAbsoluteOffset(Box::BORDER) + relative_offset_base + relative_offset_position;
		else
			absolute_offset = relative_offset_base + relative_offset_position;

		// Add any parent scrolling onto our position as well. Could cache this if required.
		if (!offset_fixed)
		{
			Element* scroll_parent = parent;
			while (scroll_parent != NULL)
			{
				absolute_offset -= (scroll_parent->scroll_offset + scroll_parent->content_offset);
				if (scroll_parent == offset_parent)
					break;
				else
					scroll_parent = scroll_parent->parent;
			}
		}
	}

	return absolute_offset + GetBox().GetPosition(area);
}

// Sets an alternate area to use as the client area.
void Element::SetClientArea(Box::Area _client_area)
{
	client_area = _client_area;
}

// Returns the area the element uses as its client area.
Box::Area Element::GetClientArea() const
{
	return client_area;
}

// Sets the dimensions of the element's internal content.
void Element::SetContentBox(const Vector2f& _content_offset, const Vector2f& _content_box)
{
	if (content_offset != _content_offset ||
		content_box != _content_box)
	{
		// Seems to be jittering a wee bit; might need to be looked at.
		scroll_offset.x += (content_offset.x - _content_offset.x);
		scroll_offset.y += (content_offset.y - _content_offset.y);

		content_offset = _content_offset;
		content_box = _content_box;

		scroll_offset.x = Math::Min(scroll_offset.x, GetScrollWidth() - GetClientWidth());
		scroll_offset.y = Math::Min(scroll_offset.y, GetScrollHeight() - GetClientHeight());
		DirtyOffset();
	}
}

// Sets the box describing the size of the element.
void Element::SetBox(const Box& box)
{
	if (box != boxes[0] ||
		boxes.size() > 1)
	{
		boxes[0] = box;
		boxes.resize(1);

		background->DirtyBackground();
		border->DirtyBorder();
		decoration->ReloadDecorators();

		DispatchEvent(RESIZE, Dictionary());
	}
}

// Adds a box to the end of the list describing this element's geometry.
void Element::AddBox(const Box& box)
{
	boxes.push_back(box);
	DispatchEvent(RESIZE, Dictionary());

	background->DirtyBackground();
	border->DirtyBorder();
	decoration->ReloadDecorators();
}

// Returns one of the boxes describing the size of the element.
const Box& Element::GetBox(int index)
{
	UpdateLayout();

	if (index < 0)
		return boxes[0];
	else if (index >= GetNumBoxes())
		return boxes.back();

	return boxes[index];
}

// Returns the number of boxes making up this element's geometry.
int Element::GetNumBoxes()
{
	UpdateLayout();
	return (int) boxes.size();
}

// Returns the baseline of the element, in pixels offset from the bottom of the element's content area.
float Element::GetBaseline() const
{
	return 0;
}

// Gets the intrinsic dimensions of this element, if it is of a type that has an inherent size.
bool Element::GetIntrinsicDimensions(Vector2f& ROCKET_UNUSED(dimensions))
{
	return false;
}

// Checks if a given point in screen coordinates lies within the bordered area of this element.
bool Element::IsPointWithinElement(const Vector2f& point)
{
	Vector2f position = GetAbsoluteOffset(Box::BORDER);

	for (int i = 0; i < GetNumBoxes(); ++i)
	{
		const Box& box = GetBox(i);

		Vector2f box_position = position + box.GetOffset();
		Vector2f box_dimensions = box.GetSize(Box::BORDER);
		if (point.x >= box_position.x &&
			point.x <= (box_position.x + box_dimensions.x) &&
			point.y >= box_position.y &&
			point.y <= (box_position.y + box_dimensions.y))
		{
			return true;
		}
	}

	return false;
}

// Returns the visibility of the element.
bool Element::IsVisible() const
{
	return visible;
}

// Returns the z-index of the element.
float Element::GetZIndex() const
{
	return z_index;
}

// Returns the element's font face handle.
FontFaceHandle* Element::GetFontFaceHandle() const
{
	return font_face_handle;
}

// Sets a local property override on the element.
bool Element::SetProperty(const String& name, const String& value)
{
	return style->SetProperty(name, value);
}

// Removes a local property override on the element.
void Element::RemoveProperty(const String& name)
{
	style->RemoveProperty(name);
}

// Sets a local property override on the element to a pre-parsed value.
bool Element::SetProperty(const String& name, const Property& property)
{
	return style->SetProperty(name, property);
}

// Returns one of this element's properties.
const Property* Element::GetProperty(const String& name)
{
	return style->GetProperty(name);	
}

// Returns one of this element's properties.
const Property* Element::GetLocalProperty(const String& name)
{
	return style->GetLocalProperty(name);
}

// Resolves one of this element's style.
float Element::ResolveProperty(const String& name, float base_value)
{
	return style->ResolveProperty(name, base_value);
}

// Iterates over the properties defined on this element.
bool Element::IterateProperties(int& index, PseudoClassList& pseudo_classes, String& name, const Property*& property) const
{
	return style->IterateProperties(index, pseudo_classes, name, property);
}

// Sets or removes a pseudo-class on the element.
void Element::SetPseudoClass(const String& pseudo_class, bool activate)
{
	style->SetPseudoClass(pseudo_class, activate);
}

// Checks if a specific pseudo-class has been set on the element.
bool Element::IsPseudoClassSet(const String& pseudo_class) const
{
	return style->IsPseudoClassSet(pseudo_class);
}

// Checks if a complete set of pseudo-classes are set on the element.
bool Element::ArePseudoClassesSet(const PseudoClassList& pseudo_classes) const
{
	for (PseudoClassList::const_iterator i = pseudo_classes.begin(); i != pseudo_classes.end(); ++i)
	{
		if (!IsPseudoClassSet(*i))
			return false;
	}

	return true;
}

// Gets a list of the current active pseudo classes
const PseudoClassList& Element::GetActivePseudoClasses() const
{
	return style->GetActivePseudoClasses();
}

/// Get the named attribute
Variant* Element::GetAttribute(const String& name) const
{
	return attributes.Get(name);
}

// Checks if the element has a certain attribute.
bool Element::HasAttribute(const String& name)
{
	return attributes.Get(name) != NULL;
}

// Removes an attribute from the element
void Element::RemoveAttribute(const String& name)
{
	if (attributes.Remove(name))
	{
		AttributeNameList changed_attributes;
		changed_attributes.insert(name);

		OnAttributeChange(changed_attributes);
	}
}

// Gets the outer most focus element down the tree from this node
Element* Element::GetFocusLeafNode()
{
	// If there isn't a focus, then we are the leaf.
	if (!focus)
	{
		return this;
	}

	// Recurse down the tree until we found the leaf focus element
	Element* focus_element = focus;
	while (focus_element->focus)
		focus_element = focus_element->focus;

	return focus_element;
}

// Returns the element's context.
Context* Element::GetContext()
{
	ElementDocument* document = GetOwnerDocument();
	if (document != NULL)
		return document->GetContext();

	return NULL;
}

// Set a group of attributes
void Element::SetAttributes(const ElementAttributes* _attributes)
{
	int index = 0;
	String key;
	Variant* value;

	AttributeNameList changed_attributes;

	while (_attributes->Iterate(index, key, value))
	{		
		changed_attributes.insert(key);
		attributes.Set(key, *value);
	}

	OnAttributeChange(changed_attributes);
}

// Returns the number of attributes on the element.
int Element::GetNumAttributes() const
{
	return attributes.Size();
}

// Iterates over all decorators attached to the element.
bool Element::IterateDecorators(int& index, PseudoClassList& pseudo_classes, String& name, Decorator*& decorator, DecoratorDataHandle& decorator_data)
{
	return decoration->IterateDecorators(index, pseudo_classes, name, decorator, decorator_data);
}

// Gets the name of the element.
const String& Element::GetTagName() const
{
	return tag;
}

// Gets the ID of the element.
const String& Element::GetId() const
{
	return id;
}

// Sets the ID of the element.
void Element::SetId(const String& _id)
{
	SetAttribute("id", _id);
}

// Gets the horizontal offset from the context's left edge to element's left border edge.
float Element::GetAbsoluteLeft()
{
	return GetAbsoluteOffset(Box::BORDER).x;
}

// Gets the vertical offset from the context's top edge to element's top border edge.
float Element::GetAbsoluteTop()
{
	return GetAbsoluteOffset(Box::BORDER).y;
}

// Gets the width of the left border of an element.
float Element::GetClientLeft()
{
	UpdateLayout();
	return GetBox().GetPosition(client_area).x;
}

// Gets the height of the top border of an element.
float Element::GetClientTop()
{
	UpdateLayout();
	return GetBox().GetPosition(client_area).y;
}

// Gets the inner width of the element.
float Element::GetClientWidth()
{
	UpdateLayout();
	return GetBox().GetSize(client_area).x - scroll->GetScrollbarSize(ElementScroll::VERTICAL);
}

// Gets the inner height of the element.
float Element::GetClientHeight()
{
	UpdateLayout();
	return GetBox().GetSize(client_area).y - scroll->GetScrollbarSize(ElementScroll::HORIZONTAL);
}

// Returns the element from which all offset calculations are currently computed.
Element* Element::GetOffsetParent()
{
	return parent;
}

// Gets the distance from this element's left border to its offset parent's left border.
float Element::GetOffsetLeft()
{
	UpdateLayout();
	return relative_offset_base.x + relative_offset_position.x;
}

// Gets the distance from this element's top border to its offset parent's top border.
float Element::GetOffsetTop()
{
	UpdateLayout();
	return relative_offset_base.y + relative_offset_position.y;
}

// Gets the width of the element, including the client area, padding, borders and scrollbars, but not margins.
float Element::GetOffsetWidth()
{
	UpdateLayout();
	return GetBox().GetSize(Box::BORDER).x;
}

// Gets the height of the element, including the client area, padding, borders and scrollbars, but not margins.
float Element::GetOffsetHeight()
{
	UpdateLayout();
	return GetBox().GetSize(Box::BORDER).y;
}

// Gets the left scroll offset of the element.
float Element::GetScrollLeft()
{
	UpdateLayout();
	return scroll_offset.x;
}

// Sets the left scroll offset of the element.
void Element::SetScrollLeft(float scroll_left)
{
	scroll_offset.x = LayoutEngine::Round(Math::Clamp(scroll_left, 0.0f, GetScrollWidth() - GetClientWidth()));
	scroll->UpdateScrollbar(ElementScroll::HORIZONTAL);
	DirtyOffset();

	DispatchEvent("scroll", Dictionary());
}

// Gets the top scroll offset of the element.
float Element::GetScrollTop()
{
	UpdateLayout();
	return scroll_offset.y;
}

// Sets the top scroll offset of the element.
void Element::SetScrollTop(float scroll_top)
{
	scroll_offset.y = LayoutEngine::Round(Math::Clamp(scroll_top, 0.0f, GetScrollHeight() - GetClientHeight()));
	scroll->UpdateScrollbar(ElementScroll::VERTICAL);
	DirtyOffset();

	DispatchEvent("scroll", Dictionary());
}

// Gets the width of the scrollable content of the element; it includes the element padding but not its margin.
float Element::GetScrollWidth()
{
	return Math::Max(content_box.x, GetClientWidth());
}

// Gets the height of the scrollable content of the element; it includes the element padding but not its margin.
float Element::GetScrollHeight()
{
	return Math::Max(content_box.y, GetClientHeight());
}

// Gets the object representing the declarations of an element's style attributes.
ElementStyle* Element::GetStyle()
{
	return style;
}

// Gets the document this element belongs to.
ElementDocument* Element::GetOwnerDocument()
{
	if (parent == NULL)
		return NULL;
	
	if (!owner_document)
	{
		owner_document = parent->GetOwnerDocument();
	}

	return owner_document;
}

// Gets this element's parent node.
Element* Element::GetParentNode() const
{
	return parent;
}

// Gets the element immediately following this one in the tree.
Element* Element::GetNextSibling() const
{
	if (parent == NULL)
		return NULL;

	for (size_t i = 0; i < parent->children.size() - (parent->num_non_dom_children + 1); i++)
	{
		if (parent->children[i] == this)
			return parent->children[i + 1];
	}

	return NULL;
}

// Gets the element immediately preceding this one in the tree.
Element* Element::GetPreviousSibling() const
{
	if (parent == NULL)
		return NULL;

	for (size_t i = 1; i < parent->children.size() - parent->num_non_dom_children; i++)
	{
		if (parent->children[i] == this)
			return parent->children[i - 1];
	}

	return NULL;
}

// Returns the first child of this element.
Element* Element::GetFirstChild() const
{
	if (GetNumChildren() > 0)
		return children[0];

	return NULL;
}

// Gets the last child of this element.
Element* Element::GetLastChild() const
{
	if (GetNumChildren() > 0)
		return *(children.end() - (num_non_dom_children + 1));

	return NULL;
}

Element* Element::GetChild(int index) const
{
	if (index < 0 || index >= (int) children.size())
		return NULL;

	return children[index];
}

int Element::GetNumChildren(bool include_non_dom_elements) const
{
	return (int) children.size() - (include_non_dom_elements ? 0 : num_non_dom_children);
}

// Gets the markup and content of the element.
void Element::GetInnerRML(String& content) const
{
	for (int i = 0; i < GetNumChildren(); i++)
	{
		children[i]->GetRML(content);
	}
}

// Gets the markup and content of the element.
String Element::GetInnerRML() const {
	String result;
	GetInnerRML(result);
	return result;
}

// Sets the markup and content of the element. All existing children will be replaced.
void Element::SetInnerRML(const String& rml)
{
	// Remove all DOM children.
	while ((int) children.size() > num_non_dom_children)
		RemoveChild(children.front());

	Factory::InstanceElementText(this, rml);
}

// Sets the current element as the focus object.
bool Element::Focus()
{
	// Are we allowed focus?
	int focus_property = GetProperty< int >(FOCUS);
	if (focus_property == FOCUS_NONE)
		return false;

	// Ask our context if we can switch focus.
	Context* context = GetContext();
	if (context == NULL)
		return false;

	if (!context->OnFocusChange(this))
		return false;

	// Set this as the end of the focus chain.
	focus = NULL;

	// Update the focus chain up the hierarchy.
	Element* element = this;
	while (element->GetParentNode())
	{
		element->GetParentNode()->focus = element;
		element = element->GetParentNode();
	}

	return true;
}

// Removes focus from from this element.
void Element::Blur()
{
	if (parent)
	{
		Context* context = GetContext();
		if (context == NULL)
			return;

		if (context->GetFocusElement() == this)
		{
			parent->Focus();
		}
		else if (parent->focus == this)
		{
			parent->focus = NULL;
		}
	}
}

// Fakes a mouse click on this element.
void Element::Click()
{
	Context* context = GetContext();
	if (context == NULL)
		return;

	context->GenerateClickEvent(this);
}

// Adds an event listener
void Element::AddEventListener(const String& event, EventListener* listener, bool in_capture_phase)
{
	event_dispatcher->AttachEvent(event, listener, in_capture_phase);
}

// Removes an event listener from this element.
void Element::RemoveEventListener(const String& event, EventListener* listener, bool in_capture_phase)
{
	event_dispatcher->DetachEvent(event, listener, in_capture_phase);
}

// Dispatches the specified event
bool Element::DispatchEvent(const String& event, const Dictionary& parameters, bool interruptible)
{
	return event_dispatcher->DispatchEvent(this, event, parameters, interruptible);
}

// Scrolls the parent element's contents so that this element is visible.
void Element::ScrollIntoView(bool align_with_top)
{
	Vector2f size(0, 0);
	if (!align_with_top &&
		!boxes.empty())
	{
		size.y = boxes.back().GetOffset().y +
				 boxes.back().GetSize(Box::BORDER).y;
	}

	Element* scroll_parent = parent;
	while (scroll_parent != NULL)
	{
		int overflow_x_property = scroll_parent->GetProperty< int >(OVERFLOW_X);
		int overflow_y_property = scroll_parent->GetProperty< int >(OVERFLOW_Y);

		if ((overflow_x_property != OVERFLOW_VISIBLE &&
			 scroll_parent->GetScrollWidth() > scroll_parent->GetClientWidth()) ||
			(overflow_y_property != OVERFLOW_VISIBLE &&
			 scroll_parent->GetScrollHeight() > scroll_parent->GetClientHeight()))
		{
			Vector2f offset = scroll_parent->GetAbsoluteOffset(Box::BORDER) - GetAbsoluteOffset(Box::BORDER);
			Vector2f scroll_offset(scroll_parent->GetScrollLeft(), scroll_parent->GetScrollTop());
			scroll_offset -= offset;
			scroll_offset.x += scroll_parent->GetClientLeft();
			scroll_offset.y += scroll_parent->GetClientTop();

			if (!align_with_top)
				scroll_offset.y -= (scroll_parent->GetClientHeight() - size.y);

			if (overflow_x_property != OVERFLOW_VISIBLE)
				scroll_parent->SetScrollLeft(scroll_offset.x);
			if (overflow_y_property != OVERFLOW_VISIBLE)
				scroll_parent->SetScrollTop(scroll_offset.y);
		}

		scroll_parent = scroll_parent->GetParentNode();
	}
}

// Appends a child to this element
void Element::AppendChild(Element* child, bool dom_element)
{
	child->AddReference();
	child->SetParent(this);
	if (dom_element)
		children.insert(children.end() - num_non_dom_children, child);
	else
	{
		children.push_back(child);
		num_non_dom_children++;
	}

	child->GetStyle()->DirtyDefinition();
	child->GetStyle()->DirtyProperties();

	child->OnChildAdd(child);
	DirtyStackingContext();
	DirtyStructure();

	if (dom_element)
		DirtyLayout();
}

// Adds a child to this element, directly after the adjacent element. Inherits
// the dom/non-dom status from the adjacent element.
void Element::InsertBefore(Element* child, Element* adjacent_element)
{
	// Find the position in the list of children of the adjacent element. If
	// it's NULL or we can't find it, then we insert it at the end of the dom
	// children, as a dom element.
	size_t child_index = 0;
	bool found_child = false;
	if (adjacent_element)
	{
		for (child_index = 0; child_index < children.size(); child_index++)
		{
			if (children[child_index] == adjacent_element)
			{
				found_child = true;
				break;
			}
		}
	}

	if (found_child)
	{
		child->AddReference();
		child->SetParent(this);

		if ((int) child_index >= GetNumChildren())
			num_non_dom_children++;
		else
			DirtyLayout();

		children.insert(children.begin() + child_index, child);

		child->GetStyle()->DirtyDefinition();
		child->GetStyle()->DirtyProperties();

		child->OnChildAdd(child);
		DirtyStackingContext();
		DirtyStructure();
	}
	else
	{
		AppendChild(child);
	}	
}

// Replaces the second node with the first node.
bool Element::ReplaceChild(Element* inserted_element, Element* replaced_element)
{
	inserted_element->AddReference();
	inserted_element->SetParent(this);

	ElementList::iterator insertion_point = children.begin();
	while (insertion_point != children.end() && *insertion_point != replaced_element)
	{
		++insertion_point;
	}

	if (insertion_point == children.end())
	{
		AppendChild(inserted_element);
		return false;
	}

	children.insert(insertion_point, inserted_element);
	RemoveChild(replaced_element);

	inserted_element->GetStyle()->DirtyDefinition();
	inserted_element->GetStyle()->DirtyProperties();
	inserted_element->OnChildAdd(inserted_element);

	return true;
}

// Removes the specified child
bool Element::RemoveChild(Element* child)
{
	size_t child_index = 0;

	for (ElementList::iterator itr = children.begin(); itr != children.end(); ++itr)
	{
		// Add the element to the delete list
		if ((*itr) == child)
		{
			// Inform the context of the element's pending removal (if we have a valid context).
			Context* context = GetContext();
			if (context)
				context->OnElementRemove(child);

			child->OnChildRemove(child);

			if (child_index >= children.size() - num_non_dom_children)
				num_non_dom_children--;

			deleted_children.push_back(child);
			children.erase(itr);

			// Remove the child element as the focussed child of this element.
			if (child == focus)
			{
				focus = NULL;

				// If this child (or a descendant of this child) is the context's currently
				// focussed element, set the focus to us instead.
				Context* context = GetContext();
				if (context != NULL)
				{
					Element* focus_element = context->GetFocusElement();
					while (focus_element != NULL)
					{
						if (focus_element == child)
						{
							Focus();
							break;
						}

						focus_element = focus_element->GetParentNode();
					}
				}
			}

			DirtyLayout();
			DirtyStackingContext();
			DirtyStructure();

			return true;
		}

		child_index++;
	}

	return false;
}

bool Element::HasChildNodes() const
{
	return (int) children.size() > num_non_dom_children;
}

Element* Element::GetElementById(const String& id)
{
	// Check for special-case tokens.
	if (id == "#self")
		return this;
	else if (id == "#document")
		return GetOwnerDocument();
	else if (id == "#parent")
		return this->parent;
	else
	{
		Element* search_root = GetOwnerDocument();
		if (search_root == NULL)
			search_root = this;
		return ElementUtilities::GetElementById(search_root, id);
	}
}

// Get all elements with the given tag.
void Element::GetElementsByTagName(ElementList& elements, const String& tag)
{
	return ElementUtilities::GetElementsByTagName(elements, this, tag);
}

// Get all elements with the given class set on them.
void Element::GetElementsByClassName(ElementList& elements, const String& class_name)
{
	return ElementUtilities::GetElementsByClassName(elements, this, class_name);
}

// Access the event dispatcher
EventDispatcher* Element::GetEventDispatcher() const
{
	return event_dispatcher;
}

// Access the element background.
ElementBackground* Element::GetElementBackground() const
{
	return background;
}

// Access the element border.
ElementBorder* Element::GetElementBorder() const
{
	return border;
}

// Access the element decorators
ElementDecoration* Element::GetElementDecoration() const
{
	return decoration;
}

// Returns the element's scrollbar functionality.
ElementScroll* Element::GetElementScroll() const
{
	return scroll;
}
	
int Element::GetClippingIgnoreDepth()
{
	if (clipping_state_dirty)
	{
		IsClippingEnabled();
	}
	
	return clipping_ignore_depth;
}
	
bool Element::IsClippingEnabled()
{
	if (clipping_state_dirty)
	{
		// Is clipping enabled for this element, yes unless both overlow properties are set to visible
		clipping_enabled = style->GetProperty(OVERFLOW_X)->Get< int >() != OVERFLOW_VISIBLE 
							|| style->GetProperty(OVERFLOW_Y)->Get< int >() != OVERFLOW_VISIBLE;
		
		// Get the clipping ignore depth from the clip property
		clipping_ignore_depth = 0;
		const Property* clip_property = GetProperty(CLIP);
		if (clip_property->unit == Property::NUMBER)
			clipping_ignore_depth = clip_property->Get< int >();
		else if (clip_property->Get< int >() == CLIP_NONE)
			clipping_ignore_depth = -1;
		
		clipping_state_dirty = false;
	}
	
	return clipping_enabled;
}

// Gets the render interface owned by this element's context.
RenderInterface* Element::GetRenderInterface()
{
	Context* context = GetContext();
	if (context != NULL)
		return context->GetRenderInterface();

	return Rocket::Core::GetRenderInterface();
}

void Element::SetInstancer(ElementInstancer* _instancer)
{
	// Only record the first instancer being set as some instancers call other instancers to do their dirty work, in
	// which case we don't want to update the lowest level instancer.
	if (instancer == NULL)
	{
		instancer = _instancer;
		instancer->AddReference();
	}
}

// Forces the element to generate a local stacking context, regardless of the value of its z-index property.
void Element::ForceLocalStackingContext()
{
	local_stacking_context_forced = true;
	local_stacking_context = true;

	DirtyStackingContext();
}

// Called during the update loop after children are rendered.
void Element::OnUpdate()
{
}

// Called during render after backgrounds, borders, decorators, but before children, are rendered.
void Element::OnRender()
{
}

// Called during a layout operation, when the element is being positioned and sized.
void Element::OnLayout()
{
}

// Called when attributes on the element are changed.
void Element::OnAttributeChange(const AttributeNameList& changed_attributes)
{
	if (changed_attributes.find("id") != changed_attributes.end())
	{
		id = GetAttribute< String >("id", "");
		style->DirtyDefinition();
	}

	if (changed_attributes.find("class") != changed_attributes.end())
	{
		style->SetClassNames(GetAttribute< String >("class", ""));
	}

	// Add any inline style declarations.
	if (changed_attributes.find("style") != changed_attributes.end())
	{
		PropertyDictionary properties;
		StyleSheetParser parser;
		parser.ParseProperties(properties, GetAttribute< String >("style", ""));

		Rocket::Core::PropertyMap property_map = properties.GetProperties();
		for (Rocket::Core::PropertyMap::iterator i = property_map.begin(); i != property_map.end(); ++i)
		{
			SetProperty((*i).first, (*i).second);
		}
	}
}

// Called when properties on the element are changed.
void Element::OnPropertyChange(const PropertyNameList& changed_properties)
{
	// Force a relayout if any of the changed properties require it.
	for (PropertyNameList::const_iterator i = changed_properties.begin(); i != changed_properties.end(); ++i)
	{
		const PropertyDefinition* property_definition = StyleSheetSpecification::GetProperty(*i);
		if (property_definition)
		{
			if (property_definition->IsLayoutForced())
			{
				DirtyLayout();
				break;
			}
		}
	}

	// Update the visibility.
	if (changed_properties.find(VISIBILITY) != changed_properties.end() ||
		changed_properties.find(DISPLAY) != changed_properties.end())
	{
		bool new_visibility = GetProperty< int >(VISIBILITY) == VISIBILITY_VISIBLE &&
							  GetProperty< int >(DISPLAY) != DISPLAY_NONE;

		if (visible != new_visibility)
		{
			visible = new_visibility;

			if (parent != NULL)
				parent->DirtyStackingContext();
		}

		if (changed_properties.find(DISPLAY) != changed_properties.end())
		{
			if (parent != NULL)
				parent->DirtyStructure();
		}
	}

	// Update the position.
	if (changed_properties.find(LEFT) != changed_properties.end() ||
		changed_properties.find(RIGHT) != changed_properties.end() ||
		changed_properties.find(TOP) != changed_properties.end() ||
		changed_properties.find(BOTTOM) != changed_properties.end())
	{
		UpdateOffset();
		DirtyOffset();
	}

	// Update the z-index.
	if (changed_properties.find(Z_INDEX) != changed_properties.end())
	{
		const Property* z_index_property = GetProperty(Z_INDEX);

		if (z_index_property->unit == Property::KEYWORD &&
			z_index_property->value.Get< int >() == Z_INDEX_AUTO)
		{
			if (local_stacking_context &&
				!local_stacking_context_forced)
			{
				// We're no longer acting as a stacking context.
				local_stacking_context = false;

				stacking_context_dirty = false;
				stacking_context.clear();
			}

			// If our old z-index was not zero, then we must dirty our stacking context so we'll be re-indexed.
			if (z_index != 0)
			{
				z_index = 0;
				DirtyStackingContext();
			}
		}
		else
		{
			float new_z_index;
			if (z_index_property->unit == Property::KEYWORD)
			{
				if (z_index_property->value.Get< int >() == Z_INDEX_TOP)
					new_z_index = FLT_MAX;
				else
					new_z_index = -FLT_MAX;
			}
			else
				new_z_index = z_index_property->value.Get< float >();

			if (new_z_index != z_index)
			{
				z_index = new_z_index;

				if (parent != NULL)
					parent->DirtyStackingContext();
			}

			if (!local_stacking_context)
			{
				local_stacking_context = true;
				stacking_context_dirty = true;
			}
		}
	}

	// Dirty the background if it's changed.
	if (changed_properties.find(BACKGROUND_COLOR) != changed_properties.end())
		background->DirtyBackground();

	// Dirty the border if it's changed.
	if (changed_properties.find(BORDER_TOP_WIDTH) != changed_properties.end() ||
		changed_properties.find(BORDER_RIGHT_WIDTH) != changed_properties.end() ||
		changed_properties.find(BORDER_BOTTOM_WIDTH) != changed_properties.end() ||
		changed_properties.find(BORDER_LEFT_WIDTH) != changed_properties.end() ||
		changed_properties.find(BORDER_TOP_COLOR) != changed_properties.end() ||
		changed_properties.find(BORDER_RIGHT_COLOR) != changed_properties.end() ||
		changed_properties.find(BORDER_BOTTOM_COLOR) != changed_properties.end() ||
		changed_properties.find(BORDER_LEFT_COLOR) != changed_properties.end())
		border->DirtyBorder();

	// Fetch a new font face if it has been changed.
	if (changed_properties.find(FONT_FAMILY) != changed_properties.end() ||
		changed_properties.find(FONT_CHARSET) != changed_properties.end() ||
		changed_properties.find(FONT_WEIGHT) != changed_properties.end() ||
		changed_properties.find(FONT_STYLE) != changed_properties.end() ||
		changed_properties.find(FONT_SIZE) != changed_properties.end())
	{
		// Store the old em; if it changes, then we need to dirty all em-relative properties.
		int old_em = -1;
		if (font_face_handle != NULL)
			old_em = font_face_handle->GetLineHeight();

		// Fetch the new font face.
		FontFaceHandle* new_font_face_handle = ElementUtilities::GetFontFaceHandle(this);

		// If this is different from our current font face, then we've got to nuke
		// all our characters and tell our parent that we have to be re-laid out.
		if (new_font_face_handle != font_face_handle)
		{
			if (font_face_handle)
				font_face_handle->RemoveReference();

			font_face_handle = new_font_face_handle;

			// Our font face has changed; odds are, so has our em. All of our em-relative values
			// have therefore probably changed as well, so we'll need to dirty them.
			int new_em = -1;
			if (font_face_handle != NULL)
				new_em = font_face_handle->GetLineHeight();

			if (old_em != new_em)
			{
				style->DirtyEmProperties();
			}
		}
		else if (new_font_face_handle != NULL)
			new_font_face_handle->RemoveReference();
	}
	
	// Check for clipping state changes
	if (changed_properties.find(CLIP) != changed_properties.end() ||
		changed_properties.find(OVERFLOW_X) != changed_properties.end() ||
		changed_properties.find(OVERFLOW_Y) != changed_properties.end())
	{
		clipping_state_dirty = true;
	}
}

// Called when a child node has been added somewhere in the hierarchy
void Element::OnChildAdd(Element* child)
{
	if (parent)
		parent->OnChildAdd(child);
}

// Called when a child node has been removed somewhere in the hierarchy
void Element::OnChildRemove(Element* child)
{
	if (parent)
		parent->OnChildRemove(child);
}

// Update the element's layout if required.
void Element::UpdateLayout()
{
	ElementDocument* document = GetOwnerDocument();
	if (document != NULL)
		document->UpdateLayout();
}

// Forces a re-layout of this element, and any other children required.
void Element::DirtyLayout()
{
	Element* document = GetOwnerDocument();
	if (document != NULL)
		document->DirtyLayout();
}

// Forces a reevaluation of applicable font effects.
void Element::DirtyFont()
{
	for (size_t i = 0; i < children.size(); ++i)
		children[i]->DirtyFont();
}

void Element::OnReferenceDeactivate()
{
	if (instancer)
	{
		instancer->ReleaseElement(this);
	}
	else
	{
		// Hopefully we can just delete ourselves.
		//delete this;
		Log::Message(Log::LT_WARNING, "Leak detected: element %s not instanced via Rocket Factory. Unable to release.", GetAddress().CString());
	}
}

void Element::ProcessEvent(Event& event)
{
	if (event == MOUSEDOWN && IsPointWithinElement(Vector2f(event.GetParameter< float >("mouse_x", 0), event.GetParameter< float >("mouse_y", 0))))
		SetPseudoClass("active", true);

	if (event == MOUSESCROLL)
	{
		int wheel_delta = event.GetParameter< int >("wheel_delta", 0);
		if ((wheel_delta < 0 && GetScrollTop() > 0) ||
			(wheel_delta > 0 && GetScrollHeight() > GetScrollTop() + GetClientHeight()))
		{
			int overflow_property = GetProperty< int >(OVERFLOW_Y);
			if (overflow_property == OVERFLOW_AUTO ||
				overflow_property == OVERFLOW_SCROLL)
			{
				SetScrollTop(GetScrollTop() + wheel_delta * ElementUtilities::GetLineHeight(this));
				event.StopPropagation();
			}
		}

		return;
	}

	if (event.GetTargetElement() == this)
	{
		if (event == MOUSEOVER)
			SetPseudoClass("hover", true);
		else if (event == MOUSEOUT)
			SetPseudoClass("hover", false);
		else if (event == FOCUS)
			SetPseudoClass(FOCUS, true);
		else if (event == BLUR)
			SetPseudoClass(FOCUS, false);
	}
}

void Element::GetRML(String& content)
{
	// First we start the open tag, add the attributes then close the open tag.
	// Then comes the children in order, then we add our close tag.
	content.Append("<");
	content.Append(tag);

	int index = 0;
	String name;
	String value;
	while (IterateAttributes(index, name, value))	
	{
		size_t length = name.Length() + value.Length() + 8;
		String attribute(length, " %s=\"%s\"", name.CString(), value.CString());
		content.Append(attribute);
	}

	if (HasChildNodes())
	{
		content.Append(">");

		GetInnerRML(content);

		content.Append("</");
		content.Append(tag);
		content.Append(">");
	}
	else
	{
		content.Append(" />");
	}
}

void Element::SetParent(Element* _parent)
{	
	// If there's an old parent, detach from it first.
	if (parent &&
		parent != _parent)
		parent->RemoveChild(this);

	// Save our parent
	parent = _parent;
}

void Element::ReleaseDeletedElements()
{
	for (size_t i = 0; i < active_children.size(); i++)
	{
		active_children[i]->ReleaseDeletedElements();
	}

	ReleaseElements(deleted_children);
	active_children = children;
}

void Element::ReleaseElements(ElementList& released_elements)
{
	// Remove deleted children from this element.
	while (!released_elements.empty())
	{
		Element* element = released_elements.back();
		released_elements.pop_back();

		// If this element has been added back into our list, then we remove our previous oustanding reference on it
		// and continue.
		if (std::find(children.begin(), children.end(), element) != children.end())
		{
			element->RemoveReference();
			continue;
		}

		// Set the parent to NULL unless it's been reparented already.
		if (element->GetParentNode() == this)
			element->parent = NULL;

		element->RemoveReference();
	}
}

void Element::DirtyOffset()
{
	offset_dirty = true;

	// Not strictly true ... ?
	for (size_t i = 0; i < children.size(); i++)
		children[i]->DirtyOffset();
}

void Element::UpdateOffset()
{
	int position_property = GetProperty< int >(POSITION);
	if (position_property == POSITION_ABSOLUTE ||
		position_property == POSITION_FIXED)
	{
		if (offset_parent != NULL)
		{
			const Box& parent_box = offset_parent->GetBox();
			Vector2f containing_block = parent_box.GetSize(Box::PADDING);

			const Property *left = GetLocalProperty(LEFT);
			const Property *right = GetLocalProperty(RIGHT);
			// If the element is anchored left, then the position is offset by that resolved value.
			if (left != NULL && left->unit != Property::KEYWORD)
				relative_offset_base.x = parent_box.GetEdge(Box::BORDER, Box::LEFT) + (ResolveProperty(LEFT, containing_block.x) + GetBox().GetEdge(Box::MARGIN, Box::LEFT));
			// If the element is anchored right, then the position is set first so the element's right-most edge
			// (including margins) will render up against the containing box's right-most content edge, and then
			// offset by the resolved value.
			if (right != NULL && right->unit != Property::KEYWORD)
				relative_offset_base.x = containing_block.x + parent_box.GetEdge(Box::BORDER, Box::LEFT) - (ResolveProperty(RIGHT, containing_block.x) + GetBox().GetSize(Box::BORDER).x + GetBox().GetEdge(Box::MARGIN, Box::RIGHT));

			const Property *top = GetLocalProperty(TOP);
			const Property *bottom = GetLocalProperty(BOTTOM);
			// If the element is anchored top, then the position is offset by that resolved value.
			if (top != NULL && top->unit != Property::KEYWORD)
				relative_offset_base.y = parent_box.GetEdge(Box::BORDER, Box::TOP) + (ResolveProperty(TOP, containing_block.y) + GetBox().GetEdge(Box::MARGIN, Box::TOP));
			// If the element is anchored bottom, then the position is set first so the element's right-most edge
			// (including margins) will render up against the containing box's right-most content edge, and then
			// offset by the resolved value.
			else if (bottom != NULL && bottom->unit != Property::KEYWORD)
				relative_offset_base.y = containing_block.y + parent_box.GetEdge(Box::BORDER, Box::TOP) - (ResolveProperty(BOTTOM, containing_block.y) + GetBox().GetSize(Box::BORDER).y + GetBox().GetEdge(Box::MARGIN, Box::BOTTOM));
		}
	}
	else if (position_property == POSITION_RELATIVE)
	{
		if (offset_parent != NULL)
		{
			const Box& parent_box = offset_parent->GetBox();
			Vector2f containing_block = parent_box.GetSize();

			const Property *left = GetLocalProperty(LEFT);
			const Property *right = GetLocalProperty(RIGHT);
			if (left != NULL && left->unit != Property::KEYWORD)
				relative_offset_position.x = ResolveProperty(LEFT, containing_block.x);
			else if (right != NULL && right->unit != Property::KEYWORD)
				relative_offset_position.x = -1 * ResolveProperty(RIGHT, containing_block.x);
			else
				relative_offset_position.x = 0;

			const Property *top = GetLocalProperty(TOP);
			const Property *bottom = GetLocalProperty(BOTTOM);
			if (top != NULL && top->unit != Property::KEYWORD)
				relative_offset_position.y = ResolveProperty(TOP, containing_block.y);
			else if (bottom != NULL && bottom->unit != Property::KEYWORD)
				relative_offset_position.y = -1 * ResolveProperty(BOTTOM, containing_block.y);
			else
				relative_offset_position.y = 0;
		}
	}
	else
	{
		relative_offset_position.x = 0;
		relative_offset_position.y = 0;
	}

	LayoutEngine::Round(relative_offset_base);
	LayoutEngine::Round(relative_offset_position);
}

void Element::BuildLocalStackingContext()
{
	stacking_context_dirty = false;
	stacking_context.clear();

	BuildStackingContext(&stacking_context);
	std::stable_sort(stacking_context.begin(), stacking_context.end(), ElementSortZIndex());
}

void Element::BuildStackingContext(ElementList* new_stacking_context)
{
	// Build the list of ordered children. Our child list is sorted within the stacking context so stacked elements
	// will render in the right order; ie, positioned elements will render on top of inline elements, which will render
	// on top of floated elements, which will render on top of block elements.
	std::vector< std::pair< Element*, float > > ordered_children;
	for (size_t i = 0; i < children.size(); ++i)
	{
		Element* child = children[i];

		if (!child->IsVisible())
			continue;

		std::pair< Element*, float > ordered_child;
		ordered_child.first = child;

		if (child->GetProperty< int >(POSITION) != POSITION_STATIC)
			ordered_child.second = 3;
		else if (child->GetProperty< int >(FLOAT) != FLOAT_NONE)
			ordered_child.second = 1;
		else if (child->GetProperty< int >(DISPLAY) == DISPLAY_BLOCK)
			ordered_child.second = 0;
		else
			ordered_child.second = 2;

		ordered_children.push_back(ordered_child);
	}

	// Sort the list!
	std::stable_sort(ordered_children.begin(), ordered_children.end(), ElementSortZOrder());

	// Add the list of ordered children into the stacking context in order.
	for (size_t i = 0; i < ordered_children.size(); ++i)
	{
		new_stacking_context->push_back(ordered_children[i].first);

		if (!ordered_children[i].first->local_stacking_context)
			ordered_children[i].first->BuildStackingContext(new_stacking_context);
	}
}

void Element::DirtyStackingContext()
{
	// The first ancestor of ours that doesn't have an automatic z-index is the ancestor that is establishing our local
	// stacking context.
	Element* stacking_context_parent = this;
	while (stacking_context_parent != NULL &&
		   !stacking_context_parent->local_stacking_context)
		stacking_context_parent = stacking_context_parent->GetParentNode();

	if (stacking_context_parent != NULL)
		stacking_context_parent->stacking_context_dirty = true;
}

void Element::DirtyStructure()
{
	// Clear the cached owner document
	owner_document = NULL;
	
	// Inform all children that the structure is drity
	for (size_t i = 0; i < children.size(); ++i)
	{
		const ElementDefinition* element_definition = children[i]->GetStyle()->GetDefinition();
		if (element_definition != NULL &&
			element_definition->IsStructurallyVolatile())
		{
			children[i]->GetStyle()->DirtyDefinition();
		}

		children[i]->DirtyStructure();
	}
}

}
}
