/*
  ==============================================================================

    SynthVoice.cpp
    Created: 29 Oct 2023 2:55:44am
    Author:  user

  ==============================================================================
*/

#include "SynthVoice.h"
#include "PluginProcessor.h"

// SynthVoice============================================================================================================

SynthVoice::SynthVoice(NEASynthesiserAudioProcessor& p) : parentProcessor(p),
lastTwoInputSamples(2, { 0.0f, 0.0f }), lastTwoOutputSamples(2, { 0.0f, 0.0f })
{
    currentSampleIndex = 0;
    startSampleIndex = 0;
    currentOsc1Angle = 0.0f;
    currentOsc2Angle = 0.0f;
    currentOsc1LFOAngle = 0.0f;
    currentOsc2LFOAngle = 0.0f;
    _midiNote = 0;
    midiVelocity = 0.0f;
    isNoteOn = false;
    _isFree = true;
}

void SynthVoice::resetVoice(double midiVelocity, int startSampleIndex) {
    currentSampleIndex = 0;
    currentOsc1Angle = 0.0f;
    currentOsc2Angle = 0.0f;
    currentOsc1LFOAngle = 0.0f;
    currentOsc2LFOAngle = 0.0f;
    isNoteOn = true;
    _isFree = false;

    this->midiVelocity = midiVelocity;
    this->startSampleIndex = startSampleIndex;
}

void SynthVoice::addVoice(int midiNote, double midiVelocity, int startSampleIndex) {
    //this is identical to resetVoice except that it also resets the midiNote. This function is only called in the
    //addVoice() method of the SynthVoiceArray class

    resetVoice(midiVelocity, startSampleIndex);
    this->_midiNote = midiNote;
}

void SynthVoice::turnOffVoice(int startSampleIndex) {
    currentSampleIndex = 0;
    isNoteOn = false;
    this->startSampleIndex = startSampleIndex;
}

const int& SynthVoice::midiNote() const {
    return _midiNote;
}

const bool& SynthVoice::isFree() const {
    return _isFree;
}

double SynthVoice::getCurrentVolume()
{
    if (!isNoteOn)      //if isNoteOn == False
    {    
        if (currentSampleIndex < parentProcessor.volumeEnv.release) 
        {
            //use releaseVolume instead of the sustain volume for the computation here
            tailVolume = releaseVolume - (static_cast<double>(currentSampleIndex) *
                releaseVolume / static_cast<double>(parentProcessor.volumeEnv.release));

            return tailVolume;
        } 
        else 
        {
            //explicitly not setting tailVolume here, since tailVolume should be the final volume you calculate from 
            //this function.

            return 0;
        }
    }

    if (currentSampleIndex < parentProcessor.volumeEnv.attack)
    {
        if (parentProcessor.volumeEnv.decay == 0)
        {
            releaseVolume = currentSampleIndex * static_cast<double>(parentProcessor.volumeEnv.sustain)
                / static_cast<double>(parentProcessor.volumeEnv.attack);

            tailVolume = releaseVolume;

            return tailVolume;
        }

        releaseVolume = currentSampleIndex / static_cast<double>(parentProcessor.volumeEnv.attack);

        tailVolume = releaseVolume;
        
        return tailVolume;
    }
    else if (currentSampleIndex - parentProcessor.volumeEnv.attack < parentProcessor.volumeEnv.decay)
    {
        int shiftedCurrentSampleIndex = currentSampleIndex - parentProcessor.volumeEnv.attack;

        releaseVolume = 1 + (shiftedCurrentSampleIndex * ((parentProcessor.volumeEnv.sustain - 1) /
            static_cast<double>(parentProcessor.volumeEnv.decay)));

        tailVolume = releaseVolume;

        return tailVolume;
    }
    else
    {
        releaseVolume = parentProcessor.volumeEnv.sustain;
        tailVolume = releaseVolume;
        return tailVolume;
    }
}



