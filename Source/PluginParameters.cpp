/*
  ==============================================================================

    PluginParameters.cpp
    Created: 29 May 2022 7:58:00pm
    Author:  Nathan J. Hood

  ==============================================================================
*/

#include "PluginParameters.h"
#include "PluginProcessor.h"

Parameters::Parameters(AudioPluginAudioProcessor& p, APVTS& apvts) : audioProcessor(p), state(apvts)
{
}

void Parameters::setParameterLayout(Params& params)
{
    const auto dBMax = juce::Decibels::gainToDecibels(16.0f);
    const auto dBMin = juce::Decibels::gainToDecibels(0.0625f);
    const auto dBOut = juce::Decibels::gainToDecibels(0.5f, -120.0f) * 20.0f;

    const auto gainRange = juce::NormalisableRange<float>(dBMin, dBMax, 0.01f, 1.00f);
    const auto mixRange = juce::NormalisableRange<float>(00.00f, 100.00f, 0.01f, 1.00f);
    const auto outputRange = juce::NormalisableRange<float>(dBOut, dBMax, 0.01f, 1.00f);

    const auto osString = juce::StringArray({ "--", "2x", "4x", "8x", "16x" });
    const auto pString = juce::StringArray({ "floats", "doubles" });

    const auto decibels = juce::String { ( "dB" ) };
    const auto frequency = juce::String { ( "Hz" ) };
    const auto reso = juce::String { ( "q"  ) };
    const auto percentage = juce::String { ( "%"  ) };
    const auto milliseconds = juce::String { ( ".ms" ) };
    const auto inOut = juce::String { ("IO") };

    const auto genParam = juce::AudioProcessorParameter::genericParameter;
    const auto inMeter = juce::AudioProcessorParameter::inputMeter;
    const auto outParam = juce::AudioProcessorParameter::outputGain;
    const auto outMeter = juce::AudioProcessorParameter::outputMeter;
    
    juce::ignoreUnused(inMeter);
    juce::ignoreUnused(outParam);
    juce::ignoreUnused(outMeter);

    params.add
        //======================================================================
        (std::make_unique<juce::AudioProcessorParameterGroup>("masterID", "0", "seperator0",
            //==================================================================
            std::make_unique<juce::AudioParameterChoice>("osID", "Oversampling", osString, 0),
            std::make_unique<juce::AudioParameterFloat>("outputID", "Output", outputRange, 00.00f, decibels, genParam),
            std::make_unique<juce::AudioParameterFloat>("mixID", "Mix", mixRange, 100.00f, percentage, genParam),
            std::make_unique<juce::AudioParameterBool>("bypassID", "Bypass", false)
            //==================================================================
            ));
}

//==============================================================================
