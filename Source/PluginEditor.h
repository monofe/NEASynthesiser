/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include <cstdio>

//==============================================================================
/**
*/

class NEASynthesiserAudioProcessorEditor;

class LinearSlider : public juce::Slider
{
public:
    void paint(juce::Graphics& g) override;
};

//============================================================================================================================

class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) override;


    void drawComboBox(juce::Graphics& g, int width, int height, bool,
        int, int, int, int, juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        const bool isSeparator, const bool isActive,
        const bool isHighlighted, const bool isTicked,
        const bool hasSubMenu, const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* const textColourToUse) override;
};

//============================================================================================================================

class OtherRotaryLookAndFeel : public juce::LookAndFeel_V4
{
private:
    juce::Colour activatedColour;
    bool isActivated;

public:
    OtherRotaryLookAndFeel(juce::Colour c);

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) override;

    void setActivated(bool b);
};

//============================================================================================================================


class SymmetricalRotaryLookAndFeel : public juce::LookAndFeel_V4
{
private:
    OtherRotaryLookAndFeel& filterEnvSmallLookAndFeel;
    NEASynthesiserAudioProcessorEditor& pluginEditor;

public:
    SymmetricalRotaryLookAndFeel(OtherRotaryLookAndFeel& pfilterEnvLF, NEASynthesiserAudioProcessorEditor& ppluginEditor);

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) override;
};

//============================================================================================================================

class LFORotaryLookAndFeel : public OtherRotaryLookAndFeel
{
private:
    juce::Slider& dependentSlider;                  //the slider to be greyed out if this one's value is zero
    OtherRotaryLookAndFeel& dependentLookAndFeel;   //the LookAndFeel object of the above slider

public:
    LFORotaryLookAndFeel(OtherRotaryLookAndFeel& pdependentLookAndFeel, juce::Slider& pdependentSlider, juce::Colour colour);

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) override;
};

//============================================================================================================================

class SnapSlider : public juce::Slider
{
public:

    double snapValue(double attemptedValue, juce::Slider::DragMode dragMode) override {

        if (true /*idk*/) {
            return attemptedValue;
        }

        if ((attemptedValue < 0.10 || attemptedValue > -0.10) && attemptedValue) {
            return 0.0;
        }

        return attemptedValue;
    }
};

class NEASynthesiserAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NEASynthesiserAudioProcessorEditor (NEASynthesiserAudioProcessor&);
    ~NEASynthesiserAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

//private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NEASynthesiserAudioProcessor& audioProcessor;
    MainLookAndFeel mainLookAndFeel;
    OtherRotaryLookAndFeel smallRotaryLookAndFeel;
    OtherRotaryLookAndFeel otherBigRotaryLookAndFeel;
    OtherRotaryLookAndFeel filterEnvSmallRotaryLookAndFeel;
    SymmetricalRotaryLookAndFeel symmetricalRotaryLookAndFeel;
    LFORotaryLookAndFeel lfoAmountLookAndFeel;
    LFORotaryLookAndFeel lfoRateLookAndFeel;

    //GUI elements here
    juce::ComboBox osc1type;    //this is a drop-down box
    LinearSlider osc1vol;
    juce::Slider osc1coarsePitch;
    juce::Slider osc1finePitch;
    juce::Slider osc1pan;
    juce::Slider osc1phaseOffset;

    juce::ComboBox osc2type;    //this is a drop-down box
    LinearSlider osc2vol;
    juce::Slider osc2coarsePitch;
    juce::Slider osc2finePitch;
    juce::Slider osc2pan;
    juce::Slider osc2phaseOffset;

    juce::Slider volEnvAttack;
    juce::Slider volEnvDecay;
    juce::Slider volEnvSustain;
    juce::Slider volEnvRelease;

    juce::ComboBox filterType;
    juce::Slider filterCutoffFrequency;
    juce::Slider filterResonance;

    juce::Slider filterEnvAmount;
    juce::Slider filterEnvAttack;
    juce::Slider filterEnvDecay;
    juce::Slider filterEnvSustain;
    juce::Slider filterEnvRelease;

    juce::ComboBox LFODest;
    juce::Slider LFOAmnt;
    juce::Slider LFORate;



    //attachments here
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1volAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1coarsePitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1finePitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1phaseOffsetAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc2typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2volAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2coarsePitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2finePitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2phaseOffsetAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volEnvAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volEnvDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volEnvSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volEnvReleaseAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterEnvReleaseAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> LFODestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> LFOAmntAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> LFORateAttachment;



    //labels
    juce::Label osc1mainLabel;
    juce::Label osc1volLabel;
    juce::Label osc1coarsePitchLabel;
    juce::Label osc1finePitchLabel;
    juce::Label osc1panLabel;
    juce::Label osc1phaseOffsetLabel;

    juce::Label osc2mainLabel;
    juce::Label osc2volLabel;
    juce::Label osc2coarsePitchLabel;
    juce::Label osc2finePitchLabel;
    juce::Label osc2panLabel;
    juce::Label osc2phaseOffsetLabel;

    juce::Label volEnvMainLabel;
    juce::Label volEnvAttackLabel;
    juce::Label volEnvDecayLabel;
    juce::Label volEnvSustainLabel;
    juce::Label volEnvReleaseLabel;

    juce::Label filterMainLabel;
    juce::Label filterCutoffFrequencyLabel;
    juce::Label filterResonanceLabel;

    juce::Label filterEnvMainLabel;
    juce::Label filterEnvAmountLabel;
    juce::Label filterEnvAttackLabel;
    juce::Label filterEnvDecayLabel;
    juce::Label filterEnvSustainLabel;
    juce::Label filterEnvReleaseLabel;

    juce::Label LFOMainLabel;
    juce::Label LFOAmntLabel;
    juce::Label LFORateLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NEASynthesiserAudioProcessorEditor)
};


