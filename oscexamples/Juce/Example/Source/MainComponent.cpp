/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible (display);
    
    oscReceiver.addListener (this);
    if (oscReceiver.connect (8000) == false)
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     "Connection Error !",
                                     "Could not open port 9876");
    
    setSize (680, 420);
}

MainComponent::~MainComponent()
{
    oscReceiver.disconnect();
}

void MainComponent::resized()
{
    display.setBounds (getLocalBounds());
}

//==============================================================================
void MainComponent::oscMessageReceived (const OSCMessage& message)
{
    MessageManager::callAsync ([this, m = message](){display.setPose (m);}); //pop to message thread
}
