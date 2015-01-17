//
//  Injector.h
//  Injector
//
//  Created by Max Raskin on 1/17/15.
//  Copyright (c) 2015 Max Raskin. All rights reserved.
//

#ifndef Injector_Injector_h
#define Injector_Injector_h

#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>

namespace goatnative
{
    using std::shared_ptr;
    using std::make_shared;
    using std::function;
    using std::pair;
    using std::unordered_map;
    using std::size_t;

    //
    // Injector
    // Inversion of Control (IoC) container.
    // Let's you create a type-safe mapping of class hierarchies by injecting constructor arguments.
    //
    // Idea and code based upon:
    // http://www.codeproject.com/Articles/567981/AnplusIOCplusContainerplususingplusVariadicplusTem
    //
    // Optimized out usage of RTTI to get type info - see comments near type_id below.
    //
    // Usage example:
    // ==============
    // Injector injector;
    //
    // injector.registerSingleton<Child2>();
    // auto inst3 = make_shared<Child3>();
    // injector.registerInstance<Child3>(inst3);
    // injector.registerClass<Child1>();
    //
    // injector.registerClass<Parent, Child1, Child2, Child3>();
    //
    // auto parent = injector<Parent>.getInstance();
    //
    // TODO - (1) add method for factory interface registration i.e each call to
    //            getInstance<IInteface>() will return a new instance.
    //        (2) thread safety.
    //
    class Injector
    {
    public:
        template <typename T, typename... Dependencies>
        Injector& registerClass()
        {
            auto creator = [this]() -> T*
            {
                return new T(getInstance<Dependencies>()...);
            };
            
            _typesToCreators.insert(pair<size_t, function<void*()>>{type_id<T>(), creator} );
            
            return *this;
        }
        
        template <typename T>
        Injector& registerInstance(shared_ptr<T> instance)
        {
            shared_ptr<IHolder> holder = shared_ptr<Holder<T>>{new Holder<T>{instance}};
            
            _typesToInstances.insert(pair<size_t, shared_ptr<IHolder>>{type_id<T>(), holder});
            
            return *this;
        }
        
        template <typename T, typename... Dependencies>
        Injector& registerSingleton()
        {
            auto instance = make_shared<T>(getInstance<Dependencies>()...);
            
            return registerInstance<T>(instance);
        }
        
        template <typename Interface, typename RegisteredConcreteClass>
        Injector& registerSingletonInterface()
        {
            auto instance = getInstance<RegisteredConcreteClass>();
            
            shared_ptr<IHolder> holder =
                shared_ptr<Holder<Interface>>{new Holder<Interface>{instance}};
            
            _typesToInstances.insert(pair<size_t, shared_ptr<IHolder>>{type_id<Interface>(), holder});
            return *this;
        }
        
        template <typename T>
        shared_ptr<T> getInstance()
        {
            // Try getting registered singleton or instance.
            if (_typesToInstances.find(type_id<T>()) != _typesToInstances.end())
            {
                // get as reference to avoid refcount increment
                auto& iholder =  _typesToInstances[type_id<T>()];
                
                auto holder = dynamic_cast<Holder<T>*>(iholder.get());
                return holder->_instance;
            } // Otherwise attempt getting the creator and act as factory.
            else if (_typesToCreators.find(type_id<T>()) != _typesToCreators.end())
            {
                auto& creator = _typesToCreators[type_id<T>()];
                
                return shared_ptr<T>{(T*)creator()};
            }
            
            // If you debug, in some debuggers (e.g Apple's lldb in Xcode) it will breakpoint in this assert
            // and by looking in the stack trace you'll be able to see which class you forgot to map.
            assert(false && "One of your injected dependencies isn't mapped, please check your mappings.");
            
            return nullptr;
        }
        
    private:

        struct IHolder
        {
            virtual ~IHolder() = default;
        };
        
        template <typename T>
        struct Holder : public IHolder
        {
            Holder(shared_ptr<T> instance) : _instance(instance)
            {}

            shared_ptr<T> _instance;
        };
        
        template<typename T>
        struct type { static void id() { } };
        
        // Custom type info method that uses size_t and not string for type name -
        // map lookups should go faster than if we would have used RTTI's typeid<T>().name() which returns a string
        // as key.
        // Taken from http://codereview.stackexchange.com/questions/44936/unique-type-id-in-c
        template<typename T>
        size_t type_id() { return reinterpret_cast<size_t>(&type<T>::id); }
        
        // Holds instances - keeps singletons and custom registered instances
        unordered_map<size_t, shared_ptr<IHolder>> _typesToInstances;
        // Holds creators used to instansiate a type
        unordered_map<size_t, function<void*()>> _typesToCreators;

    };
} // namespace goatnative

#endif
