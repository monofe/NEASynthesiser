/*
  ==============================================================================

    Oscillator.h
    Created: 29 Oct 2023 1:05:34am
    Author:  user

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class NEASynthesiserAudioProcessor;

class Oscillator {
public:
    //this defines symbolic constants representing the waveform types. this is defined within the class scope because
    //it is only used by this class
    enum OscillatorType {
        SINE, SQUARE, SAW
    };

    enum OscillatorType type;
    double volume;              //float between 0 and 1
    int coarsePitch;            //integer between -12 and 12 inclusive
    int finePitch;              //integer between -100 and 100 inclusive (because it is measured in cents)
    double pan;                 //float between -1 and 1 inclusive;
    double phaseOffset;

    
    Oscillator(NEASynthesiserAudioProcessor&);

    std::vector<std::vector<float>> generateAudio(int blockSize, int midiNote, int& startSample, double& currentAngle,
        double& currentLFOAngle, bool isNoteOn) const;

private:
    NEASynthesiserAudioProcessor& parentProcessor;
};
