/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>
#include <iostream>

//==============================================================================
NEASynthesiserAudioProcessor::NEASynthesiserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    osc1(*this), osc2(*this), voiceArr(*this), 
    filter(*this),
    apvts(*this, nullptr, "parameters", createParameters())
{
}

NEASynthesiserAudioProcessor::~NEASynthesiserAudioProcessor()
{
}

//==============================================================================
const juce::String NEASynthesiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NEASynthesiserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NEASynthesiserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NEASynthesiserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NEASynthesiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NEASynthesiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NEASynthesiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NEASynthesiserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NEASynthesiserAudioProcessor::getProgramName (int index)
{
    return {};
}

void NEASynthesiserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NEASynthesiserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    this->sampleRate = sampleRate;
}

void NEASynthesiserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NEASynthesiserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NEASynthesiserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    //My code from below here--------------------------------------------------------------------------

    //retrieving values from the GUI elements:
        osc1.type = (Oscillator::OscillatorType) (apvts.getRawParameterValue("OSC1_TYPE")->load());
        
        osc1.volume = apvts.getRawParameterValue("OSC1_VOL")->load() * 0.25f;
        osc1.coarsePitch = apvts.getRawParameterValue("OSC1_CP")->load();
        osc1.finePitch = apvts.getRawParameterValue("OSC1_FP")->load();
        osc1.pan = apvts.getRawParameterValue("OSC1_PAN")->load();
        osc1.phaseOffset = apvts.getRawParameterValue("OSC1_PO")->load();


        //osc2.type = (Oscillator::OscillatorType)editor->osc2type.getSelectedId();
        osc2.type = (Oscillator::OscillatorType) (apvts.getRawParameterValue("OSC2_TYPE")->load());
        osc2.volume = apvts.getRawParameterValue("OSC2_VOL")->load() * 0.25f;
        osc2.coarsePitch = apvts.getRawParameterValue("OSC2_CP")->load();
        osc2.finePitch = apvts.getRawParameterValue("OSC2_FP")->load();
        osc2.pan = apvts.getRawParameterValue("OSC2_PAN")->load();
        osc2.phaseOffset = apvts.getRawParameterValue("OSC2_PO")->load();

        volumeEnv.attack = apvts.getRawParameterValue("VOL_ENV_ATTACK")->load() * sampleRate / 1000;
        volumeEnv.decay = apvts.getRawParameterValue("VOL_ENV_DECAY")->load() * sampleRate / 1000;
        volumeEnv.sustain = apvts.getRawParameterValue("VOL_ENV_SUSTAIN")->load();
        volumeEnv.release = apvts.getRawParameterValue("VOL_ENV_RELEASE")->load() * sampleRate / 1000;

        //filter.type = (FrequencyFilter::FilterType)editor->filterType.getSelectedId();
        filter.type = (FrequencyFilter::FilterType) (apvts.getRawParameterValue("FILTER_TYPE")->load());
        filter.centreFrequency = apvts.getRawParameterValue("FILTER_CF")->load();
        filter.resonance = apvts.getRawParameterValue("FILTER_RES")->load();

        filter.env.amount = apvts.getRawParameterValue("FILTER_ENV_AMOUNT")->load();
        filter.env.attack = apvts.getRawParameterValue("FILTER_ENV_ATTACK")->load() * sampleRate / 1000;
        filter.env.decay = apvts.getRawParameterValue("FILTER_ENV_DECAY")->load() * sampleRate / 1000;
        filter.env.sustain = apvts.getRawParameterValue("FILTER_ENV_SUSTAIN")->load();
        filter.env.release = apvts.getRawParameterValue("FILTER_ENV_RELEASE")->load() * sampleRate / 1000;

        lfo.destination = (LFO::DestinationType)apvts.getRawParameterValue("LFO_DEST")->load();
        lfo.amount = apvts.getRawParameterValue("LFO_AMOUNT")->load();
        lfo.rate = apvts.getRawParameterValue("LFO_RATE")->load();

    int timestamp = 0;          //this is when the note starts or ends, it will be passed to startSampleIndex

    //this loop updates the SynthVoiceArray based on the new midi messages that have been input
    for (auto meta : midiMessages) {
        auto msg = meta.getMessage();
        timestamp = meta.samplePosition;

        if (!msg.isNoteOnOrOff()) {
            //there are many kinds of midi messages, but the only ones that matter for this
            //solution are note on and note off messages. if there is any message that
            //is not one of these, then simply ignore it
            continue;
        }

        auto noteNumber = msg.getNoteNumber();
        auto velocity = msg.getFloatVelocity();

        //find the index of the voice in the SynthVoiceArray that is playing the
        //note given by the midi message msg
        int index = voiceArr.find(noteNumber);

        if (msg.isNoteOn()) {
            if (index == -1) {
                //if note is not contained in the array, then add the voice to the array
                voiceArr.addVoice(noteNumber, velocity, timestamp);
            }
            else {
                //if the note is contained in the array, then reset the voice
                voiceArr.resetVoice(index, velocity, timestamp);
            }
        }
        else {  //if the message is to turn the note off
            if (index == -1) {
                //if the note is not contained in the array, then do nothing
            }
            else {
                //if the note is contained in the array, turn off the voice
                voiceArr.turnOffVoice(index, timestamp);
            }
        }
    }

    //now generate audio
    auto output = voiceArr.generateAudio(buffer.getNumSamples());

    double lfoAngleDelta = juce::MathConstants<double>::twoPi * (lfo.rate / sampleRate) *
        (buffer.getNumSamples());

    filter.currentLFOAngle += lfoAngleDelta;

    //now write the audio into the buffer argument
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        //for this loop, i will assume that totalNumInputChannels == totalNumOutputChannels == 2
        auto* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            channelData[i] = output[channel][i];
        }
    }
}

