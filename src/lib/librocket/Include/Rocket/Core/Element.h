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

#ifndef ROCKETCOREELEMENT_H
#define ROCKETCOREELEMENT_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/ScriptInterface.h>
#include <Rocket/Core/Header.h>
#include <Rocket/Core/Box.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {
	class Dictionary;
}
}

namespace Rocket {
namespace Core {

class Context;
class Decorator;
class ElementInstancer;
class EventDispatcher;
class EventListener;
class ElementBackground;
class ElementBorder;
class ElementDecoration;
class ElementDefinition;
class ElementDocument;
class ElementScroll;
class ElementStyle;
class FontFaceHandle;
class PropertyDictionary;
class RenderInterface;
class StyleSheet;

/**
	A generic element in the DOM tree.

	@author Peter Curry
 */

class ROCKETCORE_API Element : public ScriptInterface
{
public:
	/// Constructs a new libRocket element. This should not be called directly; use the Factory
	/// instead.
	/// @param[in] tag The tag the element was declared as in RML.
	Element(const String& tag);
	virtual ~Element();

	void Update();
	void Render();

	/// Clones this element, returning a new, unparented element.
	Element* Clone() const;

	/** @name Classes
	 */
	//@{
	/// Sets or removes a class on the element.
	/// @param[in] class_name The name of the class to add or remove from the class list.
	/// @param[in] activate True if the class is to be added, false to be removed.
	void SetClass(const String& class_name, bool activate);
	/// Checks if a class is set on the element.
	/// @param[in] class_name The name of the class to check for.
	/// @return True if the class is set on the element, false otherwise.
	bool IsClassSet(const String& class_name) const;
	/// Specifies the entire list of classes for this element. This will replace any others specified.
	/// @param[in] class_names The list of class names to set on the style, separated by spaces.
	void SetClassNames(const String& class_names);
	/// Return the active class list.
	/// @return The space-separated list of classes active on the element.
	String GetClassNames() const;
	//@}

	/// Returns the active style sheet for this element. This may be NULL.
	/// @return The element's style sheet.
	virtual StyleSheet* GetStyleSheet() const;

	/// Returns the element's definition, updating if necessary.
	/// @return The element's definition.
	const ElementDefinition* GetDefinition();

	/// Fills a string with the full address of this element.
	/// @param[in] include_pseudo_classes True if the address is to include the pseudo-classes of the leaf element.
	/// @return The address of the element, including its full parentage.
	String GetAddress(bool include_pseudo_classes = false) const;

	/// Sets the position of this element, as a two-dimensional offset from another element.
	/// @param[in] offset The offset (in pixels) of our primary box's top-left border corner from our offset parent's top-left border corner.
	/// @param[in] offset_parent The element this element is being positioned relative to.
	/// @param[in] offset_fixed True if the element is fixed in place (and will not scroll), false if not.
	void SetOffset(const Vector2f& offset, Element* offset_parent, bool offset_fixed = false);
	/// Returns the position of the top-left corner of one of the areas of this element's primary box, relative to its
	/// offset parent's top-left border corner.
	/// @param[in] area The desired area position.
	/// @return The relative offset.
	Vector2f GetRelativeOffset(Box::Area area = Box::CONTENT);
	/// Returns the position of the top-left corner of one of the areas of this element's primary box, relative to
	/// the element root.
	/// @param[in] area The desired area position.
	/// @return The absolute offset.
	Vector2f GetAbsoluteOffset(Box::Area area = Box::CONTENT);

	/// Sets an alternate area to use as the client area.
	/// @param[in] client_area The box area to use as the element's client area.
	void SetClientArea(Box::Area client_area);
	/// Returns the area the element uses as its client area.
	/// @return The box area used as the element's client area.
	Box::Area GetClientArea() const;

	/// Sets the dimensions of the element's internal content. This is the tightest fitting box surrounding all of
	/// this element's logical children, plus the element's padding.
	/// @param[in] content_offset The offset of the box's internal content.
	/// @param[in] content_box The dimensions of the box's internal content.
	void SetContentBox(const Vector2f& content_offset, const Vector2f& content_box);
	/// Sets the box describing the size of the element, and removes all others.
	/// @param[in] box The new dimensions box for the element.
	void SetBox(const Box& box);
	/// Adds a box to the end of the list describing this element's geometry.
	/// @param[in] box The auxiliary box for the element.
	void AddBox(const Box& box);
	/// Returns one of the boxes describing the size of the element.
	/// @param[in] index The index of the desired box. If this is outside of the bounds of the element's list of boxes, it will be clamped.
	/// @return The requested box.
	const Box& GetBox(int index = 0);
	/// Returns the number of boxes making up this element's geometry.
	/// @return the number of boxes making up this element's geometry.
	int GetNumBoxes();

