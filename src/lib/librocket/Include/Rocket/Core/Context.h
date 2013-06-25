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

#ifndef ROCKETCORECONTEXT_H
#define ROCKETCORECONTEXT_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/ElementReference.h>
#include <Rocket/Core/Input.h>
#include <Rocket/Core/String.h>
#include <Rocket/Core/ScriptInterface.h>

namespace Rocket {
namespace Core {

class Stream;
class Dictionary;

}
}

namespace Rocket {
namespace Core {

class ContextInstancer;
class ElementDocument;
class EventListener;
class RenderInterface;

/**
	A context for storing, rendering and processing RML documents. Multiple contexts can exist simultaneously.

	@author Peter Curry
 */

class ROCKETCORE_API Context : public ScriptInterface
{
public:
	/// Constructs a new, uninitialised context. This should not be called directly, use Core::CreateContext()
	/// instead.
	/// @param[in] name The name of the context.
	Context(const String& name);
	/// Destroys a context.
	virtual ~Context();

	/// Returns the name of the context.
	/// @return The context's name.
	const String& GetName() const;

	/// Changes the dimensions of the context.
	/// @param[in] dimensions The new dimensions of the context.
	void SetDimensions(const Vector2i& dimensions);
	/// Returns the dimensions of the context.
	/// @return The current dimensions of the context.
	const Vector2i& GetDimensions() const;

	/// Updates all elements in the context's documents.
	bool Update();
	/// Renders all visible elements in the context's documents.
	bool Render();

	/// Creates a new, empty document and places it into this context.
	/// @param[in] tag The document type to create.
	/// @return The new document, or NULL if no document could be created. The document is returned with a reference owned by the caller.
	ElementDocument* CreateDocument(const String& tag = "body");
	/// Load a document into the context.
	/// @param[in] document_path The path to the document to load.
	/// @return The loaded document, or NULL if no document was loaded. The document is returned with a reference owned by the caller.
	ElementDocument* LoadDocument(const String& document_path);
	/// Load a document into the context.
	/// @param[in] document_stream The opened stream, ready to read.
	/// @return The loaded document, or NULL if no document was loaded. The document is returned with a reference owned by the caller.
	ElementDocument* LoadDocument(Stream* document_stream);
	/// Load a document into the context.
	/// @param[in] string The string containing the document RML.
	/// @return The loaded document, or NULL if no document was loaded. The document is returned with a reference owned by the caller.
	ElementDocument* LoadDocumentFromMemory(const String& string);
	/// Unload the given document.
	/// @param[in] document The document to unload.
	void UnloadDocument(ElementDocument* document);
	/// Unloads all loaded documents.
	void UnloadAllDocuments();

	/// Adds a previously-loaded cursor document as a mouse cursor within this context. This allows you to share
	/// cursors between contexts.
	/// @param[in] cursor_document The document to add as a cursor into this context.
	void AddMouseCursor(ElementDocument* cursor_document);
	/// Loads a document as a mouse cursor within this context.
	/// @param[in] cursor_document_path The path to the document to load as a cursor.
	/// @return The loaded cursor document, or NULL if no document was loaded. The document is returned with a reference owned by the caller.
	ElementDocument* LoadMouseCursor(const String& cursor_document_path);
	/// Unload the given cursor.
	/// @param[in] cursor_name The name of cursor to unload.
	void UnloadMouseCursor(const String& cursor_name);
	/// Unloads all currently loaded cursors.
	void UnloadAllMouseCursors();
	/// Sets a cursor as the active cursor.
	/// @param[in] cursor_name The name of the cursor to activate.
	/// @return True if a cursor exists with the given name, false if not.
	bool SetMouseCursor(const String& cursor_name);
	/// Shows or hides the cursor.
	/// @param[in] show True to show the cursor, false to hide it.
	void ShowMouseCursor(bool show);

	/// Returns the first document in the context with the given id.
	/// @param[in] id The id of the desired document.
	/// @return The document (if it was found), or NULL if no document exists with the ID. The document is returned with a borrowed reference.
	ElementDocument* GetDocument(const String& id);
	/// Returns a document in the context by index.
	/// @param[in] index The index of the desired document.
	/// @return The document (if one exists with this index), or NULL if the index was invalid. The document is returned with a borrowed reference.
	ElementDocument* GetDocument(int index);
	/// Returns the number of documents in the context.
	/// @return The number of documents in the context.
	int GetNumDocuments() const;

