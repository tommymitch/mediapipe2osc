//
//  Utils.hpp
//  OscppTest
//
//  Created by Tom Mitchell on 16/04/2021.
//  Copyright © 2021 Tom Mitchell. All rights reserved.
//

#pragma once

template <class ListenerClass>
class BasicListenerList
{
public:
    BasicListenerList() = default;
    ~BasicListenerList() = default;
    
    void add (ListenerClass* listenerToAdd)
    {
        if (listenerToAdd != nullptr && std::find (listeners.begin(), listeners.end(), listenerToAdd) == listeners.end())
            listeners.push_back (listenerToAdd);
    }
    void remove (ListenerClass* listenerToRemove)
    {
        auto it = std::find (listeners.begin(), listeners.end(), listenerToRemove);
        if(it != listeners.end())
            listeners.erase (it);
    }
    
    template <typename Callback>
    void call (Callback&& callback)
    {
        for (auto& l : listeners)
            callback (*l);
    }
    
private:
    std::vector<ListenerClass*> listeners;
};
