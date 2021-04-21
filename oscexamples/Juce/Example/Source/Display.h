/*
  ==============================================================================

    Display.h
    Created: 5 Dec 2018 3:33:18pm
    Author:  Tom Mitchell

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


class Display : public Component,
                private Timer
{
public:
    Display();
    void setPose (const OSCMessage& message);
    
    //Component
    void paint (Graphics& g) override;
    
private:
    //Timer
    void timerCallback() override;
    
    std::mutex mutex;
    std::array<std::array<float, 63>, 2> hands;
};
