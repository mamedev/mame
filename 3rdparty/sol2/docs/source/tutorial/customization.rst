adding your own types
=====================

Sometimes, overriding Sol to make it handle certain ``struct``'s and ``class``'es as something other than just userdata is desirable. The way to do this is to take advantage of the 4 customization points for Sol. These are ``sol::lua_size<T>``, ``sol::stack::pusher<T, C>``, ``sol::stack::getter<T, C>``, ``sol::stack::checker<T, sol::type t,  C>``.

These are template class/structs, so you'll override them using a technique C++ calls *class/struct specialization*. Below is an example of a struct that gets broken apart into 2 pieces when going in the C++ --> Lua direction, and then pulled back into a struct when going in the Lua --> C++:

.. code-block:: cpp
	:caption: two_things.hpp
	:name: customization-overriding

	#include <sol.hpp>

	struct two_things {
		int a;
		bool b;
	};

	namespace sol {

		// First, the expected size
		// Specialization of a struct
		// We expect 2, so use 2
		template <>
		struct lua_size<two_things> : std::integral_constant<int, 2> {};

		// Now, specialize various stack structures
		namespace stack {

			template <>
			struct checker<two_things> {
				template <typename Handler>
				static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
					// indices can be negative to count backwards from the top of the stack,
					// rather than the bottom up
					// to deal with this, we adjust the index to
					// its absolute position using the lua_absindex function 
					int absolute_index = lua_absindex(L, index);
					// Check first and second second index for being the proper types
					bool success = stack::check<int>(L, absolute_index - 1, handler) 
						&& stack::check<bool>(L, absolute_index, handler);
					tracking.use(2);
					return success;
				}
			};

			template <>
			struct getter<two_things> {
				static two_things get(lua_State* L, int index, record& tracking) {
					int absolute_index = lua_absindex(L, index);
					// Get the first element
					int a = stack::get<int>(L, absolute_index - 1);
					// Get the second element, 
					// in the +1 position from the first
					bool b = stack::get<bool>(L, absolute_index);
					// we use 2 slots, each of the previous takes 1
					tracking.use(2);
					return two_things{ a, b };
				}
			};

			template <>
			struct pusher<two_things> {
				static int push(lua_State* L, const two_things& things) {
					int amount = stack::push(L, things.a);
					// amount will be 1: int pushes 1 item
					amount += stack::push(L, things.b);
					// amount 2 now, since bool pushes a single item
					// Return 2 things
					return amount;
				}
			};

		}
	}


This is the base formula that you can follow to extend to your own classes. Using it in the rest of the library should then be seamless:

.. code-block:: cpp
	:caption: customization: using it
	:name: customization-using

	#include <sol.hpp>
	#include <two_things.hpp>

	int main () {

		sol::state lua;

		// Create a pass-through style of function
		lua.script("function f ( a, b ) return a, b end");

		// get the function out of Lua
		sol::function f = lua["f"];

		two_things things = f(two_things{24, true});
		// things.a == 24
		// things.b == true
				
		return 0;
	}


And that's it!

A few things of note about the implementation: First, there's an auxiliary parameter of type :doc:`sol::stack::record<../api/stack>` for the getters and checkers. This keeps track of what the last complete operation performed. Since we retrieved 2 members, we use ``tracking.use(2);`` to indicate that we used 2 stack positions (one for ``bool``, one for ``int``). The second thing to note here is that we made sure to use the ``index`` parameter, and then proceeded to add 1 to it for the next one.

You can make something pushable into Lua, but not get-able in the same way if you only specialize one part of the system. If you need to retrieve it (as a return using one or multiple values from Lua), you should specialize the ``sol::stack::getter`` template class and the ``sol::stack::checker`` template class. If you need to push it into Lua at some point, then you'll want to specialize the ``sol::stack::pusher`` template class. The ``sol::lua_size`` template class trait needs to be specialized for both cases, unless it only pushes 1 item, in which case the default implementation will assume 1.

.. note::

	It is important to note here that the ``getter``, ``pusher`` and ``checker`` differentiate between a type ``T`` and a pointer to a type ``T*``. This means that if you want to work purely with, say, a ``T*`` handle that does not have the same semantics as just ``T``, you may need to specify checkers/getters/pushers for both ``T*`` and ``T``. The checkers for ``T*`` forward to the checkers for ``T``, but the getter for ``T*`` does not forward to the getter for ``T`` (e.g., because of ``int*`` not being quite the same as ``int``).

In general, this is fine since most getters/checkers only use 1 stack point. But, if you're doing more complex nested classes, it would be useful to use ``tracking.last`` to understand how many stack indices the last getter/checker operation did and increment it by ``index + tracking.last`` after using a ``stack::check<..>( L, index, tracking)`` call.

You can read more about the structs themselves :ref:`over on the API page for stack<extension_points>`, and if there's something that goes wrong or you have anymore questions, please feel free to drop a line on the Github Issues page or send an e-mail!