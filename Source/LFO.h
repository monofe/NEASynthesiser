/*
  ==============================================================================

    LFO.h
    Created: 6 Nov 2023 2:38:51am
    Author:  user

  ==============================================================================
*/

#pragma once

class LFO {
public:
    enum DestinationType {
        PITCH, FILTER
    };
    
    double rate;
    double amount;
    enum DestinationType destination;
};

