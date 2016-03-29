# goatnative-inject
C++11 Dependency Injection (IoC - inversion of control) class using variadic templates and shared pointers.
This ought to give a good example of variadic templates usage and other C++11 features such as auto and shared_ptr.

The implementation behind the goatnative::Inject is taken from www.codeproject.com/Articles/567981/AnplusIOCplusContainerplususingplusVariadicplusTem
I've extended that implementation by:

* Adding a registerInterface method that lets you map interfaces to previously registered instance classes.

## Features
* Register types to their dependencies.
* Register as singleton.
* Register interfaces to to concrete types.
* Thread safe.

## Limits
* Currently injection is done only via constructor.
* Injectee constructors are expected to have shared_ptr to each injected types

## Examples

### Example - Hello world

```cpp
#include "Injector/goatnative/Injector.h"
#include <iostream>

using goatnative::Injector;

class IHello
{
public:
    virtual void hello() const = 0;
    virtual ~IHello() = default;
};


class Hello : public IHello
{
public:
    virtual void hello() const
    {
        std::cout << "hello world!" << std::endl;
    }
};


int main(int argc, const char** argv)
{
    Injector injector;
    
    // Let's register the concrete class
    injector.registerClass<Hello>();
    
    // Now register IHello with Hello so that each time we ask for IHello 
    // We get Hello -
    injector.registerInterface<IHello, Hello>();
    
    auto helloInstance = injector.getInstance<IHello>();
    
    helloInstance->hello();
}
```

### Example - Singleton Class
```cpp
#include <cassert>
#include <functional>

#include "Injector/goatnative/Injector.h"

using goatnative::Injector;

class IExecutor
{
public:
    virtual void execute(std::function<void()> func) = 0;
    virtual ~IExecutor() = default;
};

class SyncExecutor : public IExecutor
{
public:
    void execute(std::function<void()> func) override
    {
        func();
    }
};

int main(int argc, const char * argv[]) {
    Injector injector;
    
    // Here we're registering IExecutor with SyncExecutor as singleton
    injector.registerSingleton<SyncExecutor>();
    injector.registerInterface<IExecutor, SyncExecutor>();
    
    // Whenever we'll ask for IExecutor we'll get the same instance of SyncExecutor
    auto executor1 = injector.getInstance<IExecutor>();
    auto executor2 = injector.getInstance<IExecutor>();
    
    // Compare returned pointer which are expected to be the same
    assert(executor1.get() == executor2.get());
    return 0;
}
```
### Example - map to an existing instance
```cpp
#include <cassert>
#include <memory>

#include "Injector/goatnative/Injector.h"

using goatnative::Injector;

class NullObject
{
};

int main(int argc, const char * argv[]) {
    Injector injector;

    // Here's an example of how to register a manually created instance of an object
    auto nullInstance = std::make_shared<NullObject>();
    
    injector.registerInstance(nullInstance);
    
    auto null1 = injector.getInstance<NullObject>();
    auto null2 = injector.getInstance<NullObject>();
    
    assert(null1.get() == null2.get());
    assert(null1.get() == nullInstance.get());
    return 0;
}
```
    
### Example - class with dependencies
```cpp
#include <iostream>
#include <memory>
#include <string>

#include "Injector/goatnative/Injector.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;

using goatnative::Injector;

class ICar
{
public:
    virtual void startIgnition() = 0;
    virtual ~ICar() = default;
};

class IEngine
{
public:
    virtual double getVolume() const = 0;
    virtual ~IEngine() = default;
};

class IWheels
{
public:
    virtual bool inflated() const = 0;
    virtual ~IWheels() = default;
};

class Engine : public IEngine
{
public:
    virtual double getVolume() const override
    {
        return 10.5;
    }
};

class Wheels : public IWheels
{
public:
    virtual bool inflated() const override
    {
        return false;
    }
};

class Car : public ICar
{
public:
    Car(shared_ptr<IEngine> engine, shared_ptr<IWheels> wheels) : _engine(engine), _wheels(wheels)
    {
        
    }
    
    virtual void startIgnition()
    {
        std::cout << "starting ignition, engine volume: " << _engine->getVolume()
                  << ", wheels inflated? " << _wheels->inflated() << std::endl;
    }

private:
    shared_ptr<IEngine> _engine;
    shared_ptr<IWheels> _wheels;
};


int main(int argc, const char * argv[]) {
    Injector injector;
    
    // Here's an example of a more complex graph of objects tied together
    // using Injector -
    
    // Engine
    injector.registerClass<Engine>();
    injector.registerInterface<IEngine, Engine>();
    
    // Wheels
    injector.registerClass<Wheels>();
    injector.registerInterface<IWheels, Wheels>();
    
    // Register car with its dependencies - IEngine and IWheels (required by its ctor)
    injector.registerClass<Car, IEngine, IWheels>();
    injector.registerInterface<ICar, Car>();
    
    // Car is instansiated with instances of Engine, Wheels and Car automatically!
    auto car = injector.getInstance<Car>();
    car->startIgnition();
    
    return 0;
}
```

* See more examples in main.cpp

## TODO
* Move tests from main.cpp to Google Test.
