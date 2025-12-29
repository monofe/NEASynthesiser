/*
  ==============================================================================

    SynthVoice.h
    Created: 29 Oct 2023 2:55:32am
    Author:  user

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include <vector>

class NEASynthesiserAudioProcessor;

class SynthVoice {
private:
    int currentSampleIndex;
    int startSampleIndex;
    double currentOsc1Angle;
    double currentOsc2Angle;
    double currentOsc1LFOAngle;
    double currentOsc2LFOAngle;
    int _midiNote;
    double midiVelocity;
    bool isNoteOn;
    bool _isFree;
    double tailVolume;              //the volume calculated in getCurrentVolume() right before the tail
    double releaseVolume;           //the last volume of the note before being released
    double releaseFrequency;        //the last centre frequency of the filter before the note is released

    std::vector<std::vector<float>> lastTwoInputSamples;
    std::vector<std::vector<float>> lastTwoOutputSamples;

    NEASynthesiserAudioProcessor& parentProcessor;

    double getCurrentVolume();

public:
    SynthVoice(NEASynthesiserAudioProcessor&);
    void resetVoice(double midiVelocity, int startSampleIndex);
    void addVoice(int midiNote, double midiVelocity, int startSampleIndex);
    void turnOffVoice(int startSampleIndex);

    const int& midiNote() const;    //getter method
    const bool& isFree() const;    //getter method

    std::vector<std::vector<float>> generateAudio(int blockSize);
};

class SynthVoiceArray {
private:
    std::vector<SynthVoice> arr;
    const int maxNumVoices = 32;

    NEASynthesiserAudioProcessor& parentProcessor;

public:
    SynthVoiceArray(NEASynthesiserAudioProcessor&);
    int find(int midiNote) const;
    void addVoice(int midiNote, double midiVelocity, int startSampleIndex);
    void resetVoice(int index, double midiVelocity, int startSampleIndex);
    void turnOffVoice(int index, int startSampleIndex);

    std::vector<std::vector<float>> generateAudio(int blockSize);
};

