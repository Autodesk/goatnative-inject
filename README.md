# goatnative-inject
C++11 Dependency Injection (IoC - inversion of control) class using variadic templates and shared pointers.
This ought to give a good example of variadic templates usage and other C++11 features such as auto and shared_ptr.

The implementation behind the goatnative::Inject is taken from www.codeproject.com/Articles/567981/AnplusIOCplusContainerplususingplusVariadicplusTem
I've extended that implementation by:

* Adding a registerInterface method that lets you map interfaces to previously registered instance classes.
* Avoided usage of RTTI to identify types, found an interesting approach in http://codereview.stackexchange.com/questions/44936/unique-type-id-in-c 
  which lets you get the type efficiently into a size_t type (instead of typeid(T).string() which returns an std::string).

## Features
* Register types to their dependencies.
* Register as singleton.
* Register interfaces to to concrete types.

## Limits
* Currently injection is done only via constructor.
* Injectee constructors are expected to have shared_ptr to each injected types

## Examples
Injector injector;
// Assumption: Notifier has a ctor of Notifier(shared_ptr<InputData>)  
injector.registerSingleton<Notifier, InputData>(); // Notifier depends on InputData
injector.registerSingletonInterface<INotifier, Notifier>();

auto notifier = injector.getInstance&lt;INotifier&gt;();

// Use notifier

* See more examples in main.cpp

## TODO
* Currently, the registerSingletonInterface() method maps interfaces to instances, need to add a method for 
  factory interface registration which after invoking it will cause each call to getInstance<IInteface>() to return a new instance.
* Make thread safe using stl's concurrency.
* Move tests from main.cpp to Google Test.