	/// Returns the baseline of the element, in pixels offset from the bottom of the element's content area.
	/// @return The element's baseline. A negative baseline will be further 'up' the element, a positive on further 'down'. The default element will return 0.
	virtual float GetBaseline() const;
	/// Gets the intrinsic dimensions of this element, if it is of a type that has an inherent size. This size will
	/// only be overriden by a styled width or height.
	/// @param[in] dimensions The dimensions to size, if appropriate.
	/// @return True if the element has intrinsic dimensions, false otherwise. The default element will return false.
	virtual bool GetIntrinsicDimensions(Vector2f& dimensions);

	/// Checks if a given point in screen coordinates lies within the bordered area of this element.
	/// @param[in] point The point to test.
	/// @return True if the element is within this element, false otherwise.
	virtual bool IsPointWithinElement(const Vector2f& point);

	/// Returns the visibility of the element.
	/// @return True if the element is visible, false otherwise.
	bool IsVisible() const;
	/// Returns the z-index of the element.
	/// @return The element's z-index.
	float GetZIndex() const;

	/// Returns the element's font face handle.
	/// @return The element's font face handle.
	FontFaceHandle* GetFontFaceHandle() const;

	/** @name Properties
	 */
	//@{
	/// Sets a local property override on the element.
	/// @param[in] name The name of the new property.
	/// @param[in] value The new property to set.
	/// @return True if the property parsed successfully, false otherwise.
	bool SetProperty(const String& name, const String& value);
	/// Sets a local property override on the element to a pre-parsed value.
	/// @param[in] name The name of the new property.
	/// @param[in] property The parsed property to set.
	/// @return True if the property was set successfully, false otherwise.
	bool SetProperty(const String& name, const Property& property);
	/// Removes a local property override on the element; its value will revert to that defined in
	/// the style sheet.
	/// @param[in] name The name of the local property definition to remove.
	void RemoveProperty(const String& name);
	/// Returns one of this element's properties. If this element is not defined this property, or a parent cannot
	/// be found that we can inherit the property from, the default value will be returned.
	/// @param[in] name The name of the property to fetch the value for.
	/// @return The value of this property for this element, or NULL if no property exists with the given name.
	const Property* GetProperty(const String& name);		
	/// Returns the values of one of this element's properties.		
	/// @param[in] name The name of the property to get.
	/// @return The value of this property.
	template < typename T >
	T GetProperty(const String& name);
	/// Returns one of this element's properties. If this element is not defined this property, NULL will be
	/// returned.
	/// @param[in] name The name of the property to fetch the value for.
	/// @return The value of this property for this element, or NULL if this property has not been explicitly defined for this element.
	const Property* GetLocalProperty(const String& name);		
	/// Resolves one of this element's properties. If the value is a number or px, this is returned. If it's a 
	/// percentage then it is resolved based on the second argument (the base value).
	/// @param[in] name The name of the property to resolve the value for.
	/// @param[in] base_value The value that is scaled by the percentage value, if it is a percentage.
	/// @return The value of this property for this element.
	float ResolveProperty(const String& name, float base_value);

	/// Iterates over the properties defined on this element.
	/// @param[inout] index Index of the property to fetch. This is incremented to the next valid index after the fetch. Indices are not necessarily incremental.
	/// @param[out] pseudo_classes The pseudo-classes the property is defined by.
	/// @param[out] name The name of the property at the specified index.
	/// @param[out] property The property at the specified index.
	/// @return True if a property was successfully fetched.
	bool IterateProperties(int& index, PseudoClassList& pseudo_classes, String& name, const Property*& property) const;
	///@}

