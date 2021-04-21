/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Display.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component,
                        private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //Component=====================================================================
    void resized() override;

private:
    //OSCReceiver::Listener=========================================================
    void oscMessageReceived (const OSCMessage& message) override;

    OSCReceiver oscReceiver;
    Display display;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