	/// Returns the hover element.
	/// @return The element the mouse cursor is hovering over. The element is returned with a borrowed reference.
	Element* GetHoverElement();

	/// Returns the focus element.
	/// @return The element with input focus. The element is returned with a borrowed reference.
	Element* GetFocusElement();

	/// Returns the root element that holds all the documents
	/// @return The root element. The element is returned with a borrowed reference.
	Element* GetRootElement();

	/// Brings the document to the front of the document stack.
	/// @param[in] document The document to pull to the front of the stack.
	void PullDocumentToFront(ElementDocument* document);
	/// Sends the document to the back of the document stack.
	/// @param[in] document The document to push to the bottom of the stack.
	void PushDocumentToBack(ElementDocument* document);

	/// Adds an event listener to the context's root element.
	/// @param[in] event The name of the event to attach to.
	/// @param[in] listener Listener object to be attached.
	/// @param[in] in_capture_phase True if the listener is to be attached to the capture phase, false for the bubble phase.
	void AddEventListener(const String& event, EventListener* listener, bool in_capture_phase = false);
	/// Removes an event listener from the context's root element.
	/// @param[in] event The name of the event to detach from.
	/// @param[in] listener Listener object to be detached.
	/// @param[in] in_capture_phase True to detach from the capture phase, false from the bubble phase.
	void RemoveEventListener(const String& event, EventListener* listener, bool in_capture_phase = false);

	/// Sends a key down event into this context.
	/// @param[in] key_identifier The key pressed.
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool ProcessKeyDown(Input::KeyIdentifier key_identifier, int key_modifier_state);
	/// Sends a key up event into this context.
	/// @param[in] key_identifier The key released.
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool ProcessKeyUp(Input::KeyIdentifier key_identifier, int key_modifier_state);

	/// Sends a single character of text as text input into this context.
	/// @param[in] character The UCS-2 character to send into this context.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool ProcessTextInput(word character);
	/// Sends a string of text as text input into this context.
	/// @param[in] string The UCS-2 string to send into this context.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool ProcessTextInput(const String& string);

	/// Sends a mouse movement event into this context.
	/// @param[in] x The x-coordinate of the mouse cursor, in window-coordinates (ie, 0 should be the left of the client area).
	/// @param[in] y The y-coordinate of the mouse cursor, in window-coordinates (ie, 0 should be the top of the client area).
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	void ProcessMouseMove(int x, int y, int key_modifier_state);
	/// Sends a mouse-button down event into this context.
	/// @param[in] button_index The index of the button that was pressed; 0 for the left button, 1 for right, and any others from 2 onwards.
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	void ProcessMouseButtonDown(int button_index, int key_modifier_state);
	/// Sends a mouse-button up event into this context.
	/// @param[in] button_index The index of the button that was release; 0 for the left button, 1 for right, and any others from 2 onwards.
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	void ProcessMouseButtonUp(int button_index, int key_modifier_state);
	/// Sends a mouse-wheel movement event into this context.
	/// @param[in] wheel_delta The mouse-wheel movement this frame. Rocket treats a negative delta as up movement (away from the user), positive as down.
	/// @param[in] key_modifier_state The state of key modifiers (shift, control, caps-lock, etc) keys; this should be generated by ORing together members of the Input::KeyModifier enumeration.
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool ProcessMouseWheel(int wheel_delta, int key_modifier_state);

	/// Gets the context's render interface.
	/// @return The render interface the context renders through.
	RenderInterface* GetRenderInterface() const;
	/// Gets the current clipping region for the render traversal
	/// @param[out] origin The clipping origin
	/// @param[out] dimensions The clipping dimensions
	bool GetActiveClipRegion(Vector2i& origin, Vector2i& dimensions) const;
	/// Sets the current clipping region for the render traversal
	/// @param[out] origin The clipping origin
	/// @param[out] dimensions The clipping dimensions
	void SetActiveClipRegion(const Vector2i& origin, const Vector2i& dimensions);