	/** @name Pseudo-classes
	 */
	//@{
	/// Sets or removes a pseudo-class on the element.
	/// @param[in] pseudo_class The pseudo class to activate or deactivate.
	/// @param[in] activate True if the pseudo-class is to be activated, false to be deactivated.
	void SetPseudoClass(const String& pseudo_class, bool activate);
	/// Checks if a specific pseudo-class has been set on the element.
	/// @param[in] pseudo_class The name of the pseudo-class to check for.
	/// @return True if the pseudo-class is set on the element, false if not.
	bool IsPseudoClassSet(const String& pseudo_class) const;
	/// Checks if a complete set of pseudo-classes are set on the element.
	/// @param[in] pseudo_classes The set of pseudo-classes to check for.
	/// @return True if all of the pseudo-classes are set, false if not.
	bool ArePseudoClassesSet(const PseudoClassList& pseudo_classes) const;
	/// Gets a list of the current active pseudo-classes.
	/// @return The list of active pseudo-classes.
	const PseudoClassList& GetActivePseudoClasses() const;
	//@}

	/** @name Attributes
	 */
	//@{
	/// Sets an attribute on the element.
	/// @param[in] name Name of the attribute.
	/// @param[in] value Value of the attribute.
	template< typename T >
	void SetAttribute(const String& name, const T& value);
	/// Gets the specified attribute.
	/// @param[in] name Name of the attribute to retrieve.
	/// @return A variant representing the attribute, or NULL if the attribute doesn't exist.
	Variant* GetAttribute(const String& name) const;
	/// Gets the specified attribute, with default value.
	/// @param[in] name Name of the attribute to retrieve.
	/// @param[in] default_value Value to return if the attribute doesn't exist.
	template< typename T >
	T GetAttribute(const String& name, const T& default_value) const;
	/// Checks if the element has a certain attribute.
	/// @param[in] name The name of the attribute to check for.
	/// @return True if the element has the given attribute, false if not.
	bool HasAttribute(const String& name);
	/// Removes the attribute from the element.
	/// @param[in] name Name of the attribute.
	void RemoveAttribute(const String& name);
	/// Set a group of attributes.
	/// @param[in] attributes Attributes to set.
	void SetAttributes(const ElementAttributes* attributes);
	/// Iterates over the attributes.
	/// @param[inout] index Index to fetch. This is incremented after the fetch.
	/// @param[out] name Name of the attribute at the specified index
	/// @param[out] value Value of the attribute at the specified index.
	/// @return True if an attribute was successfully fetched.
	template< typename T >
	bool IterateAttributes(int& index, String& name, T& value) const;
	/// Returns the number of attributes on the element.
	/// @return The number of attributes on the element.
	int GetNumAttributes() const;
	//@}

	/** @name Decorators
	 */
	//@{
	/// Iterates over all decorators attached to the element. Note that all decorators are iterated
	/// over, not just active ones.
	/// @param[inout] index Index to fetch. This is incremented after the fetch.
	/// @param[out] pseudo_classes The pseudo-classes the decorator required to be active before it renders.
	/// @param[out] name The name of the decorator at the specified index.
	/// @param[out] decorator The decorator at the specified index.
	/// @param[out] decorator_data This element's handle to any data is has stored against the decorator.
	/// @return True if a decorator was successfully fetched, false if not.
	bool IterateDecorators(int& index, PseudoClassList& pseudo_classes, String& name, Decorator*& decorator, DecoratorDataHandle& decorator_data);
	//@}

	/// Gets the outer-most focus element down the tree from this node.
	/// @return Outer-most focus element.
	Element* GetFocusLeafNode();

	/// Returns the element's context.
	/// @return The context this element's document exists within.
	Context* GetContext();

	/** @name DOM Properties
	 */
	//@{

	/// Gets the name of the element.
	/// @return The name of the element.
	const String& GetTagName() const;

	/// Gets the id of the element.
	/// @return The element's id.
	const String& GetId() const;
	/// Sets the id of the element.
	/// @param[in] id The new id of the element.
	void SetId(const String& id);

	/// Gets the horizontal offset from the context's left edge to element's left border edge.
	/// @return The horizontal offset of the element within its context, in pixels.
	float GetAbsoluteLeft();
	/// Gets the vertical offset from the context's top edge to element's top border edge.
	/// @return The vertical offset of the element within its context, in pixels.
	float GetAbsoluteTop();