std::vector<std::vector<float>> SynthVoice::generateAudio(int blockSize) {

    //the 0th index of each list contains the left channel and the 1st index contains the right channel
    auto tempStartSampleIndex = startSampleIndex;
    
    auto osc1Audio = parentProcessor.osc1.generateAudio(blockSize, _midiNote, startSampleIndex,
        currentOsc1Angle, currentOsc1LFOAngle, isNoteOn);

    auto osc2Audio = parentProcessor.osc2.generateAudio(blockSize, _midiNote, tempStartSampleIndex,
        currentOsc2Angle, currentOsc2LFOAngle, isNoteOn);

    std::vector<float> leftChannelOutput(blockSize, 0.0f);
    std::vector<float> rightChannelOutput(blockSize, 0.0f);

    auto adsrVol = getCurrentVolume();
    
    auto volume = midiVelocity * adsrVol;

    if (currentSampleIndex == 0 && isNoteOn && blockSize > 1) {
        int firstZeroIndex;
        bool zeroEncounteredFlag = false;

        //the goal here is to remove all samples before the first zero, so there isnt 
        //any popping sound when the note is switched on

        //the first for loop finds the index of the first zero value
        for (firstZeroIndex = 1; firstZeroIndex < blockSize; ++firstZeroIndex) {
            if ((osc1Audio[0][firstZeroIndex] >= 0.0f && osc1Audio[0][firstZeroIndex - 1] <= 0.0f)
                || (osc1Audio[0][firstZeroIndex] <= 0.0f && osc1Audio[0][firstZeroIndex - 1] >= 0.0f)) {
                zeroEncounteredFlag = true;
                break;
            }
        }

        if (zeroEncounteredFlag) {
            zeroEncounteredFlag = false;
            //this loop sets everything before the first zero index to zero
            for (int i = 0; i < firstZeroIndex; ++i) {
                osc1Audio[0][i] = 0.0f;
                osc1Audio[1][i] = 0.0f;
            }
        }

        //the same is repeated for osc2Audio
        for (firstZeroIndex = 1; firstZeroIndex < blockSize; ++firstZeroIndex) {
            if ((osc2Audio[0][firstZeroIndex] >= 0.0f && osc2Audio[0][firstZeroIndex - 1] <= 0.0f)
                || (osc2Audio[0][firstZeroIndex] <= 0.0f && osc2Audio[0][firstZeroIndex - 1] >= 0.0f)) {
                zeroEncounteredFlag = true;
                break;
            }
        }

        if (zeroEncounteredFlag) {
            zeroEncounteredFlag = false;
            for (int i = 0; i < firstZeroIndex; ++i) {
                osc2Audio[0][i] = 0.0f;
                osc2Audio[1][i] = 0.0f;
            }
        }
    }

    if (adsrVol == 0.0 && !isNoteOn) {
        //generate the audio for this last block. each sample will be given its own volume, so that it converges smoothly
        //to zero
        volume = tailVolume * midiVelocity;

        for (int i = 0; i < blockSize; ++i) {
            //this uses a similar computation as when calculating the volume in the release stage
            auto sampleVolume = (volume) - (volume * i / blockSize);

            leftChannelOutput[i] = (osc1Audio[0][i] + osc2Audio[0][i]) * sampleVolume;
            rightChannelOutput[i] = (osc1Audio[1][i] + osc2Audio[1][i]) * sampleVolume;
        }
    }
    else
    {
        //generate audio as normal
        for (int i = 0; i < blockSize; ++i) {
            leftChannelOutput[i] = (osc1Audio[0][i] + osc2Audio[0][i]) * volume;
            rightChannelOutput[i] = (osc1Audio[1][i] + osc2Audio[1][i]) * volume;
        }
    }

    std::vector<std::vector<float>> output = { leftChannelOutput, rightChannelOutput };

    output = parentProcessor.filter.filterAudio(output, currentSampleIndex, isNoteOn, lastTwoInputSamples,
        lastTwoOutputSamples, releaseFrequency);

    currentSampleIndex += blockSize;

    //if blocksize < 2, then there would be another index out of range error
    //so you have to take care of this case again
    if (blockSize < 2) 
    {

        //lastTwoInputSamples[0] represents the left channel, and [1] represents the right channel
        //same with lastTwoOutputSamples

        lastTwoInputSamples = { { lastTwoInputSamples[0][1], leftChannelOutput[blockSize - 1]},
        { lastTwoInputSamples[1][1], rightChannelOutput[blockSize - 1]} };

        lastTwoOutputSamples = { { lastTwoOutputSamples[0][1], output[0][blockSize - 1] },
            { lastTwoOutputSamples[1][1], output[1][blockSize - 1]} };
    }
    else if (adsrVol == 0.0 && !isNoteOn)
    {
        //reset attributes
        resetVoice(0.0f, 0);
        isNoteOn = false;
        _isFree = true;
        _midiNote = 0;
        lastTwoInputSamples = { {0.0f, 0.0f}, {0.0f, 0.0f} };
        lastTwoOutputSamples = { {0.0f, 0.0f}, {0.0f, 0.0f} };
    }
    else 
    {
        lastTwoInputSamples = { {leftChannelOutput[blockSize - 2], leftChannelOutput[blockSize - 1]},
            { rightChannelOutput[blockSize - 2], rightChannelOutput[blockSize - 1]} };

        lastTwoOutputSamples = { { output[0][blockSize - 2], output[0][blockSize - 1] },
            { output[1][blockSize - 2], output[1][blockSize - 1]} };
    }


    return output;
}


// SynthVoiceArray===========================================================================================================


SynthVoiceArray::SynthVoiceArray(NEASynthesiserAudioProcessor& p) : parentProcessor(p),
    arr(32, SynthVoice(p)) {}

int SynthVoiceArray::find(int midiNote) const {
    
    for (int i = 0; i < maxNumVoices; ++i) {
        if (arr[i].midiNote() == midiNote) {
            return i;
        }
    }

    return -1;
}

void SynthVoiceArray::addVoice(int midiNote, double midiVelocity, int startSampleIndex) {

    for (int i = 0; i < maxNumVoices; ++i) {
        if (arr[i].isFree()) {
            arr[i].addVoice(midiNote, midiVelocity, startSampleIndex);
            break;
        }
    }
}

std::vector<std::vector<float>> SynthVoiceArray::generateAudio(int blockSize) {
    
    std::vector<float> left(blockSize, 0.0f);
    std::vector<float> right(blockSize, 0.0f);

    for (auto& voice : arr) {       //for each element in the arr
        if (voice.isFree()) {       //if the voice is free, then it is not generating audio so skip this
            continue;
        }

        auto ret = voice.generateAudio(blockSize);

        for (int i = 0; i < blockSize; ++i) {
            left[i] += ret[0][i];      //deals with left channel
            right[i] += ret[1][i];      //deals with right channel
        }
    }

    std::vector<std::vector<float>> output(2, {0.0f});
    output[0] = left;
    output[1] = right;

    return output;
}

void SynthVoiceArray::resetVoice(int index, double midiVelocity, int startSampleIndex) {
    arr[index].resetVoice(midiVelocity, startSampleIndex);
}

void SynthVoiceArray::turnOffVoice(int index, int startSampleIndex) {
    arr[index].turnOffVoice(startSampleIndex);
}