//==============================================================================
bool NEASynthesiserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NEASynthesiserAudioProcessor::createEditor()
{
    return new NEASynthesiserAudioProcessorEditor (*this);
}

//==============================================================================
void NEASynthesiserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NEASynthesiserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NEASynthesiserAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout NEASynthesiserAudioProcessor::createParameters() 
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;    //this is the list of pointers to parameter objects

    //now we just create new parameter objects and add them to the list

    //Oscillator 1
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC1_TYPE", "Osc 1 Type",
        juce::StringArray({ "Sine", "Square", "Saw" }), 0));

    //params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1_TYPE", "Osc 1 Type", -0.5f, 0.5f, -0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1_VOL", "Osc 1 Volume", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC1_CP", "Osc 1 Coarse Pitch",
        -12, 12, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC1_FP", "Osc 1 Fine Pitch", -100,
        100, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1_PAN", "Osc 1 Pan", -1.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1_PO", "Osc 1 Phase Offset",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));

    //ajdkag

    //Oscillator 2
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC2_TYPE", "Osc 2 Type",
        juce::StringArray({ "Sine", "Square", "Saw" }), 0));

    //params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2_TYPE", "Osc 2 Type", -0.5f, 0.5f, -0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2_VOL", "Osc 2 Volume", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC2_CP", "Osc 2 Coarse Pitch",
        -12, 12, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC2_FP", "Osc 2 Fine Pitch", -100,
        100, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2_PAN", "Osc 2 Pan", -1.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2_PO", "Osc 2 Phase Offset",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));


    //Vol env
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOL_ENV_ATTACK", "Volume Envelope Attack",
        juce::NormalisableRange<float>(0.0f, 5000.0f, 0.f, 0.4), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOL_ENV_DECAY", "Volume Envelope Decay", 
        juce::NormalisableRange<float>(0.0f, 5000.0f, 0.f, 0.4), 5000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOL_ENV_SUSTAIN", "Volume Envelope Sustain", 
        0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOL_ENV_RELEASE", "Volume Envelope Release", 
        juce::NormalisableRange<float>(0.0f, 5000.0f, 0.f, 0.4), 0.0f));

    //Filter
    params.push_back(std::make_unique< juce::AudioParameterChoice>("FILTER_TYPE", "Filter Type",
        juce::StringArray({ "Low-pass", "High-pass" }), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_CF", "Filter Cutoff Frequency",
        juce::NormalisableRange<float>(40.0f, 20000.0f, 0.f, 0.23), 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_RES", "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.f, 0.3), 0.7071068f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_AMOUNT", "Filter Envelope Amount",
        juce::NormalisableRange<float>(-20000.0f, 20000.0f, 0.f, 0.3, true), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_ATTACK", "Filter Envelope Attack", 
        juce::NormalisableRange<float>(0.0f, 5000.0f, 0.f, 0.4), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_DECAY", "Filter Envelope Decay", 
        juce::NormalisableRange<float>(0.0f, 5000.0f, 0.f, 0.4), 5000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_SUSTAIN", "Filter Envelope Sustain", 
        0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_ENV_RELEASE", "Filter Envelope Release", 
        juce::NormalisableRange<float>(20.0f, 5000.0f, 0.f, 0.4), 20.0f));

    //LFO
    params.push_back(std::make_unique< juce::AudioParameterChoice>("LFO_DEST", "LFO Destination",
        juce::StringArray({ "Pitch", "Filter" }), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LFO_AMOUNT", "LFO Amount", 
        juce::NormalisableRange<float>(1.0f, 4.0f, 0.f, 0.3), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LFO_RATE", "LFO Rate", 
        juce::NormalisableRange<float>(0.0f, 20.0f, 0.f, 0.6), 0.0f));



    return { params.begin(), params.end() };
}