	/// Gets the horizontal offset from the element's left border edge to the left edge of its client area. This is
	/// usually the edge of the padding, but may be the content area for some replaced elements.
	/// @return The horizontal offset of the element's client area, in pixels.
	float GetClientLeft();
	/// Gets the vertical offset from the element's top border edge to the top edge of its client area. This is
	/// usually the edge of the padding, but may be the content area for some replaced elements.
	/// @return The vertical offset of the element's client area, in pixels.
	float GetClientTop();
	/// Gets the width of the element's client area. This is usually the padded area less the vertical scrollbar
	/// width, but may be the content area for some replaced elements.
	/// @return The width of the element's client area, usually including padding but not the vertical scrollbar width, border or margin.
	float GetClientWidth();
	/// Gets the height of the element's client area. This is usually the padded area less the horizontal scrollbar
	/// height, but may be the content area for some replaced elements.
	/// @return The inner height of the element, usually including padding but not the horizontal scrollbar height, border or margin.
	float GetClientHeight();

	/// Returns the element from which all offset calculations are currently computed.
	/// @return This element's offset parent.
	Element* GetOffsetParent();
	/// Gets the distance from this element's left border to its offset parent's left border.
	/// @return The horizontal distance (in pixels) from this element's offset parent to itself.
	float GetOffsetLeft();
	/// Gets the distance from this element's top border to its offset parent's top border.
	/// @return The vertical distance (in pixels) from this element's offset parent to itself.
	float GetOffsetTop();
	/// Gets the width of the element, including the client area, padding, borders and scrollbars, but not margins.
	/// @return The width of the rendered element, in pixels.
	float GetOffsetWidth();
	/// Gets the height of the element, including the client area, padding, borders and scrollbars, but not margins.
	/// @return The height of the rendered element, in pixels.
	float GetOffsetHeight();

	/// Gets the left scroll offset of the element.
	/// @return The element's left scroll offset.
	float GetScrollLeft();
	/// Sets the left scroll offset of the element.
	/// @param[in] scroll_left The element's new left scroll offset.
	void SetScrollLeft(float scroll_left);
	/// Gets the top scroll offset of the element.
	/// @return The element's top scroll offset.
	float GetScrollTop();
	/// Sets the top scroll offset of the element.
	/// @param[in] scroll_top The element's new top scroll offset.
	void SetScrollTop(float scroll_top);
	/// Gets the width of the scrollable content of the element; it includes the element padding but not its margin.
	/// @return The width (in pixels) of the of the scrollable content of the element.
	float GetScrollWidth();
	/// Gets the height of the scrollable content of the element; it includes the element padding but not its margin.
	/// @return The height (in pixels) of the of the scrollable content of the element.
	float GetScrollHeight();

	/// Gets the object representing the declarations of an element's style attributes.
	/// @return The element's style.
	ElementStyle* GetStyle();

	/// Gets the document this element belongs to.
	/// @return This element's document.
	virtual ElementDocument* GetOwnerDocument();

	/// Gets this element's parent node.
	/// @return This element's parent.
	Element* GetParentNode() const;

	/// Gets the element immediately following this one in the tree.
	/// @return This element's next sibling element, or NULL if there is no sibling element.
	Element* GetNextSibling() const;
	/// Gets the element immediately preceding this one in the tree.
	/// @return This element's previous sibling element, or NULL if there is no sibling element.
	Element* GetPreviousSibling() const;

	/// Returns the first child of this element.
	/// @return This element's first child, or NULL if it contains no children.
	Element* GetFirstChild() const;
	/// Gets the last child of this element.
	/// @return This element's last child, or NULL if it contains no children.
	Element* GetLastChild() const;
	/// Get the child element at the given index.
	/// @param[in] index Index of child to get.
	/// @return The child element at the given index.
	Element* GetChild(int index) const;
	/// Get the current number of children in this element
	/// @param[in] include_non_dom_elements True if the caller wants to include the non DOM children. Only set this to true if you know what you're doing!
	/// @return The number of children.
	int GetNumChildren(bool include_non_dom_elements = false) const;

	/// Gets the markup and content of the element.
	/// @param[out] content The content of the element.
	virtual void GetInnerRML(String& content) const;
	/// Gets the markup and content of the element.
	/// @return The content of the element.
	String GetInnerRML() const;
	/// Sets the markup and content of the element. All existing children will be replaced.
	/// @param[in] rml The new content of the element.
	void SetInnerRML(const String& rml);

