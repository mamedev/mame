.. Sol documentation master file, created by
   sphinx-quickstart on Mon Feb 29 21:49:51 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. image:: sol.png
	:target: https://github.com/ThePhD/sol2
	:alt: sol2 repository

Sol |version|
=============
a fast, simple C++ and Lua Binding
----------------------------------

When you need to hit the ground running with Lua and C++, `Sol`_ is the go-to framework for high-performance binding with an easy to use API.

.. image:: https://travis-ci.org/ThePhD/sol2.svg?branch=develop
	:target: https://travis-ci.org/ThePhD/sol2
	:alt: build status

.. image:: https://badges.gitter.im/chat-sol2/Lobby.svg
	:target: https://gitter.im/chat-sol2/Lobby
	:alt: chat about sol2 on gitter


get going:
----------

.. toctree::
	:maxdepth: 1
	:name: mastertoc
	
	tutorial/all-the-things
	tutorial/tutorial-top
	errors
	compilation
	features
	usertypes
	traits
	api/api-top
	mentions
	benchmarks
	performance
	safety
	exceptions
	rtti
	codecvt
	cmake
	licenses
	origin


"I need feature X, maybe you have it?"
--------------------------------------
Take a look at the :doc:`Features<features>` page: it links to much of the API. You can also just straight up browse the :doc:`api<api/api-top>` or ease in with the :doc:`tutorials<tutorial/tutorial-top>`. To know more about the implementation for usertypes, see :doc:`here<usertypes>` To know how function arguments are handled, see :ref:`this note<function-argument-handling>`. Don't see a feature you want? Send inquiries for support for a particular abstraction to the `issues`_ tracker.


the basics:
-----------

.. note::
	More examples can be found in the `examples directory`_


.. code-block:: c++
	:caption: functions
	:linenos:

	#include <sol.hpp>
	#include <cassert>

	int main() {
		sol::state lua;
		int x = 0;
		lua.set_function("beep", [&x]{ ++x; });
		lua.script("beep()");
		assert(x == 1);

		sol::function beep = lua["beep"];
		beep();
		assert(x == 2);

		return 0;
	}


.. code-block:: c++
	:caption: linking C++ structures to Lua
	:linenos:

	#include <sol.hpp>
	#include <cassert>

	struct vars {
		int boop = 0;

		int bop () const {
			return boop + 1;
		}
	};

	int main() {
		sol::state lua;
		lua.new_usertype<vars>("vars", 
			"boop", &vars::boop
			"bop", &vars::bop);
		lua.script("beep = vars.new()\n"
			"beep.boop = 1\n"
			"bopvalue = beep:bop()");

		vars& beep = lua["beep"];
		int bopvalue = lua["bopvalue"];

		assert(beep.boop == 1);
		assert(lua.get<vars>("beep").boop == 1);
		assert(beep.bop() == 2);
		assert(bopvalue == 2);

		return 0;
	}			



Indices and tables
==================

* :ref:`genindex`
* :ref:`search`

.. _Sol: https://github.com/ThePhD/sol2
.. _issues: https://github.com/ThePhD/sol2/issues
.. _examples directory: https://github.com/ThePhD/sol2/tree/develop/examples