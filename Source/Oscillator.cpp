/*
  ==============================================================================

    Oscillator.cpp
    Created: 29 Oct 2023 1:26:40am
    Author:  user

  ==============================================================================
*/

#include "Oscillator.h"
#include "PluginProcessor.h"
#include <cmath>
#define TWELFTH_ROOT_OF_TWO 1.05946309436f

//square wave function with period of twoPi
double square(double angle) {
    //takes modulus of angle with respect to twoPi, essentially finding the remainder when angle is divided by twoPi
    angle = std::fmodl(angle, juce::MathConstants<double>::twoPi);

    if (angle <= juce::MathConstants<double>::pi) {
        return 1.0;
    }
    else {
        return -1.0;
    }
}

//saw wave function with period of twoPi
double saw(double angle) {
    //remainder when angle is divided by twoPi
    angle = std::fmodl(angle, juce::MathConstants<double>::twoPi);

    double value = angle / juce::MathConstants<double>::pi;

    if (angle <= juce::MathConstants<double>::pi) {
        return value;
    }
    else {
        return value - 2.0;
    }
}

//array of function pointers.
double (*wave[3])(double) = { &std::sin, &square, &saw };


Oscillator::Oscillator(NEASynthesiserAudioProcessor& p) : parentProcessor(p) {
    type = SINE;
    volume = 0.0f;
    coarsePitch = 0;
    finePitch = 0;
    pan = 0;
    phaseOffset = 0;
}

std::vector<std::vector<float>> Oscillator::generateAudio(int blockSize, int midiNote, int& startSample, double& currentAngle,
    double& currentLFOAngle, bool isNoteOn) const {

    //the way the panning works is that it just reduces the volume of one of the channels. At 0 panning, both channels will be at
    //maximum volume
    double leftChannelVolume = 1;
    double rightChannelVolume = 1;

    double frequency = juce::MidiMessage::getMidiNoteInHertz(midiNote + coarsePitch);
    frequency *= std::pow(TWELFTH_ROOT_OF_TWO, finePitch / 100.0f);

    //dealing with the lfo here
    if (parentProcessor.lfo.destination == parentProcessor.lfo.PITCH) {
        //lfo equation
        double lfoFactor = std::pow(parentProcessor.lfo.amount, std::sin(currentLFOAngle));
        frequency *= lfoFactor;

        int numSamples;

        if (isNoteOn) {
            numSamples = blockSize - startSample;
        }
        else {
            numSamples = blockSize;
        }

        double lfoAngleDelta = juce::MathConstants<double>::twoPi * (parentProcessor.lfo.rate / parentProcessor.sampleRate) *
            (numSamples);

        currentLFOAngle += lfoAngleDelta;
    }

    double cyclesPerSample = frequency / parentProcessor.sampleRate;
    double angleDelta = juce::MathConstants<double>::twoPi * cyclesPerSample;

    if (pan < 0) {
        //decrease rightChannelVolume
        rightChannelVolume = rightChannelVolume + pan;
    }
    else if (pan > 0) {
        //decrease leftChannelVolume
        leftChannelVolume = leftChannelVolume - pan;
    }

    leftChannelVolume = leftChannelVolume * volume;
    rightChannelVolume = rightChannelVolume * volume;

    std::vector<float> leftChannelOutput(blockSize, 0.0f);    //creates an array with a size of blockSize
    std::vector<float> rightChannelOutput(blockSize, 0.0f);   //creates an array with a size of blockSize

    double currentAngleWithPhase = currentAngle + phaseOffset;
    
    if (!isNoteOn) {    //equivalent to (isNoteOn == false)
        for (int i = 0; i < startSample; ++i) {
            leftChannelOutput[i] = (wave[type])(currentAngleWithPhase) * leftChannelVolume;
            rightChannelOutput[i] = (wave[type])(currentAngleWithPhase) * rightChannelVolume;
            currentAngleWithPhase += angleDelta;
        }
    }

    for (int i = startSample; i < blockSize; ++i) {
        leftChannelOutput[i] = (wave[type])(currentAngleWithPhase) * leftChannelVolume;
        rightChannelOutput[i] = (wave[type])(currentAngleWithPhase) * rightChannelVolume;
        currentAngleWithPhase += angleDelta;
    }

    startSample = 0;
    currentAngle = currentAngleWithPhase - phaseOffset;
    
    std::vector<std::vector<float>> output(2, {0.0f});
    output[0] = leftChannelOutput;
    output[1] = rightChannelOutput;
    return output;
}