	/// Sets the instancer to use for releasing this object.
	/// @param[in] instancer The context's instancer.
	void SetInstancer(ContextInstancer* instancer);

protected:
	virtual void OnReferenceDeactivate();

private:
	String name;
	Vector2i dimensions;

	ContextInstancer* instancer;

	typedef std::set< ElementReference > ElementSet;
	typedef std::vector< ElementReference > ElementList;
	// Set of elements that are currently in hover state.
	ElementSet hover_chain;
	// List of elements that are currently in active state.
	ElementList active_chain;
	// History of windows that have had focus
	ElementList document_focus_history;

	// Documents that have been unloaded from the context but not yet released.
	ElementList unloaded_documents;

	// Root of the element tree.
	Element* root;
	// The element that current has input focus.
	ElementReference focus;
	// The top-most element being hovered over.
	ElementReference hover;
	// The element that was being hovered over when the primary mouse button was pressed most recently.
	ElementReference active;

	// The element that was clicked on last.
	Element* last_click_element;
	// The time the last click occured.
	float last_click_time;

	typedef std::map< String, ElementDocument* > CursorMap;
	CursorMap cursors;
	ElementReference default_cursor;
	ElementReference active_cursor;
	bool show_cursor;

	ElementDocument* cursor_proxy;

	// The element that is currently being dragged (or about to be dragged).
	ElementReference drag;
	// True if a drag has begun (ie, the ondragstart event has been fired for the drag element), false otherwise.
	bool drag_started;
	// True if the current drag is a verbose drag (ie, sends ondragover, ondragout, ondragdrop, etc, events).
	bool drag_verbose;
	// Used when dragging a cloned object.
	Element* drag_clone;

	// The element currently being dragged over; this is equivalent to hover, but only set while an element is being
	// dragged, and excludes the dragged element.
	ElementReference drag_hover;
	// Set of elements that are currently being dragged over; this differs from the hover state as the dragged element
	// itself can't be part of it.
	ElementSet drag_hover_chain;

	// Input state; stored from the most recent input events we receive from the application.
	Vector2i mouse_position;

	// The render interface this context renders through.
	RenderInterface* render_interface;
	Vector2i clip_origin;
	Vector2i clip_dimensions;

	// Internal callback for when an element is removed from the hierarchy.
	void OnElementRemove(Element* element);
	// Internal callback for when a new element gains focus.
	bool OnFocusChange(Element* element);

	// Generates an event for faking clicks on an element.
	void GenerateClickEvent(Element* element);

	// Updates the current hover elements, sending required events.
	void UpdateHoverChain(const Dictionary& parameters, const Dictionary& drag_parameters, const Vector2i& old_mouse_position);
	// Returns the youngest descendent of the given element which is under the given point in screen coordinates.
	// @param[in] point The point to test.
	// @param[in] ignore_element If set, this element and its descendents will be ignored.
	// @param[in] element Used internally.
	// @return The element under the point, or NULL if nothing is.
	Element* GetElementAtPoint(const Vector2f& point, const Element* ignore_element = NULL, Element* element = NULL);

	// Creates the drag clone from the given element. The old drag clone will be released if
	// necessary.
	// @param[in] element The element to clone.
	void CreateDragClone(Element* element);
	// Releases the drag clone, if one exists.
	void ReleaseDragClone();

	// Builds the parameters for a generic key event.
	void GenerateKeyEventParameters(Dictionary& parameters, Input::KeyIdentifier key_identifier);
	// Builds the parameters for a generic mouse event.
	void GenerateMouseEventParameters(Dictionary& parameters, int button_index = -1);
	// Builds the parameters for the key modifier state.
	void GenerateKeyModifierEventParameters(Dictionary& parameters, int key_modifier_state);
	// Builds the parameters for a drag event.
	void GenerateDragEventParameters(Dictionary& parameters);

	// Releases all unloaded documents pending destruction.
	void ReleaseUnloadedDocuments();

	// Sends the specified event to all elements in new_items that don't appear in old_items.
	static void SendEvents(const ElementSet& old_items, const ElementSet& new_items, const String& event, const Dictionary& parameters, bool interruptible);

	friend class Element;
	friend ROCKETCORE_API Context* CreateContext(const String&, const Vector2i&, RenderInterface*);
};

}
}

#endif