	//@}

	/** @name DOM Methods
	 */
	//@{

	/// Gives focus to the current element.
	/// @return True if the change focus request was successful
	bool Focus();
	/// Removes focus from from this element.
	void Blur();
	/// Fakes a mouse click on this element.
	void Click();

	/// Adds an event listener to this element.
	/// @param[in] event Event to attach to.
	/// @param[in] listener The listener object to be attached.
	/// @param[in] in_capture_phase True to attach in the capture phase, false in bubble phase.
	void AddEventListener(const String& event, EventListener* listener, bool in_capture_phase = false);
	/// Removes an event listener from this element.
	/// @param[in] event Event to detach from.
	/// @param[in] listener The listener object to be detached.
	/// @param[in] in_capture_phase True to detach from the capture phase, false from the bubble phase.
	void RemoveEventListener(const String& event, EventListener* listener, bool in_capture_phase = false);
	/// Sends an event to this element.
	/// @param[in] event Name of the event in string form.
	/// @param[in] parameters The event parameters.
	/// @param[in] interruptible True if the propagation of the event be stopped.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool DispatchEvent(const String& event, const Dictionary& parameters, bool interruptible = false);

	/// Scrolls the parent element's contents so that this element is visible.
	/// @param[in] align_with_top If true, the element will align itself to the top of the parent element's window. If false, the element will be aligned to the bottom of the parent element's window.
	void ScrollIntoView(bool align_with_top = true);

	/// Append a child to this element.
	/// @param[in] element The element to append as a child.
	/// @param[in] dom_element True if the element is to be part of the DOM, false otherwise. Only set this to false if you know what you're doing!
	void AppendChild(Element* element, bool dom_element = true);
	/// Adds a child to this element, directly after the adjacent element. The new element inherits the DOM/non-DOM
	/// status from the adjacent element.
	/// @param[in] element Element to insert into the this element.
	/// @param[in] adjacent_element The element to insert directly before.
	void InsertBefore(Element* element, Element* adjacent_element);
	/// Replaces the second node with the first node.
	/// @param[in] inserted_element The element that will be inserted and replace the other element.
	/// @param[in] replaced_element The existing element that will be replaced. If this doesn't exist, inserted_element will be appended.
	/// @return True if the replaced_element was found, false otherwise.
	bool ReplaceChild(Element* inserted_element, Element* replaced_element);
	/// Remove a child element from this element.
	/// @param[in] The element to remove.
	/// @returns True if the element was found and removed.
	bool RemoveChild(Element* element);
	/// Returns whether or not this element has any DOM children.
	/// @return True if the element has at least one DOM child, false otherwise.
	bool HasChildNodes() const;

	/// Get a child element by its ID.
	/// @param[in] id Id of the the child element
	/// @return The child of this element with the given ID, or NULL if no such child exists.
	Element* GetElementById(const String& id);
	/// Get all descendant elements with the given tag.
	/// @param[out] elements Resulting elements.
	/// @param[in] tag Tag to search for.
	void GetElementsByTagName(ElementList& elements, const String& tag);
	/// Get all descendant elements with the given class set on them.
	/// @param[out] elements Resulting elements.
	/// @param[in] tag Tag to search for.
	void GetElementsByClassName(ElementList& elements, const String& class_name);
	//@}

	/**
		@name Internal Functions
	 */
	//@{
	/// Access the event dispatcher for this element.
	/// @return The element's dispatcher.
	EventDispatcher* GetEventDispatcher() const;
	/// Access the element background.
	/// @return The element's background.
	ElementBackground* GetElementBackground() const;
	/// Access the element border.
	/// @return The element's boder.
	ElementBorder* GetElementBorder() const;
	/// Access the element decorators.
	/// @return The element decoration.
	ElementDecoration* GetElementDecoration() const;
	/// Returns the element's scrollbar functionality.
	/// @return The element's scrolling functionality.
	ElementScroll* GetElementScroll() const;
	//@}
	
	/// Returns true if this element requires clipping
	int GetClippingIgnoreDepth();
	/// Returns true if this element has clipping enabled
	bool IsClippingEnabled();

	/// Gets the render interface owned by this element's context.
	/// @return The element's context's render interface.
	RenderInterface* GetRenderInterface();

