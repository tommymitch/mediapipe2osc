/*
  ==============================================================================

    Display.cpp
    Created: 5 Dec 2018 3:33:18pm
    Author:  Tom Mitchell

  ==============================================================================
*/

#include "Display.h"
#include <algorithm>

Display::Display()
{
    startTimerHz (60);
    for (auto& h : hands)
        for (auto&f : h)
            f = 0.f;
}

void Display::paint (Graphics& g)
{
    auto width = getWidth();
    auto height = getHeight();
    
    std::array<std::array<float, 63>, 2> handsCopy;
    {
    std::lock_guard<std::mutex> lg(mutex);
    handsCopy = hands;
    }
    
    for (auto& h : handsCopy)
    {
        auto counter = 0;
        while (counter < h.size())
        {
            auto x = h[counter++];
            auto y = h[counter++];
            auto z = std::clamp (h[counter++] + 0.3f, 0.f, 1.f);
            const Rectangle<float> part ({0.f ,0.f },{10.f, 10.f});
            
            g.setColour (Colour (1.f - z, 1.f - z, 1.f - z, 1.f));
            g.fillEllipse (part.withCentre ({x * width, y * height}));
        }
    }
    
}

void Display::setPose (const OSCMessage& pose)
{
    const auto handIndex = pose.getAddressPattern() == "/left" ? 0 : 1;
    
    jassert (pose.size() == hands[handIndex].size());
    
    auto counter = 0;
    
    std::lock_guard<std::mutex> lg(mutex);
    for (auto a : pose)
    {
        jassert(a.isFloat32());
        hands[handIndex][counter++] = a.getFloat32();
    }
}

void Display::timerCallback()
{
    repaint();
}
