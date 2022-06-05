/*
  ==============================================================================

    AutoComponent.cpp
    Created: 30 May 2022 2:54:47pm
    Author:  (?) & StoneyDSP

  ==============================================================================
*/

#include "AutoComponent.h"

/*
  ==============================================================================

    Look and Feel.

  ==============================================================================
*/

AutoComponentLookAndFeel::AutoComponentLookAndFeel()
{
    //setColourScheme(LookAndFeel_V4::getMidnightColourScheme());
    /*LookAndFeel_V4::getLightColourScheme();
    LookAndFeel_V4::getDarkColourScheme();
    LookAndFeel_V4::getGreyColourScheme();
    LookAndFeel_V4::getMidnightColourScheme();*/

    ///* Knob style */
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::purple);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightgrey);
    //setColourScheme();
    //setColour(Slider::backgroundColourId, juce::Colours::brown);
    setColour(juce::Slider::thumbColourId, juce::Colours::pink);
    setColour(juce::Slider::trackColourId, juce::Colours::black);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::grey);
    setColour(juce::Slider::textBoxHighlightColourId, juce::Colours::blue);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lightgrey);

    //* Button style */
    setColour(juce::Button::buttonDown, juce::Colours::orangered);
    setColour(juce::Button::buttonNormal, juce::Colours::darkgrey);
    setColour(juce::Button::buttonOver, juce::Colours::lightslategrey);

    ///* Text Button style */
    //setColour(TextButton::buttonColourId, juce::Colours::grey);
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::orangered);
    setColour(juce::TextButton::buttonNormal, juce::Colours::darkgrey);
    setColour(juce::TextButton::buttonOver, juce::Colours::lightgrey);
    //setColour(TextButton::buttonDown, juce::Colours::green);
    //setColour(TextButton::textColourOnId, juce::Colours::white);
    //setColour(TextButton::textColourOffId, juce::Colours::transparentWhite);
}

/*
  ==============================================================================

    AutoComponent.

  ==============================================================================
*/

AutoComponent::AutoComponent(juce::AudioProcessor& p, APVTS& apvts, std::function<void()> paramLambda, std::function<juce::String(double)>&& apvtsLambda) : valueSupplier(std::move(apvtsLambda) )
{
    auto addSlider = [=, &apvts] (juce::AudioParameterFloat* param)
    {
        SliderWithAttachment* newSlide = new SliderWithAttachment;

        addAndMakeVisible (newSlide->slider);
        newSlide->attachment.reset (new SliderAttachment (apvts, param->paramID, newSlide->slider));

        auto suffix = " " + param->getLabel().fromLastOccurrenceOf ("_", false, false);
        newSlide->slider.setTextValueSuffix (suffix);

        newSlide->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        newSlide->slider.setName (param->name);
        newSlide->slider.textFromValueFunction = valueSupplier;
        //newSlide->slider.textFromValueFunction = nullptr; // @TODO: Don't override lambda from VTS
        newSlide->slider.setNumDecimalPlacesToDisplay (2);
        newSlide->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        newSlide->slider.onValueChange = paramLambda;

        sliders.add (newSlide);
    };

    auto addBox = [=, &apvts] (juce::AudioParameterChoice* param)
    {
        BoxWithAttachment* newBox = new BoxWithAttachment;

        addAndMakeVisible (newBox->box);

        newBox->box.setName (param->name);
        newBox->box.addItemList (param->choices, 1);
        newBox->box.setSelectedItemIndex (0);
        newBox->box.onChange = paramLambda;

        newBox->attachment.reset (new ComboBoxAttachment (apvts, param->paramID, newBox->box));

        boxes.add (newBox);
    };

    auto addButton = [=, &apvts] (juce::AudioParameterBool* param)
    {
        ButtonWithAttachment* newButton = new ButtonWithAttachment;

        addAndMakeVisible (newButton->button);

        newButton->button.setButtonText (param->name);
        newButton->button.setClickingTogglesState (true);
        newButton->button.onStateChange = paramLambda;

        newButton->attachment.reset (new ButtonAttachment (apvts, param->paramID, newButton->button));

        buttons.add (newButton);
    };

    auto params = p.getParameters();

    for (auto* param : params)
    {
        //======================================================================
        /** If = ParameterFloat, make new Rotary Slider with Attachment */

        if (auto* paramFloat = dynamic_cast<juce::AudioParameterFloat*> (param))
        {
            addSlider (paramFloat);
            continue;
        }

        //======================================================================
        /** If = ParameterChoice, make new Box with Attachment */

        if (auto* paramChoice = dynamic_cast<juce::AudioParameterChoice*> (param))
        {
            addBox (paramChoice);
            continue;
        }
        
        //======================================================================
        /** If = ParameterBool, make new Button with Attachment */

        if (auto* paramBool = dynamic_cast<juce::AudioParameterBool*> (param))
        {
            addButton (paramBool);
            continue;
        }
    }

    setSize (getWidth(), 100);
}

//==============================================================================

void AutoComponent::paint (juce::Graphics& g)
{
    //==========================================================================
    /** Paint Slider/Box name. */

    auto paintName = [this, &g] (juce::Component& comp, juce::String name)
    {
        const int height = 20;
        const int initialY = 2;
        juce::Rectangle<int> nameBox (comp.getX(), initialY, comp.getWidth(), height);
        g.setColour(juce::Colours::antiquewhite);
        g.setFont(15.0f);
        g.drawFittedText (name, nameBox, juce::Justification::centred, 1);
    };

    for (auto* s : sliders)
        paintName (s->slider, s->slider.getName());

    for (auto* b : boxes)
        paintName (b->box, b->box.getName());

    //==========================================================================
    /** Apply local Look and Feel. */

    auto applyLookAndFeel = [this, &g] (juce::Component& comp)
    {
        comp.setLookAndFeel(&lookAndfeel);
    };

    for (auto* s : sliders)
        applyLookAndFeel (s->slider);

    for (auto* b : boxes)
        applyLookAndFeel (b->box);

    for (auto* b : buttons)
        applyLookAndFeel(b->button);
}

//==============================================================================

void AutoComponent::resized()
{
    //==========================================================================
    /** This is generally where you'll want to lay out the positions of any
    /** subcomponents in your editor... */
    
    int x = 5;
    bool first = true;

    for (auto* s : sliders)
    {
        int offset = first ? 0 : 20;
        s->slider.setBounds (x - offset, 15, 85, 80);
        x = s->slider.getRight();
        first = false;
    }

    for (auto* b : boxes)
    {
        int offset = first ? 0 : 0;
        b->box.setBounds (x - offset, 40, 70, 20);
        x = b->box.getRight();
        first = false;
    }

    for (auto* b : buttons)
    {
        int offset = first ? 0 : 0;
        b->button.setBounds (x - offset, 40, 70, 20);
        x = b->button.getRight();
        first = false;
    }
}
//==============================================================================
