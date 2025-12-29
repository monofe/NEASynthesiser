/*
  ==============================================================================

    Filter.h
    Created: 5 Nov 2023 5:25:45pm
    Author:  user

  ==============================================================================
*/

#pragma once
#include <vector>
#include <JuceHeader.h>
#include "Envelope.h"

class NEASynthesiserAudioProcessor;

class FrequencyFilter 
{
public:
    enum FilterType {
        LOWPASS, HIGHPASS
    };

    enum FilterType type;
    double centreFrequency;
    double resonance;
    Envelope env;
    double currentLFOAngle;

    FrequencyFilter(NEASynthesiserAudioProcessor&);

    std::vector<std::vector<float>> filterAudio(std::vector<std::vector<float>> inputAudio
        , int currentSampleIndex, bool isNoteOn, std::vector<std::vector<float>> lastTwoInputSamples,
        std::vector<std::vector<float>> lastTwoOutputSamples, double& releaseFrequency);

    double getCurrentCentreFrequency(int currentSampleIndex, bool isNoteOn, double& releaseFrequency);

private:
    NEASynthesiserAudioProcessor& parentProcessor;
};