	/// Sets the instancer to use for releasing this element.
	/// @param[in] instancer Instancer to set on this element.
	void SetInstancer(ElementInstancer* instancer);

	/// Called for every event sent to this element or one of its descendants.
	/// @param[in] event The event to process.
	virtual void ProcessEvent(Event& event);
	
	/// Update the element's layout if required.
	void UpdateLayout();

protected:
	/// Forces the element to generate a local stacking context, regardless of the value of its z-index
	/// property.
	void ForceLocalStackingContext();

	/// Called during the update loop after children are updated.
	virtual void OnUpdate();
	/// Called during render after backgrounds, borders, decorators, but before children, are rendered.
	virtual void OnRender();

	/// Called during a layout operation, when the element is being positioned and sized.
	virtual void OnLayout();

	/// Called when attributes on the element are changed.
	/// @param[in] changed_attributes The attributes changed on the element.
	virtual void OnAttributeChange(const AttributeNameList& changed_attributes);
	/// Called when properties on the element are changed.
	/// @param[in] changed_properties The properties changed on the element.
	virtual void OnPropertyChange(const PropertyNameList& changed_properties);

	/// Called when a child node has been added somewhere in the hierarchy.
	// @param[in] child The element that has been added. This may be this element.
	virtual void OnChildAdd(Element* child);
	/// Called when a child node has been removed somewhere in the hierarchy.
	// @param[in] child The element that has been removed. This may be this element.
	virtual void OnChildRemove(Element* child);

	/// Forces a re-layout of this element, and any other elements required.
	virtual void DirtyLayout();

	/// Forces a reevaluation of applicable font effects.
	virtual void DirtyFont();

	/// Returns the RML of this element and all children.
	/// @param[out] content The content of this element and those under it, in XML form.
	virtual void GetRML(String& content);

	virtual void OnReferenceDeactivate();

private:
	void SetParent(Element* parent);

	void ReleaseDeletedElements();
	void ReleaseElements(ElementList& elements);

	void DirtyOffset();
	void UpdateOffset();

	void BuildLocalStackingContext();
	void BuildStackingContext(ElementList* stacking_context);
	void DirtyStackingContext();

	void DirtyStructure();

	// Original tag this element came from.
	String tag;

	// The optional, unique ID of this object.
	String id;

	// Instancer that created us, used for destruction.
	ElementInstancer* instancer;

	// Parent element.
	Element* parent;
	// Currently focused child object
	Element* focus;
	// The owning document
	ElementDocument* owner_document;

	// The event dispatcher for this element.
	EventDispatcher* event_dispatcher;
	// Style information for this element.
	ElementStyle* style;
	// Background functionality for this element.
	ElementBackground* background;
	// Border functionality for this element.
	ElementBorder* border;
	// Decorator information for this element.
	ElementDecoration* decoration;
	// Scrollbar information for this element.
	ElementScroll* scroll;
	// Attributes on this element.
	ElementAttributes attributes;

	// The offset of the element, and the element it is offset from.
	Element* offset_parent;
	Vector2f relative_offset_base;		// the base offset from the parent
	Vector2f relative_offset_position;	// the offset of a relatively positioned element
	bool offset_fixed;

	mutable Vector2f absolute_offset;
	mutable bool offset_dirty;

	// The offset this element adds to its logical children due to scrolling content.
	Vector2f scroll_offset;

	// The size of the element.
	typedef std::vector< Box > BoxList;
	BoxList boxes;
	// And of the element's internal content.
	Vector2f content_offset;
	Vector2f content_box;

	// Defines what box area represents the element's client area; this is usually padding, but may be content.
	Box::Area client_area;

	// True if the element is visible and active.
	bool visible;

	ElementList children;
	int num_non_dom_children;

	ElementList active_children;
	ElementList deleted_children;

	float z_index;
	bool local_stacking_context;
	bool local_stacking_context_forced;

	ElementList stacking_context;
	bool stacking_context_dirty;

	// The element's font face; used to render text and resolve em / ex properties.
	FontFaceHandle* font_face_handle;
	
	// Cached rendering information
	int clipping_ignore_depth;
	bool clipping_enabled;
	bool clipping_state_dirty;

	friend class Context;
	friend class ElementStyle;
	friend class LayoutEngine;
	friend class LayoutInlineBox;
};

#include <Rocket/Core/Element.inl>

}
}

#endif
