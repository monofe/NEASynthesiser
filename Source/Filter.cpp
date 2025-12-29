/*
  ==============================================================================

    Filter.cpp
    Created: 5 Nov 2023 5:26:13pm
    Author:  user

  ==============================================================================
*/

#include "Filter.h"
#include "PluginProcessor.h"
#include "cmath"

FrequencyFilter::FrequencyFilter(NEASynthesiserAudioProcessor& p)
    : parentProcessor(p)
{
    type = LOWPASS;
    centreFrequency = 20000;
    resonance = 0.7071068;    //sqrt(2) / 2, this is thought of as a default value
    currentLFOAngle = 0.0f;
}

std::vector<std::vector<float>> FrequencyFilter::filterAudio(std::vector<std::vector<float>> inputAudio
    , int currentSampleIndex, bool isNoteOn, std::vector<std::vector<float>> lastTwoInputSamples,
    std::vector<std::vector<float>> lastTwoOutputSamples, double& releaseFrequency)
{
    // This algorithm is simply a code implementation of the algorithm found here:
    // https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

    double frequency = getCurrentCentreFrequency(currentSampleIndex, isNoteOn, releaseFrequency);
    auto blockSize = inputAudio[0].size();

    if (parentProcessor.lfo.destination == parentProcessor.lfo.FILTER) {
        double lfoFactor = std::pow(parentProcessor.lfo.amount, std::sin(currentLFOAngle));
        frequency = std::min(frequency * lfoFactor, 20000.0);
    }

    double omega = juce::MathConstants<double>::twoPi * frequency / parentProcessor.sampleRate;
    double alpha = std::sin(omega) / (2 * resonance);
    double cosOmega = std::cos(omega);


    double a0 = 1 + alpha;
    
    double c1, c2, c3, c4;

    if (type == LOWPASS) {
        c2 = (1 - cosOmega) / a0;
        c1 = c2 / 2;
        c3 = (-2 * cosOmega) / a0;
        c4 = (1 - alpha) / a0;
    } 
    else if (type == HIGHPASS) {
        c2 = -(1 + cosOmega) / a0;
        c1 = (1 + cosOmega) / (2 * a0 );
        c3 = (-2 * cosOmega) / a0;
        c4 = (1 - alpha) / a0;
    }

    std::vector<float> leftOutput(blockSize, 0.0f);
    std::vector<float> rightOutput(blockSize, 0.0f);

    //similarly to before, lastTwoInputSamples[0] represents the left channel, and lastTwoInputSamples[1] is the right channel
    //the same goes for lastTwoOutputSamples

    leftOutput[0] = (c1 * inputAudio[0][0]) + (c2 * lastTwoInputSamples[0][1]) + (c1 * lastTwoInputSamples[0][0])
        - (c3 * lastTwoOutputSamples[0][1]) - (c4 * lastTwoOutputSamples[0][0]);

    rightOutput[0] = (c1 * inputAudio[1][0]) + (c2 * lastTwoInputSamples[1][1]) + (c1 * lastTwoInputSamples[1][0])
        - (c3 * lastTwoOutputSamples[1][1]) - (c4 * lastTwoOutputSamples[1][0]);

    //leftOutput[0] = c1 * inputAudio[0][0];
    //rightOutput[0] = c1 * inputAudio[1][0];

    //handle case where only one input sample is passed
    if (blockSize < 2) {
        return { leftOutput, rightOutput };
    }

    //leftOutput[1] = (c1 * inputAudio[0][1]) + (c2 * inputAudio[0][0]) - (c3 * leftOutput[0]);
    //rightOutput[1] = (c1 * inputAudio[1][1]) + (c2 * inputAudio[1][0]) - (c3 * rightOutput[0]);

    leftOutput[1] = (c1 * inputAudio[0][1]) + (c2 * inputAudio[0][0]) + (c1 * lastTwoInputSamples[0][1])
        - (c3 * leftOutput[0]) - (c4 * lastTwoOutputSamples[0][1]);

    rightOutput[1] = (c1 * inputAudio[1][1]) + (c2 * inputAudio[1][0]) + (c1 * lastTwoInputSamples[1][1])
        - (c3 * rightOutput[0]) - (c4 * lastTwoOutputSamples[1][1]);

    for (int i = 2; i < blockSize; ++i) {
        leftOutput[i] = (c1 * inputAudio[0][i]) + (c2 * inputAudio[0][i - 1]) + (c1 * inputAudio[0][i - 2])
            - (c3 * leftOutput[i - 1]) - (c4 * leftOutput[i - 2]);

        rightOutput[i] = (c1 * inputAudio[1][i]) + (c2 * inputAudio[1][i - 1]) + (c1 * inputAudio[1][i - 2])
            - (c3 * rightOutput[i - 1]) - (c4 * rightOutput[i - 2]);
    }

    return { leftOutput, rightOutput };
}

double FrequencyFilter::getCurrentCentreFrequency(int currentSampleIndex, bool isNoteOn, double& releaseFrequency)
{
    if (isNoteOn == false) {
        if (currentSampleIndex > env.release) {
            return centreFrequency;
        }

        double n = releaseFrequency - centreFrequency;
        auto ret =  ((-n / env.release) * currentSampleIndex) + centreFrequency + n;
        
        return std::fmax(std::fmin(20000, ret), 40);
    }

    if (currentSampleIndex < env.attack) {
        if (env.decay == 0) {
            auto ret = ((env.amount * env.sustain / env.attack) * currentSampleIndex) + centreFrequency;
            releaseFrequency = std::fmax(std::fmin(20000, ret), 40);
            return releaseFrequency;
        }

        auto ret = ((env.amount / env.attack) * currentSampleIndex) + centreFrequency;
        releaseFrequency = std::fmax(std::fmin(20000, ret), 40);
        return releaseFrequency;

    }
    else if (currentSampleIndex - env.attack < env.decay) {
        int shiftedCurrentSample = currentSampleIndex - env.attack;
        double gradient = (env.amount * (env.sustain - 1)) / env.decay;
        auto ret = (gradient * shiftedCurrentSample) + centreFrequency + env.amount;
        releaseFrequency = std::fmax(std::fmin(20000, ret), 40);
        return releaseFrequency;

    } 
    else {
        auto ret = centreFrequency + (env.sustain * env.amount);
        releaseFrequency = std::fmax(std::fmin(20000, ret), 40);
        return releaseFrequency;
    }
}

