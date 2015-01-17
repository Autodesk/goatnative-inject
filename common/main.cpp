//
//  main.cpp
//  Injector
//
//  Created by Max Raskin on 1/17/15.
//  Copyright (c) 2015 Max Raskin. All rights reserved.
//

#include <iostream>
#include <memory>
#include <string>

#include "Injector/goatnative/Injector.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;

using goatnative::Injector;

class IConcurrency
{
public:
    virtual void createMutex() = 0;
    virtual ~IConcurrency() = default;
};

class IFileSystem
{
public:
    virtual void writeFile(const string& fileName) = 0;
    virtual ~IFileSystem() = default;
};

class INotifier
{
public:
    virtual void notify(const string& message, const string& target) = 0;
    virtual ~INotifier() = default;
};

class Concurrency : public IConcurrency
{
public:
    void createMutex() override
    {
        cout << "Creating mutex" << endl;
    }
};

class FileSystem : public IFileSystem
{
public:
    void writeFile(const string& fileName) override
    {
        cout << "Writing" << fileName << endl;
    }
};

class Notifier : public INotifier
{
public:
    void notify(const string& message, const string& target) override
    {
        cout << "Notifying " << target << " with message: " << message << endl;
    }
    
};

class ServicesProvider
{
public:
    ServicesProvider(shared_ptr<IConcurrency> concurency,
                     shared_ptr<IFileSystem> fileSystem,
                     shared_ptr<INotifier> notifier)
                : _concurency(concurency), _fileSystem(fileSystem), _notifier(notifier)
    {
        
    }
    
    shared_ptr<IConcurrency> concurrency()
    {
        return _concurency;
    }
    
    shared_ptr<IFileSystem> filesystem()
    {
        return _fileSystem;
    }
    
    shared_ptr<INotifier> notifier()
    {
        return _notifier;
    }
    
private:
    shared_ptr<IConcurrency> _concurency;
    shared_ptr<IFileSystem> _fileSystem;
    shared_ptr<INotifier> _notifier;
};

////////////////////////////////////////////

template<typename T>
struct type { static void id() { } };

// Custom type info method that uses size_t and not string for type name -
// map lookups should go faster than if we would have used RTTI's typeid<T>().name() which returns a string
// as key.
// Taken from http://codereview.stackexchange.com/questions/44936/unique-type-id-in-c
template<typename T>
size_t type_id() { return reinterpret_cast<size_t>(&type<T>::id); }


void testSingleton()
{
    Injector injector;
    
    injector.registerSingleton<Notifier>();
    injector.registerSingletonInterface<INotifier, Notifier>();
    auto notifier = injector.getInstance<INotifier>();
    auto notifier2 = injector.getInstance<INotifier>();
    
    assert(notifier == notifier2);
    
}

void testBuildWholeGraph()
{
    
    Injector injector;
    
    injector.registerSingleton<Notifier>();
    injector.registerSingletonInterface<INotifier, Notifier>();
    
    injector.registerClass<Concurrency>();
    injector.registerSingletonInterface<IConcurrency, Concurrency>();
    
    injector.registerClass<FileSystem>();
    injector.registerSingletonInterface<IFileSystem, FileSystem>();
    
    injector.registerClass<ServicesProvider, IConcurrency, IFileSystem, INotifier>();
    
    auto services = injector.getInstance<ServicesProvider>();

    assert(services->notifier());
    assert(services->filesystem());
    assert(services->concurrency());
    
    assert(services->notifier().get() == injector.getInstance<INotifier>().get());
    assert(services->concurrency().get() == injector.getInstance<IConcurrency>().get());
    assert(services->filesystem().get() == injector.getInstance<IFileSystem>().get());
}

void testFactory()
{
    Injector injector;
    
    injector.registerClass<Notifier>();
    assert(injector.getInstance<Notifier>().get() != injector.getInstance<Notifier>().get());
}

int main(int argc, const char * argv[]) {
    // TODO - move tests to Google Test
    testSingleton();
    testBuildWholeGraph();
    testFactory();

    return 0;
}


