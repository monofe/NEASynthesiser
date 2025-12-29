/*
  ==============================================================================

    Envelope.h
    Created: 30 Oct 2023 1:19:32am
    Author:  user

  ==============================================================================
*/

#pragma once

class Envelope {
public:
    int attack;
    int decay;
    double sustain;
    int release;
    double amount;

    //the integer attributes will be measured in samples
    //sustain is between 0 and 1
};

