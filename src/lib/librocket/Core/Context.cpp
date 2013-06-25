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
#include <Rocket/Core.h>
#include "EventDispatcher.h"
#include "EventIterators.h"
#include "PluginRegistry.h"
#include "StreamFile.h"
#include <Rocket/Core/StreamMemory.h>
#include <algorithm>
#include <iterator>

namespace Rocket {
namespace Core {

const float DOUBLE_CLICK_TIME = 0.5f;

Context::Context(const String& name) : name(name), dimensions(0, 0), mouse_position(0, 0), clip_origin(-1, -1), clip_dimensions(-1, -1)
{
	instancer = NULL;

	// Initialise this to NULL; this will be set in Rocket::Core::CreateContext().
	render_interface = NULL;

	root = Factory::InstanceElement(NULL, "*", "#root", XMLAttributes());
	root->SetId(name);
	root->SetOffset(Vector2f(0, 0), NULL);
	root->SetProperty(Z_INDEX, "0");

	Element* element = Factory::InstanceElement(NULL, "body", "body", XMLAttributes());
	cursor_proxy = dynamic_cast< ElementDocument* >(element);
	if (cursor_proxy == NULL)
	{
		if (element != NULL)
			element->RemoveReference();
	}
	else
		cursor_proxy->context = this;

	document_focus_history.push_back(root);
	focus = root;

	show_cursor = true;

	drag_started = false;
	drag_verbose = false;
	drag_clone = NULL;

	last_click_element = NULL;
	last_click_time = 0;
}

Context::~Context()
{
	PluginRegistry::NotifyContextDestroy(this);

	UnloadAllDocuments();
	UnloadAllMouseCursors();

	ReleaseUnloadedDocuments();

	if (cursor_proxy != NULL)
		cursor_proxy->RemoveReference();

	if (root != NULL)
		root->RemoveReference();

	if (instancer)
		instancer->RemoveReference();

	if (render_interface)
		render_interface->RemoveReference();
}

// Returns the name of the context.
const String& Context::GetName() const
{
	return name;
}

// Changes the dimensions of the screen.
void Context::SetDimensions(const Vector2i& _dimensions)
{
	if (dimensions != _dimensions)
	{
		dimensions = _dimensions;
		root->SetBox(Box(Vector2f((float) dimensions.x, (float) dimensions.y)));
		root->DirtyLayout();

		for (int i = 0; i < root->GetNumChildren(); ++i)
		{
			ElementDocument* document = root->GetChild(i)->GetOwnerDocument();
			if (document != NULL)
			{
				document->DirtyLayout();
				document->UpdatePosition();
			}
		}
		
		clip_dimensions = dimensions;
	}
}

// Returns the dimensions of the screen.
const Vector2i& Context::GetDimensions() const
{
	return dimensions;
}

// Updates all elements in the element tree.
bool Context::Update()
{
	root->Update();

	// Release any documents that were unloaded during the update.
	ReleaseUnloadedDocuments();

	return true;
}

// Renders all visible elements in the element tree.
bool Context::Render()
{
	RenderInterface* render_interface = GetRenderInterface();
	if (render_interface == NULL)
		return false;

	// Update the layout for all documents in the root. This is done now as events during the
	// update may have caused elements to require an update.
	for (int i = 0; i < root->GetNumChildren(); ++i)
		root->GetChild(i)->UpdateLayout();

	render_interface->context = this;
	ElementUtilities::ApplyActiveClipRegion(this, render_interface);

	root->Render();

	ElementUtilities::SetClippingRegion(NULL, this);

	// Render the cursor proxy so any elements attached the cursor will be rendered below the cursor.
	if (cursor_proxy != NULL)
	{
		cursor_proxy->Update();
		cursor_proxy->SetOffset(Vector2f((float) Math::Clamp(mouse_position.x, 0, dimensions.x),
													 (float) Math::Clamp(mouse_position.y, 0, dimensions.y)),
								NULL);
		cursor_proxy->Render();
	}

	// Render the cursor document if we have one and we're showing the cursor.
	if (active_cursor &&
		show_cursor)
	{
		active_cursor->Update();
		active_cursor->SetOffset(Vector2f((float) Math::Clamp(mouse_position.x, 0, dimensions.x),
													 (float) Math::Clamp(mouse_position.y, 0, dimensions.y)),
								 NULL);
		active_cursor->Render();
	}

	render_interface->context = NULL;

	return true;
}

// Creates a new, empty document and places it into this context.
ElementDocument* Context::CreateDocument(const String& tag)
{
	Element* element = Factory::InstanceElement(NULL, tag, "body", XMLAttributes());
	if (element == NULL)
	{
		Log::Message(Log::LT_ERROR, "Failed to instance document on tag '%s', instancer returned NULL.", tag.CString());
		return NULL;
	}

	ElementDocument* document = dynamic_cast< ElementDocument* >(element);
	if (document == NULL)
	{
		Log::Message(Log::LT_ERROR, "Failed to instance document on tag '%s', Found type '%s', was expecting derivative of ElementDocument.", tag.CString(), typeid(element).name());

		element->RemoveReference();
		return NULL;
	}

	document->context = this;
	root->AppendChild(document);

	PluginRegistry::NotifyDocumentLoad(document);

	return document;
}

// Load a document into the context.
ElementDocument* Context::LoadDocument(const String& document_path)
{	
	// Open the stream based on the file path
	StreamFile* stream = new StreamFile();
	if (!stream->Open(document_path))
	{
		stream->RemoveReference();
		return NULL;
	}

	// Load the document from the stream
	ElementDocument* document = LoadDocument(stream);

	stream->RemoveReference();

	return document;
}

// Load a document into the context.
ElementDocument* Context::LoadDocument(Stream* stream)
{
	PluginRegistry::NotifyDocumentOpen(this, stream->GetSourceURL().GetURL());

	// Load the document from the stream.
	ElementDocument* document = Factory::InstanceDocumentStream(this, stream);
	if (!document)
		return NULL;

	root->AppendChild(document);

	// Bind the events, run the layout and fire the 'onload' event.
	ElementUtilities::BindEventAttributes(document);
	document->UpdateLayout();

	// Dispatch the load notifications.
	PluginRegistry::NotifyDocumentLoad(document);
	document->DispatchEvent(LOAD, Dictionary(), false);

	return document;
}

// Load a document into the context.
ElementDocument* Context::LoadDocumentFromMemory(const String& string)
{
	// Open the stream based on the string contents.
	StreamMemory* stream = new StreamMemory((byte*)string.CString(), string.Length());
	stream->SetSourceURL("[document from memory]");

	// Load the document from the stream.
	ElementDocument* document = LoadDocument(stream);

	stream->RemoveReference();

	return document;
}

// Unload the given document
void Context::UnloadDocument(ElementDocument* _document)
{
	// Has this document already been unloaded?
	for (size_t i = 0; i < unloaded_documents.size(); ++i)
	{
		if (unloaded_documents[i] == _document)
			return;
	}

	// Add a reference, to ensure the document isn't released
	// while we're closing it.
	unloaded_documents.push_back(_document);
	ElementDocument* document = _document;

	if (document->GetParentNode() == root)
	{
		// Dispatch the unload notifications.
		document->DispatchEvent(UNLOAD, Dictionary(), false);
		PluginRegistry::NotifyDocumentUnload(document);

		// Remove the document from the context.
		root->RemoveChild(document);
	}

	// Remove the item from the focus history.
	ElementList::iterator itr = std::find(document_focus_history.begin(), document_focus_history.end(), document);
	if (itr != document_focus_history.end())
		document_focus_history.erase(itr);

	// Focus to the previous document if the old document is the current focus.
	if (focus && focus->GetOwnerDocument() == document)
	{
		focus = NULL;
		document_focus_history.back()->GetFocusLeafNode()->Focus();
	}

	// Clear the active element if the old document is the active element.
	if (active && active->GetOwnerDocument() == document)
	{
		active = NULL;
	}

	// Rebuild the hover state.
	UpdateHoverChain(Dictionary(), Dictionary(), mouse_position);
}

// Unload all the currently loaded documents
void Context::UnloadAllDocuments()
{
	// Unload all children.
	while (root->GetNumChildren(true) > 0)
		UnloadDocument(root->GetChild(0)->GetOwnerDocument());

	// Force cleanup of child elements now, reference counts must hit zero so that python (if it's in use) cleans up
	// before we exit this method.
	root->active_children.clear();
	root->ReleaseElements(root->deleted_children);
}

// Adds a previously-loaded cursor document as a mouse cursor within this context.
void Context::AddMouseCursor(ElementDocument* cursor_document)
{
	cursor_document->AddReference();

	CursorMap::iterator i = cursors.find(cursor_document->GetTitle());
	if (i != cursors.end())
	{
		if (active_cursor == (*i).second)
			active_cursor = cursor_document;

		if (default_cursor == (*i).second)
			default_cursor = cursor_document;

		(*i).second->RemoveReference();
	}
	cursors[cursor_document->GetTitle()] = cursor_document;

	if (!default_cursor)
	{
		default_cursor = cursor_document;
		active_cursor = cursor_document;
	}
}

// Loads a document as a mouse cursor.
ElementDocument* Context::LoadMouseCursor(const String& document_path)
{
	StreamFile* stream = new StreamFile();
	if (!stream->Open(document_path))
		return NULL;

	// Load the document from the stream.
	ElementDocument* document = Factory::InstanceDocumentStream(this, stream);
	if (document == NULL)
		return NULL;

	AddMouseCursor(document);

	// Bind the events, run the layout and fire the 'onload' event.
	ElementUtilities::BindEventAttributes(document);
	document->UpdateLayout();
	document->DispatchEvent(LOAD, Dictionary(), false);

	return document;
}

// Unload the given cursor.
void Context::UnloadMouseCursor(const String& cursor_name)
{
	CursorMap::iterator i = cursors.find(cursor_name);
	if (i != cursors.end())
	{
		if (default_cursor == (*i).second)
			default_cursor = NULL;

		if (active_cursor == (*i).second)
			active_cursor = default_cursor;

		(*i).second->RemoveReference();
		cursors.erase(i);
	}
}

// Unloads all currently loaded cursors.
void Context::UnloadAllMouseCursors()
{
	while (!cursors.empty())
		UnloadMouseCursor((*cursors.begin()).first.CString());
}

// Sets a cursor as the active cursor.
bool Context::SetMouseCursor(const String& cursor_name)
{
	CursorMap::iterator i = cursors.find(cursor_name);
	if (i == cursors.end())
	{
		active_cursor = default_cursor;
		Log::Message(Log::LT_WARNING, "Failed to find cursor '%s' in context '%s', reverting to default cursor.", cursor_name.CString(), name.CString());
		return false;
	}

	active_cursor = (*i).second;
	return true;
}

// Shows or hides the cursor.
void Context::ShowMouseCursor(bool show)
{
	show_cursor = show;
}

// Returns the first document found in the root with the given id.
ElementDocument* Context::GetDocument(const String& id)
{
	for (int i = 0; i < root->GetNumChildren(); i++)
	{
		ElementDocument* document = root->GetChild(i)->GetOwnerDocument();
		if (document == NULL)
			continue;

		if (document->GetId() == id)
			return document;
	}

	return NULL;
}

// Returns a document in the context by index.
ElementDocument* Context::GetDocument(int index)
{
	Element* element = root->GetChild(index);
	if (element == NULL)
		return NULL;

	return element->GetOwnerDocument();
}

// Returns the number of documents in the context.
int Context::GetNumDocuments() const
{
	return root->GetNumChildren();
}

// Returns the hover element.
Element* Context::GetHoverElement()
{
	return *hover;
}

// Returns the focus element.
Element* Context::GetFocusElement()
{
	return *focus;
}

// Returns the root element.
Element* Context::GetRootElement()
{
	return root;
}

// Brings the document to the front of the document stack.
void Context::PullDocumentToFront(ElementDocument* document)
{
	if (document != root->GetLastChild())
	{
		// Calling RemoveChild() / AppendChild() would be cleaner, but that dirties the document's layout
		// unnecessarily, so we'll go under the hood here.
		for (int i = 0; i < root->GetNumChildren(); ++i)
		{
			if (root->GetChild(i) == document)
			{
				root->children.erase(root->children.begin() + i);
				root->children.insert(root->children.begin() + root->GetNumChildren(), document);

				root->DirtyStackingContext();
			}
		}
	}
}

// Sends the document to the back of the document stack.
void Context::PushDocumentToBack(ElementDocument* document)
{
	if (document != root->GetFirstChild())
	{
		// See PullDocumentToFront().
		for (int i = 0; i < root->GetNumChildren(); ++i)
		{
			if (root->GetChild(i) == document)
			{
				root->children.erase(root->children.begin() + i);
				root->children.insert(root->children.begin(), document);

				root->DirtyStackingContext();
			}
		}
	}
}

// Adds an event listener to the root element.
void Context::AddEventListener(const String& event, EventListener* listener, bool in_capture_phase)
{
	root->AddEventListener(event, listener, in_capture_phase);
}

// Removes an event listener from the root element.
void Context::RemoveEventListener(const String& event, EventListener* listener, bool in_capture_phase)
{
	root->RemoveEventListener(event, listener, in_capture_phase);
}

// Sends a key down event into Rocket.
bool Context::ProcessKeyDown(Input::KeyIdentifier key_identifier, int key_modifier_state)
{
	// Generate the parameters for the key event.
	Dictionary parameters;
	GenerateKeyEventParameters(parameters, key_identifier);
	GenerateKeyModifierEventParameters(parameters, key_modifier_state);

	if (focus)
		return focus->DispatchEvent(KEYDOWN, parameters, true);
	else
		return root->DispatchEvent(KEYDOWN, parameters, true);
}

// Sends a key up event into Rocket.
bool Context::ProcessKeyUp(Input::KeyIdentifier key_identifier, int key_modifier_state)
{
	// Generate the parameters for the key event.
	Dictionary parameters;
	GenerateKeyEventParameters(parameters, key_identifier);
	GenerateKeyModifierEventParameters(parameters, key_modifier_state);

	if (focus)
		return focus->DispatchEvent(KEYUP, parameters, true);
	else
		return root->DispatchEvent(KEYUP, parameters, true);
}

// Sends a single character of text as text input into Rocket.
bool Context::ProcessTextInput(word character)
{
	// Generate the parameters for the key event.
	Dictionary parameters;
	parameters.Set("data", character);

	if (focus)
		return focus->DispatchEvent(TEXTINPUT, parameters, true);
	else
		return root->DispatchEvent(TEXTINPUT, parameters, true);
}

// Sends a string of text as text input into Rocket.
bool Context::ProcessTextInput(const String& string)
{
	bool consumed = true;

	for (size_t i = 0; i < string.Length(); ++i)
	{
		// Generate the parameters for the key event.
		Dictionary parameters;
		parameters.Set("data", string[i]);

		if (focus)
			consumed = focus->DispatchEvent(TEXTINPUT, parameters, true) && consumed;
		else
			consumed = root->DispatchEvent(TEXTINPUT, parameters, true) && consumed;
	}

	return consumed;
}

// Sends a mouse movement event into Rocket.
void Context::ProcessMouseMove(int x, int y, int key_modifier_state)
{
	// Check whether the mouse moved since the last event came through.
	Vector2i old_mouse_position = mouse_position;
	bool mouse_moved = (x != mouse_position.x) || (y != mouse_position.y);
	if (mouse_moved)
	{
		mouse_position.x = x;
		mouse_position.y = y;
	}

	// Generate the parameters for the mouse events (there could be a few!).
	Dictionary parameters;
	GenerateMouseEventParameters(parameters, -1);
	GenerateKeyModifierEventParameters(parameters, key_modifier_state);

	Dictionary drag_parameters;
	GenerateMouseEventParameters(drag_parameters);
	GenerateDragEventParameters(drag_parameters);
	GenerateKeyModifierEventParameters(drag_parameters, key_modifier_state);

	// Update the current hover chain. This will send all necessary 'onmouseout', 'onmouseover', 'ondragout' and
	// 'ondragover' messages.
	UpdateHoverChain(parameters, drag_parameters, old_mouse_position);

	// Dispatch any 'onmousemove' events.
	if (mouse_moved)
	{
		if (hover)
		{
			hover->DispatchEvent(MOUSEMOVE, parameters, true);

			if (drag_hover &&
				drag_verbose)
				drag_hover->DispatchEvent(DRAGMOVE, drag_parameters, true);
		}
	}
}
	
static Element* FindFocusElement(Element* element)
{
	ElementDocument* owner_document = element->GetOwnerDocument();
	if (!owner_document || owner_document->GetProperty< int >(FOCUS) == FOCUS_NONE)
		return NULL;
	
	while (element && element->GetProperty< int >(FOCUS) == FOCUS_NONE)
	{
		element = element->GetParentNode();
	}
	
	return element;
}

// Sends a mouse-button down event into Rocket.
void Context::ProcessMouseButtonDown(int button_index, int key_modifier_state)
{
	Dictionary parameters;
	GenerateMouseEventParameters(parameters, button_index);
	GenerateKeyModifierEventParameters(parameters, key_modifier_state);

	if (button_index == 0)
	{
		Element* new_focus = *hover;
		
		// Set the currently hovered element to focus if it isn't already the focus.
		if (hover)
		{
			new_focus = FindFocusElement(*hover);
			if (new_focus && new_focus != *focus)
			{
				if (!new_focus->Focus())
					return;
			}
		}

		// Save the just-pressed-on element as the pressed element.
		active = new_focus;

		bool propogate = true;
		
		// Call 'onmousedown' on every item in the hover chain, and copy the hover chain to the active chain.
		if (hover)
			propogate = hover->DispatchEvent(MOUSEDOWN, parameters, true);

		if (propogate)
		{
			// Check for a double-click on an element; if one has occured, we send the 'dblclick' event to the hover
			// element. If not, we'll start a timer to catch the next one.
			float click_time = GetSystemInterface()->GetElapsedTime();
			if (active == last_click_element &&
				click_time - last_click_time < DOUBLE_CLICK_TIME)
			{
				if (hover)
					propogate = hover->DispatchEvent(DBLCLICK, parameters, true);

				last_click_element = NULL;
				last_click_time = 0;
			}
			else
			{
				last_click_element = *active;
				last_click_time = click_time;
			
			}
		}

		for (ElementSet::iterator itr = hover_chain.begin(); itr != hover_chain.end(); ++itr)
			active_chain.push_back((*itr));

		if (propogate)
		{
			// Traverse down the hierarchy of the newly focussed element (if any), and see if we can begin dragging it.
			drag_started = false;
			drag = hover;
			while (drag)
			{
				int drag_style = drag->GetProperty(DRAG)->value.Get< int >();
				switch (drag_style)
				{
					case DRAG_NONE:		drag = drag->GetParentNode(); continue;
					case DRAG_BLOCK:	drag = NULL; continue;
					default:			drag_verbose = (drag_style == DRAG_DRAG_DROP || drag_style == DRAG_CLONE);
				}

				break;
			}
		}
	}
	else
	{
		// Not the primary mouse button, so we're not doing any special processing.
		if (hover)
			hover->DispatchEvent(MOUSEDOWN, parameters, true);
	}
}

// Sends a mouse-button up event into Rocket.
void Context::ProcessMouseButtonUp(int button_index, int key_modifier_state)
{
	Dictionary parameters;
	GenerateMouseEventParameters(parameters, button_index);
	GenerateKeyModifierEventParameters(parameters, key_modifier_state);

	// Process primary click.
	if (button_index == 0)
	{
		// The elements in the new hover chain have the 'onmouseup' event called on them.
		if (hover)
			hover->DispatchEvent(MOUSEUP, parameters, true);

		// If the active element (the one that was being hovered over when the mouse button was pressed) is still being
		// hovered over, we click it.
		if (hover && active && active == FindFocusElement(*hover))
		{
			active->DispatchEvent(CLICK, parameters, true);
		}

		// Unset the 'active' pseudo-class on all the elements in the active chain; because they may not necessarily
		// have had 'onmouseup' called on them, we can't guarantee this has happened already.
		std::for_each(active_chain.begin(), active_chain.end(), PseudoClassFunctor("active", false));
		active_chain.clear();

		if (drag)
		{
			if (drag_started)
			{
				Dictionary drag_parameters;
				GenerateMouseEventParameters(drag_parameters);
				GenerateDragEventParameters(drag_parameters);
				GenerateKeyModifierEventParameters(drag_parameters, key_modifier_state);

				if (drag_hover)
				{
					if (drag_verbose)
					{
						drag_hover->DispatchEvent(DRAGDROP, drag_parameters, true);
						drag_hover->DispatchEvent(DRAGOUT, drag_parameters, true);
					}
				}

				drag->DispatchEvent(DRAGEND, drag_parameters, true);

				ReleaseDragClone();
			}

			drag = NULL;
			drag_hover = NULL;
			drag_hover_chain.clear();
		}
	}
	else
	{
		// Not the left mouse button, so we're not doing any special processing.
		if (hover)
			hover->DispatchEvent(MOUSEUP, parameters, true);
	}
}

// Sends a mouse-wheel movement event into Rocket.
bool Context::ProcessMouseWheel(int wheel_delta, int key_modifier_state)
{
	if (hover)
	{
		Dictionary scroll_parameters;
		GenerateKeyModifierEventParameters(scroll_parameters, key_modifier_state);
		scroll_parameters.Set("wheel_delta", wheel_delta);

		return hover->DispatchEvent(MOUSESCROLL, scroll_parameters, true);
	}

	return true;
}

// Gets the context's render interface.
RenderInterface* Context::GetRenderInterface() const
{
	return render_interface;
}
	
// Gets the current clipping region for the render traversal
bool Context::GetActiveClipRegion(Vector2i& origin, Vector2i& dimensions) const
{
	if (clip_dimensions.x < 0 || clip_dimensions.y < 0)
		return false;
	
	origin = clip_origin;
	dimensions = clip_dimensions;
	
	return true;
}
	
// Sets the current clipping region for the render traversal
void Context::SetActiveClipRegion(const Vector2i& origin, const Vector2i& dimensions)
{
	clip_origin = origin;
	clip_dimensions = dimensions;
}

// Sets the instancer to use for releasing this object.
void Context::SetInstancer(ContextInstancer* _instancer)
{
	ROCKET_ASSERT(instancer == NULL);
	instancer = _instancer;
	instancer->AddReference();	
}

// Internal callback for when an element is removed from the hierarchy.
void Context::OnElementRemove(Element* element)
{
	ElementSet::iterator i = hover_chain.find(element);
	if (i == hover_chain.end())
		return;

	ElementSet old_hover_chain = hover_chain;
	hover_chain.erase(i);

	Element* hover_element = element;
	while (hover_element != NULL)
	{
		Element* next_hover_element = NULL;

		// Look for a child on this element's children that is also hovered.
		for (int j = 0; j < hover_element->GetNumChildren(true); ++j)
		{
			// Is this child also in the hover chain?
			Element* hover_child_element = hover_element->GetChild(j);
			ElementSet::iterator k = hover_chain.find(hover_child_element);
			if (k != hover_chain.end())
			{
				next_hover_element = hover_child_element;
				hover_chain.erase(k);

				break;
			}
		}

		hover_element = next_hover_element;
	}

	Dictionary parameters;
	GenerateMouseEventParameters(parameters, -1);
	SendEvents(old_hover_chain, hover_chain, MOUSEOUT, parameters, true);
}

// Internal callback for when a new element gains focus
bool Context::OnFocusChange(Element* new_focus)
{
	ElementSet old_chain;
	ElementSet new_chain;

	Element* old_focus = *(focus);
	ElementDocument* old_document = old_focus ? old_focus->GetOwnerDocument() : NULL;
	ElementDocument* new_document = new_focus->GetOwnerDocument();

	// If the current focus is modal and the new focus is not modal, deny the request
	if (old_document && old_document->IsModal() && (!new_document || !new_document->GetOwnerDocument()->IsModal()))
		return false;

	// Build the old chains
	Element* element = old_focus;
	while (element)
	{
		old_chain.insert(element);
		element = element->GetParentNode();
	}

	// Build the new chain
	element = new_focus;
	while (element)
	{
		new_chain.insert(element);
		element = element->GetParentNode();
	}

	Dictionary parameters;

	// Send out blur/focus events.
	SendEvents(old_chain, new_chain, BLUR, parameters, false);
	SendEvents(new_chain, old_chain, FOCUS, parameters, false);

	focus = new_focus;

	// Raise the element's document to the front, if desired.
	ElementDocument* document = focus->GetOwnerDocument();
	if (document != NULL)
	{
		const Property* z_index_property = document->GetProperty(Z_INDEX);
		if (z_index_property->unit == Property::KEYWORD &&
			z_index_property->value.Get< int >() == Z_INDEX_AUTO)
			document->PullToFront();
	}

	// Update the focus history
	if (old_document != new_document)
	{
		// If documents have changed, add the new document to the end of the history
		ElementList::iterator itr = std::find(document_focus_history.begin(), document_focus_history.end(), new_document);
		if (itr != document_focus_history.end())
			document_focus_history.erase(itr);

		if (new_document != NULL)
			document_focus_history.push_back(new_document);
	}

	return true;
}

// Generates an event for faking clicks on an element.
void Context::GenerateClickEvent(Element* element)
{
	Dictionary parameters;
	GenerateMouseEventParameters(parameters, 0);

	element->DispatchEvent(CLICK, parameters, true);
}

// Updates the current hover elements, sending required events.
void Context::UpdateHoverChain(const Dictionary& parameters, const Dictionary& drag_parameters, const Vector2i& old_mouse_position)
{
	Vector2f position((float) mouse_position.x, (float) mouse_position.y);

	// Send out drag events.
	if (drag)
	{
		if (mouse_position != old_mouse_position)
		{
			if (!drag_started)
			{
				Dictionary drag_start_parameters = drag_parameters;
				drag_start_parameters.Set("mouse_x", old_mouse_position.x);
				drag_start_parameters.Set("mouse_y", old_mouse_position.y);
				drag->DispatchEvent(DRAGSTART, drag_start_parameters);
				drag_started = true;

				if (drag->GetProperty< int >(DRAG) == DRAG_CLONE)
				{
					// Clone the element and attach it to the mouse cursor.
					CreateDragClone(*drag);
				}
			}

			drag->DispatchEvent(DRAG, drag_parameters);
		}
	}

	hover = GetElementAtPoint(position);

	if (!hover ||
		hover->GetProperty(CURSOR)->unit == Property::KEYWORD)
		active_cursor = default_cursor;
	else
		SetMouseCursor(hover->GetProperty< String >(CURSOR));

	// Build the new hover chain.
	ElementSet new_hover_chain;
	Element* element = *hover;
	while (element != NULL)
	{
		new_hover_chain.insert(element);
		element = element->GetParentNode();
	}

	// Send mouseout / mouseover events.
	SendEvents(hover_chain, new_hover_chain, MOUSEOUT, parameters, true);
	SendEvents(new_hover_chain, hover_chain, MOUSEOVER, parameters, true);

	// Send out drag events.
	if (drag)
	{
		drag_hover = GetElementAtPoint(position, *drag);

		ElementSet new_drag_hover_chain;
		element = *drag_hover;
		while (element != NULL)
		{
			new_drag_hover_chain.insert(element);
			element = element->GetParentNode();
		}

/*		if (mouse_moved && !drag_started)
		{
			drag->DispatchEvent(DRAGSTART, drag_parameters);
			drag_started = true;

			if (drag->GetProperty< int >(DRAG) == DRAG_CLONE)
			{
				// Clone the element and attach it to the mouse cursor.
				CreateDragClone(*drag);
			}
		}*/

		if (drag_started &&
			drag_verbose)
		{
			// Send out ondragover and ondragout events as appropriate.
			SendEvents(drag_hover_chain, new_drag_hover_chain, DRAGOUT, drag_parameters, true);
			SendEvents(new_drag_hover_chain, drag_hover_chain, DRAGOVER, drag_parameters, true);
		}

		drag_hover_chain.swap(new_drag_hover_chain);
	}

	// Swap the new chain in.
	hover_chain.swap(new_hover_chain);
}

// Returns the youngest descendent of the given element which is under the given point in screen coodinates.
Element* Context::GetElementAtPoint(const Vector2f& point, const Element* ignore_element, Element* element)
{
	// Update the layout on all documents prior to this call.
	for (int i = 0; i < GetNumDocuments(); ++i)
		GetDocument(i)->UpdateLayout();

	if (element == NULL)
	{
		if (ignore_element == root)
			return NULL;

		element = root;
	}

	// Check if any documents have modal focus; if so, only check down than document.
	if (element == root)
	{
		if (focus)
		{
			ElementDocument* focus_document = focus->GetOwnerDocument();
			if (focus_document != NULL &&
				focus_document->IsModal())
			{
				element = focus_document;
			}
		}
	}

	// Check any elements within our stacking context. We want to return the lowest-down element
	// that is under the cursor.
	if (element->local_stacking_context)
	{
		if (element->stacking_context_dirty)
			element->BuildLocalStackingContext();

		for (int i = (int) element->stacking_context.size() - 1; i >= 0; --i)
		{
			if (ignore_element != NULL)
			{
				Element* element_hierarchy = element->stacking_context[i];
				while (element_hierarchy != NULL)
				{
					if (element_hierarchy == ignore_element)
						break;

					element_hierarchy = element_hierarchy->GetParentNode();
				}

				if (element_hierarchy != NULL)
					continue;
			}

			Element* child_element = GetElementAtPoint(point, ignore_element, element->stacking_context[i]);
			if (child_element != NULL)
				return child_element;
		}
	}

	// Check if the point is actually within this element.
	bool within_element = element->IsPointWithinElement(point);
	if (within_element)
	{
		Vector2i clip_origin, clip_dimensions;
		if (ElementUtilities::GetClippingRegion(clip_origin, clip_dimensions, element))
		{
			within_element = point.x >= clip_origin.x &&
							 point.y >= clip_origin.y &&
							 point.x <= (clip_origin.x + clip_dimensions.x) &&
							 point.y <= (clip_origin.y + clip_dimensions.y);
		}
	}

	if (within_element)
		return element;

	return NULL;
}

// Creates the drag clone from the given element.
void Context::CreateDragClone(Element* element)
{
	if (cursor_proxy == NULL)
	{
		Log::Message(Log::LT_ERROR, "Unable to create drag clone, no cursor proxy document.");
		return;
	}

	ReleaseDragClone();

	// Instance the drag clone.
	drag_clone = element->Clone();
	if (drag_clone == NULL)
	{
		Log::Message(Log::LT_ERROR, "Unable to duplicate drag clone.");
		return;
	}

	// Append the clone to the cursor proxy element.
	cursor_proxy->AppendChild(drag_clone);
	drag_clone->RemoveReference();

	// Set the style sheet on the cursor proxy.
	cursor_proxy->SetStyleSheet(element->GetStyleSheet());

	// Set all the required properties and pseudo-classes on the clone.
	drag_clone->SetPseudoClass("drag", true);
	drag_clone->SetProperty("position", "absolute");
	drag_clone->SetProperty("left", Property(element->GetAbsoluteLeft() - element->GetBox().GetEdge(Box::MARGIN, Box::LEFT) - mouse_position.x, Property::PX));
	drag_clone->SetProperty("top", Property(element->GetAbsoluteTop() - element->GetBox().GetEdge(Box::MARGIN, Box::TOP) - mouse_position.y, Property::PX));
}

// Releases the drag clone, if one exists.
void Context::ReleaseDragClone()
{
	if (drag_clone != NULL)
	{
		cursor_proxy->RemoveChild(drag_clone);
		drag_clone = NULL;
	}
}

// Builds the parameters for a generic key event.
void Context::GenerateKeyEventParameters(Dictionary& parameters, Input::KeyIdentifier key_identifier)
{
	parameters.Set("key_identifier", (int) key_identifier);
}

// Builds the parameters for a generic mouse event.
void Context::GenerateMouseEventParameters(Dictionary& parameters, int button_index)
{
	parameters.Set("mouse_x", mouse_position.x);
	parameters.Set("mouse_y", mouse_position.y);
	if (button_index >= 0)
		parameters.Set("button", button_index);
}

// Builds the parameters for the key modifier state.
void Context::GenerateKeyModifierEventParameters(Dictionary& parameters, int key_modifier_state)
{
	static String property_names[] = {
		"ctrl_key",
		"shift_key",
		"alt_key",
		"meta_key",
		"caps_lock_key",
		"num_lock_key",
		"scroll_lock_key"
	};

	for (int i = 0; i < 7; i++)
		parameters.Set(property_names[i], (int) ((key_modifier_state & (1 << i)) > 0));
}

// Builds the parameters for a drag event.
void Context::GenerateDragEventParameters(Dictionary& parameters)
{	
	parameters.Set("drag_element", (void*) *drag);
}

// Releases all unloaded documents pending destruction.
void Context::ReleaseUnloadedDocuments()
{
	if (!unloaded_documents.empty())
	{
		ElementList documents = unloaded_documents;
		unloaded_documents.clear();

		// Clear the deleted list.
		for (size_t i = 0; i < documents.size(); ++i)
			documents[i]->GetEventDispatcher()->DetachAllEvents();
		documents.clear();
	}
}

// Sends the specified event to all elements in new_items that don't appear in old_items.
void Context::SendEvents(const ElementSet& old_items, const ElementSet& new_items, const String& event, const Dictionary& parameters, bool interruptible)
{
	ElementList elements;
	std::set_difference(old_items.begin(), old_items.end(), new_items.begin(), new_items.end(), std::back_inserter(elements));
	std::for_each(elements.begin(), elements.end(), RKTEventFunctor(event, parameters, interruptible));
}

void Context::OnReferenceDeactivate()
{
	if (instancer != NULL)
	{
		instancer->ReleaseContext(this);
	}
}

}
}
