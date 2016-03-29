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
#include <mutex>
#include <typeinfo>

namespace goatnative
{
    using std::shared_ptr;
    using std::make_shared;
    using std::function;
    using std::pair;
    using std::unordered_map;
    using std::size_t;
    using std::recursive_mutex;
    using std::lock_guard;
    
    //
    // Injector
    // Inversion of Control (IoC) container.
    // Let's you create a type-safe mapping of class hierarchies by injecting constructor arguments.
    //
    // Idea and code based upon:
    // http://www.codeproject.com/Articles/567981/AnplusIOCplusContainerplususingplusVariadicplusTem
    //
    //
    // See usage examples @ https://github.com/GoatHunter/goatnative-inject
    class Injector
    {
    public:
        template <typename T, typename... Dependencies>
        Injector& registerClass()
        {
            lock_guard<recursive_mutex> lockGuard{ _mutex };
            
            auto creator = [this]() -> T*
            {
                return new T(getInstance<Dependencies>()...);
            };
            
            _typesToCreators.insert(pair<size_t, function<void*()>>{typeid(T).hash_code(), creator} );
            
            return *this;
        }
        
        template <typename T>
        Injector& registerInstance(shared_ptr<T> instance)
        {
            lock_guard<recursive_mutex> lockGuard{ _mutex };
            
            shared_ptr<IHolder> holder = shared_ptr<Holder<T>>{new Holder<T>{instance}};
            
            _typesToInstances.insert(pair<size_t, shared_ptr<IHolder>>{typeid(T).hash_code(), holder});
            
            return *this;
        }
        
        template <typename T, typename... Dependencies>
        Injector& registerSingleton()
        {
            lock_guard<recursive_mutex> lockGuard{ _mutex };
            
            auto instance = make_shared<T>(getInstance<Dependencies>()...);
            
            return registerInstance<T>(instance);
        }
        
        template <typename Interface, typename RegisteredConcreteClass>
        Injector& registerInterface()
        {
            lock_guard<recursive_mutex> lockGuard{ _mutex };
            
            auto instanceGetter = [this]() -> shared_ptr<IHolder>
            {
                auto instance = getInstance<RegisteredConcreteClass>();
                shared_ptr<IHolder> holder = shared_ptr<Holder<Interface>>{new Holder<Interface>{ instance }};
                
                return holder;
            };
            
            _interfacesToInstanceGetters.insert(pair<size_t, function<shared_ptr<IHolder>()>>{typeid(Interface).hash_code(), instanceGetter});
            
            return *this;
        }
        
        
        template <typename T>
        shared_ptr<T> getInstance()
        {
            lock_guard<recursive_mutex> lockGuard{ _mutex };
            
            // Try getting registered singleton or instance.
            if (_typesToInstances.find(typeid(T).hash_code()) != _typesToInstances.end())
            {
                // get as reference to avoid refcount increment
                auto& iholder =  _typesToInstances[typeid(T).hash_code()];
                
                auto holder = dynamic_cast<Holder<T>*>(iholder.get());
                return holder->_instance;
            } // Otherwise attempt getting the creator and act as factory.
            else if (_typesToCreators.find(typeid(T).hash_code()) != _typesToCreators.end())
            {
                auto& creator = _typesToCreators[typeid(T).hash_code()];
                
                return shared_ptr<T>{(T*)creator()};
            }
            else if (_interfacesToInstanceGetters.find(typeid(T).hash_code()) != _interfacesToInstanceGetters.end())
            {
                auto& instanceGetter = _interfacesToInstanceGetters[typeid(T).hash_code()];
                
                auto iholder = instanceGetter();
                
                auto holder = dynamic_cast<Holder<T>*>(iholder.get());
                return holder->_instance;
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
        
        // Holds instances - keeps singletons and custom registered instances
        unordered_map<size_t, shared_ptr<IHolder>> _typesToInstances;
        // Holds creators used to instansiate a type
        unordered_map<size_t, function<void*()>> _typesToCreators;
        
        unordered_map<size_t, function<shared_ptr<IHolder>()>> _interfacesToInstanceGetters;
        
        recursive_mutex _mutex;
        
    };
} // namespace goatnative

#endif
