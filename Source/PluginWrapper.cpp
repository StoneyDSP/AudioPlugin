/*
  ==============================================================================

    PluginWrapper.cpp
    Created: 8 May 2022 9:38:17pm
    Author:  StoneyDSP

  ==============================================================================
*/

#include "PluginWrapper.h"
#include "PluginProcessor.h"

template <typename SampleType>
ProcessWrapper<SampleType>::ProcessWrapper(AudioPluginAudioProcessor& p, APVTS& apvts) : audioProcessor(p)
{
    osPtr = static_cast <juce::AudioParameterChoice*> (apvts.getParameter("osID"));
    outputPtr = static_cast <juce::AudioParameterFloat*> (apvts.getParameter("outputID"));
    mixPtr = static_cast <juce::AudioParameterFloat*> (apvts.getParameter("mixID"));
    bypassPtr = dynamic_cast <juce::AudioParameterBool*> (apvts.getParameter("bypassID"));

    jassert(osPtr != nullptr);
    jassert(outputPtr != nullptr);
    jassert(mixPtr != nullptr);
    jassert(bypassPtr != nullptr);

    auto osChannels = audioProcessor.getTotalNumInputChannels();

    auto osFilter = juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR;

    for (int i = 0; i < 5; ++i)
        overSample[i] = std::make_unique<juce::dsp::Oversampling<SampleType>>
        (osChannels, i, osFilter, true, false);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::setOversampling()
{
    curOS = (int)osPtr->getIndex();
    if (curOS != prevOS)
    {
        overSamplingFactor = 1 << curOS;
        prevOS = curOS;
        mixer.reset();
        output.reset();
    }
}

template <typename SampleType>
void ProcessWrapper<SampleType>::prepare(double sampleRate, int samplesPerBlock)
{
    overSamplingFactor = 1 << curOS;
    prevOS = curOS;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = audioProcessor.getTotalNumInputChannels();

    for (int i = 0; i < 5; ++i)
        overSample[i]->initProcessing(spec.maximumBlockSize);

    for (int i = 0; i < 5; ++i)
        overSample[i]->numChannels = (size_t)spec.numChannels;

    mixer.prepare(spec);
    output.prepare(spec);

    reset();
    update();
}

template <typename SampleType>
void ProcessWrapper<SampleType>::reset()
{
    mixer.reset();
    output.reset();

    for (int i = 0; i < 5; ++i)
        overSample[i]->numChannels = (size_t)spec.numChannels;

    for (int i = 0; i < 5; ++i)
        overSample[i]->reset();
}

template <typename SampleType>
void ProcessWrapper<SampleType>::update()
{
    setOversampling();

    //contextReplace.isBypassed = bypassPtr->get();

    mixer.setWetMixProportion(mixPtr->get() * 0.01f);

    output.setGainLinear(juce::Decibels::decibelsToGain(outputPtr->get()));
}

//==============================================================================
template <typename SampleType>
void ProcessWrapper<SampleType>::process(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    update();

    juce::dsp::AudioBlock<SampleType> block(buffer);

    mixer.pushDrySamples(block);

    juce::dsp::AudioBlock<SampleType> osBlock = overSample[curOS]->processSamplesUp(block);

    //auto& context = contextReplace(osBlock);

    auto context = juce::dsp::ProcessContextReplacing(osBlock);

    output.process(context);

    overSample[curOS]->processSamplesDown(block);

    mixer.mixWetSamples(block);
}

//==============================================================================
template class ProcessWrapper<float>;
template class ProcessWrapper<double>;