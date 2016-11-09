getting performance
===================
things to make Sol as fast as possible
--------------------------------------


As shown by the :doc:`benchmarks<benchmarks>`, Sol is very performant with its abstractions. However, in the case where you need every last drop of performance from Sol, a number of tips and API usage tricks will be documented here. PLEASE benchmark / profile your code before you start invoking these, as some of them trade in readability / clarity for performance.

* If you have a bound function call / bound member function that you are going to call in a very tight, performance-heavy loop, considering using :doc:`sol::c_call<api/c_call>`
* Be wary of passing by value / reference, and what it means by reading :ref:`this note<function-argument-handling>`.
* It is currently undocumented that usertypes will "inherit" member function / member variables from bound classes, mostly because the semantics are unclear and it is not the most performant (although it is flexible: you can register base classes after / whenever you want in relation to the derived class, provided that derived class has its bases listed). Specifying all member functions / member variables for the usertype constructor / ``new_usertype`` function call and not relying on base lookup will boost performance of member lookup
* Specifying base classes can make getting the usertype out of Sol a bit slower since we have to check and cast; if you know the exact type wherever you're retrieving it, considering not specifying the bases, retrieving the exact type from Sol, and then casting to a base type yourself
* Member variables can sometimes cost an extra lookup to occur within the Lua system (as mentioned :doc:`bottom of the usertype page<api/usertype>`); until we find out a safe way around this, member variables will always incur that extra lookup cost


That's it as far as different performance options are avilable to make Sol run faster. Again, please make sure to invoke these only when you know Sol is the bottleneck. If you find some form of the performance unacceptable to you, also feel free to open an issue at the github.