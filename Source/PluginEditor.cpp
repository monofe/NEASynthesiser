/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//const juce::Colour knobBlack = juce::Colour(0xff131313);
const juce::Colour knobBlack = juce::Colour(0xff656565);
const juce::Colour knobAqua = juce::Colour(0xff2CF4F5);
const juce::Colour knobGreen = juce::Colour(0xff0DFF11);
const juce::Colour deactivatedColour = juce::Colour(0xbfbebebe);
const juce::Colour knobFilledCircleColour = juce::Colour(0xff28292b);
const juce::Colour knobDialTickColour = juce::Colour(0xffefd7e3);


//==============================================================================
NEASynthesiserAudioProcessorEditor::NEASynthesiserAudioProcessorEditor (NEASynthesiserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 
    smallRotaryLookAndFeel(knobGreen),   //green
    filterEnvSmallRotaryLookAndFeel(knobGreen),
    otherBigRotaryLookAndFeel(knobAqua),
    lfoAmountLookAndFeel(lfoRateLookAndFeel, LFORate, knobGreen),
    lfoRateLookAndFeel(lfoAmountLookAndFeel, LFOAmnt, knobGreen),
    symmetricalRotaryLookAndFeel(filterEnvSmallRotaryLookAndFeel, *this)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (655, 395);

    p.editor = this;

    //setting the style here
    osc1type.addItem("Sine", 1);
    osc1type.addItem("Square", 2);
    osc1type.addItem("Saw", 3);

    osc2type.addItem("Sine", 1);
    osc2type.addItem("Square", 2);
    osc2type.addItem("Saw", 3);

    filterType.addItem("Low-pass", 1);
    filterType.addItem("High-pass", 2);

    LFODest.addItem("Pitch", 1);
    LFODest.addItem("Filter", 2);

    //all rotary sliders
    std::vector<juce::Slider*> sliderList = { &osc1coarsePitch, &osc1finePitch, &osc1pan, &osc1phaseOffset,
    &osc2coarsePitch, &osc2finePitch, &osc2pan, &osc2phaseOffset, 
    &volEnvAttack, &volEnvDecay, &volEnvSustain, &volEnvRelease,
    &filterCutoffFrequency, &filterResonance, &filterEnvAmount, &filterEnvAttack, &filterEnvDecay, &filterEnvSustain, 
    &filterEnvRelease,
    &LFOAmnt, &LFORate
    };

    //corresponds to the list above
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>*> attList =
    { &osc1coarsePitchAttachment, &osc1finePitchAttachment, &osc1panAttachment, &osc1phaseOffsetAttachment,
    &osc2coarsePitchAttachment, &osc2finePitchAttachment, &osc2panAttachment, &osc2phaseOffsetAttachment,
    &volEnvAttackAttachment, &volEnvDecayAttachment, &volEnvSustainAttachment, &volEnvReleaseAttachment, 
        &filterCutoffFrequencyAttachment, &filterResonanceAttachment, 
        &filterEnvAmountAttachment, &filterEnvAttackAttachment,&filterEnvDecayAttachment, &filterEnvSustainAttachment, &filterEnvReleaseAttachment,
        &LFOAmntAttachment, &LFORateAttachment
    };

    //corresponds to the list above
    std::vector<std::string> paramIDList = { "OSC1_CP", "OSC1_FP", "OSC1_PAN", "OSC1_PO",
        "OSC2_CP", "OSC2_FP", "OSC2_PAN", "OSC2_PO",
        "VOL_ENV_ATTACK", "VOL_ENV_DECAY", "VOL_ENV_SUSTAIN", "VOL_ENV_RELEASE",
        "FILTER_CF", "FILTER_RES",
        "FILTER_ENV_AMOUNT", "FILTER_ENV_ATTACK", "FILTER_ENV_DECAY", "FILTER_ENV_SUSTAIN", "FILTER_ENV_RELEASE",
        "LFO_AMOUNT", "LFO_RATE"
    };

    osc1vol.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    osc1vol.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    osc2vol.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    osc2vol.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    for (auto i = 0; i < sliderList.size(); ++i) {
        auto s = sliderList[i];

        s->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        s->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        s->setLookAndFeel(&mainLookAndFeel);

        //For all the small knobs. Filter Frequency and envelope amount are excluded.
        if (i >= 8) {
            s->setLookAndFeel(&smallRotaryLookAndFeel);
        }

        //for Filter Frequency and Filter Envelope Amount knobs. These are still big and so are being rendered
        //with a different colour

        addAndMakeVisible(s);
    }

    filterEnvAttack.setLookAndFeel(&filterEnvSmallRotaryLookAndFeel);
    filterEnvDecay.setLookAndFeel(&filterEnvSmallRotaryLookAndFeel);
    filterEnvSustain.setLookAndFeel(&filterEnvSmallRotaryLookAndFeel);
    filterEnvRelease.setLookAndFeel(&filterEnvSmallRotaryLookAndFeel);

    filterCutoffFrequency.setLookAndFeel(&otherBigRotaryLookAndFeel);
    filterEnvAmount.setLookAndFeel(&symmetricalRotaryLookAndFeel);

    LFOAmnt.setLookAndFeel(&lfoAmountLookAndFeel);
    LFORate.setLookAndFeel(&lfoRateLookAndFeel);

    for (int i = 0; i < sliderList.size(); ++i) {
        *(attList[i]) = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            paramIDList[i], *(sliderList[i]));
    }

    osc1typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
        "OSC1_TYPE", osc1type);
    osc1volAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
        "OSC1_VOL", osc1vol);
    
    osc2typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
        "OSC2_TYPE", osc2type);
    osc2volAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
        "OSC2_VOL", osc2vol);

    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
        "FILTER_TYPE", filterType);

    LFODestAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
        "LFO_DEST", LFODest);


    addAndMakeVisible(osc1type);
    addAndMakeVisible(osc1vol);
    addAndMakeVisible(osc2type);
    addAndMakeVisible(osc2vol);
    addAndMakeVisible(filterType);
    addAndMakeVisible(LFODest);

    osc1vol.setLookAndFeel(&mainLookAndFeel);
    osc2vol.setLookAndFeel(&mainLookAndFeel);
    osc1type.setLookAndFeel(&mainLookAndFeel);
    osc2type.setLookAndFeel(&mainLookAndFeel);
    filterType.setLookAndFeel(&mainLookAndFeel);
    LFODest.setLookAndFeel(&mainLookAndFeel);

    //labels
    std::vector<juce::Label*> labelList =
    { &osc1mainLabel, &osc1volLabel, &osc1coarsePitchLabel, &osc1finePitchLabel, &osc1panLabel, &osc1phaseOffsetLabel,
        &osc2mainLabel, &osc2volLabel, &osc2coarsePitchLabel, &osc2finePitchLabel, &osc2panLabel, &osc2phaseOffsetLabel,
        &volEnvMainLabel, &volEnvAttackLabel, &volEnvDecayLabel, &volEnvSustainLabel, &volEnvReleaseLabel,
        &filterMainLabel, &filterCutoffFrequencyLabel, &filterResonanceLabel,
        &filterEnvMainLabel, &filterEnvAmountLabel, &filterEnvAttackLabel, &filterEnvDecayLabel, &filterEnvSustainLabel, &filterEnvReleaseLabel,
        &LFOMainLabel, &LFOAmntLabel, &LFORateLabel
    };

    std::vector<std::string> labelTextList =
    { "Osc 1", "Volume", "Coarse", "Fine", "Pan", "Phase",
        "Osc 2", "Volume", "Coarse", "Fine", "Pan", "Phase",
        "Vol Env", "Attack", "Decay", "Sustain", "Release",
        "Filter", "Freq", "Res",
        "Filter Env", "Amount", "Attack", "Decay", "Sustain", "Release",
        "LFO", "Amount", "Rate"
    };

    for (int i = 0; i < labelList.size(); ++i) {
        addAndMakeVisible(*(labelList[i]));
        labelList[i]->setText(labelTextList[i], juce::NotificationType::dontSendNotification);
        labelList[i]->setFont(juce::Font(/*"Arial",*/ 14.0f, 0));
    }

    auto titleFontSize = 25.0f;
    osc1mainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize, 0));
    osc2mainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize, 0));
    volEnvMainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize - 1.5f, 0));
    filterMainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize - 1.5f, 0));
    filterEnvMainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize - 1.5f, 0));
    LFOMainLabel.setFont(juce::Font(/*"Arial",*/ titleFontSize - 1.5f, 0));

    osc1volLabel.setJustificationType(juce::Justification::centred);
    osc1coarsePitchLabel.setJustificationType(juce::Justification::centred);
    osc1finePitchLabel.setJustificationType(juce::Justification::centred);
    osc1panLabel.setJustificationType(juce::Justification::centred);
    osc1phaseOffsetLabel.setJustificationType(juce::Justification::centred);

    osc2volLabel.setJustificationType(juce::Justification::centred);
    osc2coarsePitchLabel.setJustificationType(juce::Justification::centred);
    osc2finePitchLabel.setJustificationType(juce::Justification::centred);
    osc2panLabel.setJustificationType(juce::Justification::centred);
    osc2phaseOffsetLabel.setJustificationType(juce::Justification::centred);

    volEnvAttackLabel.setJustificationType(juce::Justification::centred);
    volEnvDecayLabel.setJustificationType(juce::Justification::centred);
    volEnvSustainLabel.setJustificationType(juce::Justification::centred);
    volEnvReleaseLabel.setJustificationType(juce::Justification::centred);
}

NEASynthesiserAudioProcessorEditor::~NEASynthesiserAudioProcessorEditor()
{
}

//==============================================================================
void NEASynthesiserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colours::darkgrey);

    //g.setColour(juce::Colours::darkgrey);
    g.setColour(juce::Colour(0xff404040));
    //g.setColour(juce::Colour(0xff393545));

    g.fillRoundedRectangle(10.0f, 10.0f, 310, 175, 10);
    g.fillRoundedRectangle(335.0f, 10.0f, 310, 175, 10);

    g.fillRoundedRectangle(10.0f, 210.0f, 330, 175, 10);
    g.fillRoundedRectangle(355.0f, 210.0f, 135, 175, 10);
    g.fillRoundedRectangle(505.0f, 210.0f, 140, 175, 10);

    juce::Path mainDivider;
    juce::Line<float> mainDividerLine(10.0f, 185 + 25.0f/2.0f, 655.0f - 10.0f, 185 + 25.0f / 2.0f);
    mainDivider.addLineSegment(mainDividerLine, 1.5f);
    g.setColour(juce::Colours::grey);
    g.strokePath(mainDivider, juce::PathStrokeType(1.5f));

    juce::Path filterDivider;
    juce::Line<float> filterDividerLine(140.0f + 2.5, 210 + 1, 140.0f + 2.5, 210 + 175 - 1);
    filterDivider.addLineSegment(filterDividerLine, 2.0f);
    //g.setColour(juce::Colour(0xff282828));
    g.setColour(juce::Colours::darkgrey);
    g.strokePath(filterDivider, juce::PathStrokeType(2.0f));


    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NEASynthesiserAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //X, Y, width, height

    auto smallKnobSize = 37;
    auto bigKnobSize = 50;

    auto firstRowY = 115;
    auto secondHalfY = 220;
    
    osc1type.setBounds({ 20, 60, 100,40 });
    osc1coarsePitch.setBounds({ 20, firstRowY,50,50 });
    osc1finePitch.setBounds({ 80, firstRowY,50,50 });
    osc1pan.setBounds({ 140, firstRowY,50,50 });
    osc1phaseOffset.setBounds({ 200, firstRowY, 50, 50 });
    osc1vol.setBounds({ 280, 60, 20, 95 });

    osc2type.setBounds({ 315 + 30, 60, 100, 40 });
    osc2coarsePitch.setBounds({ 315 + 30,firstRowY,50,50 });
    osc2finePitch.setBounds({ 315 + 90,firstRowY,50,50 });
    osc2pan.setBounds({ 315 + 150,firstRowY,50,50 });
    osc2phaseOffset.setBounds({ 315 + 210, firstRowY, 50, 50 });
    osc2vol.setBounds({ 320 + 285, 60, 20, 95 });

    volEnvAttack.setBounds({ 370, secondHalfY + 40, smallKnobSize, smallKnobSize });
    volEnvDecay.setBounds({ 435, secondHalfY + 40, smallKnobSize, smallKnobSize });
    volEnvSustain.setBounds({ 370, secondHalfY + 115 - 10, smallKnobSize, smallKnobSize });
    volEnvRelease.setBounds({ 435, secondHalfY + 115 - 10, smallKnobSize, smallKnobSize });

    filterEnvAttack.setBounds({ 165, secondHalfY + 40, smallKnobSize, smallKnobSize });
    filterEnvDecay.setBounds({ 225, secondHalfY + 40, smallKnobSize, smallKnobSize });
    filterEnvSustain.setBounds({ 165, secondHalfY + 115 - 10, smallKnobSize, smallKnobSize });
    filterEnvRelease.setBounds({ 225, secondHalfY + 115 - 10, smallKnobSize, smallKnobSize });
    filterEnvAmount.setBounds({ 275, secondHalfY + 80 - 5, bigKnobSize, bigKnobSize });

    filterType.setBounds({ 25, secondHalfY + 40, 100, 35 });
    filterCutoffFrequency.setBounds({ 20, secondHalfY + 85, bigKnobSize, bigKnobSize });
    filterResonance.setBounds({ 90, secondHalfY + 95, smallKnobSize, smallKnobSize });

    LFODest.setBounds({ 550 - 5 - 20, secondHalfY + 40, 100, 35 });
    LFOAmnt.setBounds({ 555 - 10 - 20, secondHalfY + 95, smallKnobSize, smallKnobSize });
    LFORate.setBounds({ 620 - 10 - 20, secondHalfY + 95, smallKnobSize, smallKnobSize });

    //labels
    //=======================================================================================
    auto lowerHalfLabelWidth = 47;
    auto labelPushup = 10;

    osc1mainLabel.setBounds({ 20, 15, 130, 40 });
    osc1coarsePitchLabel.setBounds({20, firstRowY + 50 - labelPushup, 50, 20});
    osc1finePitchLabel.setBounds({ 80, firstRowY + 50 - labelPushup, 50, 20 });
    osc1panLabel.setBounds({ 140, firstRowY + 50 - labelPushup, 50, 20 });
    osc1phaseOffsetLabel.setBounds({ 200, firstRowY + 50 - labelPushup, 50, 20 });
    osc1volLabel.setBounds({ 260, firstRowY + 50 - labelPushup, 50, 20 });

    osc2mainLabel.setBounds({ 315 + 30, 15, 130, 40 });
    osc2coarsePitchLabel.setBounds({ 315 + 30, firstRowY + 50 - labelPushup, 50, 20 });
    osc2finePitchLabel.setBounds({ 315 + 90, firstRowY + 50 - labelPushup, 50, 20 });
    osc2panLabel.setBounds({ 315 + 150, firstRowY + 50 - labelPushup, 50, 20 });
    osc2phaseOffsetLabel.setBounds({ 315 + 210, firstRowY + 50 - labelPushup, 50, 20 });
    osc2volLabel.setBounds({ 315 + 270, firstRowY + 50 - labelPushup, 50, 20 });

    volEnvMainLabel.setBounds({ 365, secondHalfY - 5, 130, 40 });
    volEnvAttackLabel.setBounds({ 370 - 5, secondHalfY + 80 - labelPushup, lowerHalfLabelWidth, 20 });
    volEnvDecayLabel.setBounds({ 435 - 4, secondHalfY + 80 - labelPushup, lowerHalfLabelWidth, 20 });
    volEnvSustainLabel.setBounds({ 360 + 5, secondHalfY + 157 - labelPushup - 10, lowerHalfLabelWidth, 20 });
    volEnvReleaseLabel.setBounds({ 430 + 1, secondHalfY + 157 - labelPushup - 10, lowerHalfLabelWidth, 20 });

    filterEnvMainLabel.setBounds({ 165, secondHalfY - 5, 130, 40 });
    filterEnvAttackLabel.setBounds({ 160 + 4 - 2, secondHalfY + 80 - labelPushup, lowerHalfLabelWidth, 20 });
    filterEnvDecayLabel.setBounds({ 220 + 4 - 2, secondHalfY + 80 - labelPushup, lowerHalfLabelWidth, 20 });
    filterEnvSustainLabel.setBounds({ 160 + 1, secondHalfY + 157 - labelPushup - 10, lowerHalfLabelWidth, 20 });
    filterEnvReleaseLabel.setBounds({ 220 + 1, secondHalfY + 157 - labelPushup - 10, lowerHalfLabelWidth, 20 });
    filterEnvAmountLabel.setBounds({ 275 + 2, secondHalfY + 130 - labelPushup - 5, 50, 20 });

    filterMainLabel.setBounds({ 20, secondHalfY - 5, 130, 40 });
    filterCutoffFrequencyLabel.setBounds({ 30 - 2, secondHalfY + 135 - labelPushup, 50, 20 });
    filterResonanceLabel.setBounds({ 95 - 2, secondHalfY + 135 - labelPushup, lowerHalfLabelWidth, 20 });

    LFOMainLabel.setBounds({ 550 - 10 - 20, secondHalfY - 5, 130, 40 });
    LFOAmntLabel.setBounds({ 550 - 10 - 20, secondHalfY + 135 - labelPushup, lowerHalfLabelWidth, 20 });
    LFORateLabel.setBounds({ 620 - 10 - 20, secondHalfY + 135 - labelPushup + 2, lowerHalfLabelWidth, 15 });

}

//================================================================================================



SymmetricalRotaryLookAndFeel::SymmetricalRotaryLookAndFeel(OtherRotaryLookAndFeel& pfilterEnvLF,
    NEASynthesiserAudioProcessorEditor& ppluginEditor) :
    filterEnvSmallLookAndFeel(pfilterEnvLF),
    pluginEditor(ppluginEditor)
{}

void SymmetricalRotaryLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s)
{
    //if the width != height, we don't want to stretch the knob but instead still keep it a circle
    //so find the min of width and height and draw the knob in only that square region
    float radius = juce::jmin<float>(width, height) / 2;
    float centreX = x + width / 2;      //centre of the circle
    float centreY = y + height / 2;

    //angle of the slider thumb (the large blue circle on the default one)
    float sliderAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    //making the knob slightly smaller as it won't fit within it's square at maximum size
    float adjustedRadius = radius - (radius / 2.7f);
    float dialTickSpareAngle = 0.2f;
    auto thickness = 3.0f;          //thickness of the lines
    auto dialTickThickness = 2.0f;
    auto midAngle = rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle);  //angle at the middle

    juce::Path bluePath;
    bluePath.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
        0, juce::MathConstants<float>::twoPi, sliderAngle, true);       //from the middle to sliderAngle
    g.setColour(knobAqua);
    g.strokePath(bluePath, juce::PathStrokeType(thickness));

    juce::Path blackPath1;
    juce::Path blackPath2;
    //blackPath1 is the left line, blackPath2 is the right line

    juce::Path dialTick;
    dialTick.addRectangle(-dialTickThickness / 2, dialTickThickness / 2, dialTickThickness, adjustedRadius);

    if (sliderPosProportional >= 0.5) {
        auto blueAngle = sliderAngle - juce::MathConstants<float>::twoPi;

        //the furthest blackPath1 can go is to the midpoint
        float blackPath1EndAngle = juce::jmin<float>(sliderAngle - dialTickSpareAngle, midAngle);

        blackPath1.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, rotaryStartAngle,
            blackPath1EndAngle,
            true);

        blackPath2.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, sliderAngle + dialTickSpareAngle, rotaryEndAngle, true);
    }
    else
    {
        auto blueAngle = juce::MathConstants<float>::twoPi - sliderAngle;

        //the furthest blackPath2 can go is to the midpoint
        auto blackPath2StartAngle = juce::jmax<float>(sliderAngle + dialTickSpareAngle, midAngle);

        blackPath1.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, rotaryStartAngle,
            sliderAngle - dialTickSpareAngle,
            true);

        blackPath2.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, blackPath2StartAngle, rotaryEndAngle, true);
    }

    g.setColour(knobBlack);
    if (sliderAngle - rotaryStartAngle > dialTickSpareAngle)
        g.strokePath(blackPath1, juce::PathStrokeType(thickness));

    if (rotaryEndAngle - sliderAngle > dialTickSpareAngle)
        g.strokePath(blackPath2, juce::PathStrokeType(thickness));

    g.setColour(juce::Colour(knobFilledCircleColour));
    juce::Path filledCircle;
    filledCircle.addCentredArc(centreX, centreY, adjustedRadius - thickness + 1.3, adjustedRadius - thickness + 1.3,
        0, rotaryStartAngle, rotaryStartAngle + juce::MathConstants<double>::twoPi,
        true);

    g.fillPath(filledCircle);

    g.setColour(knobDialTickColour);
    g.fillPath(dialTick, juce::AffineTransform::rotation(sliderAngle + juce::MathConstants<float>::pi)
        .translated(centreX, centreY));

    juce::Path triangle;
    //specify points of the triangle as coordinates
    auto triangleWidth = 12.0f;
    auto triangleHeight = 4.0f;

    triangle.addTriangle(centreX, centreY - thickness - adjustedRadius,
        centreX + triangleWidth / 2, centreY - thickness - adjustedRadius - triangleHeight,
        centreX - triangleWidth / 2, centreY - thickness - adjustedRadius - triangleHeight);

    if (sliderPosProportional > 0.51 || sliderPosProportional < 0.49) { //if the knob is at 0, set the triangle colour black. else blue
        g.setColour(knobAqua);
        filterEnvSmallLookAndFeel.setActivated(true);
    }
    else {
        g.setColour(knobBlack);
        filterEnvSmallLookAndFeel.setActivated(false);
    }
    
    //repaint the relevant knobs to make their colours update in real-time
    pluginEditor.filterEnvAttack.repaint();
    pluginEditor.filterEnvDecay.repaint();
    pluginEditor.filterEnvSustain.repaint();
    pluginEditor.filterEnvRelease.repaint();

    g.fillPath(triangle);
}



//==================================================================================================



OtherRotaryLookAndFeel::OtherRotaryLookAndFeel(juce::Colour c) : activatedColour(c), isActivated(true)
{}

void OtherRotaryLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s)
{
    //if the width != height, we don't want to stretch the knob but instead still keep it a circle
    //so find the min of width and height and draw the knob in only that square region
    float radius = juce::jmin<float>(width, height) / 2;
    float centreX = x + width / 2;      //centre of the circle
    float centreY = y + height / 2;

    //angle of the slider thumb (the large blue circle on the default one)
    float sliderAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    //making the knob slightly smaller as it won't fit within it's square at maximum size
    float adjustedRadius = radius - (radius / 2.7f);
    float dialTickSpareAngle = 0.25f;
    auto thickness = 3.0f;          //thickness of the lines
    auto dialTickThickness = 2.0f;


    juce::Path colouredPath;
    colouredPath.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius, 0, rotaryStartAngle,
        sliderAngle, true);

    juce::Colour colour = isActivated ? activatedColour : deactivatedColour;

    g.setColour(colour);
    g.strokePath(colouredPath, juce::PathStrokeType(thickness));

    juce::Path blackPath;
    blackPath.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
        0, sliderAngle + dialTickSpareAngle, rotaryEndAngle, true);

    juce::Path dialTick;
    dialTick.addRectangle(-dialTickThickness / 2, dialTickThickness / 2, dialTickThickness, adjustedRadius);

    g.setColour(knobBlack);
    if (rotaryEndAngle - sliderAngle > dialTickSpareAngle)
        g.strokePath(blackPath, juce::PathStrokeType(thickness));

    g.setColour(juce::Colour(knobFilledCircleColour));
    juce::Path filledCircle;
    filledCircle.addCentredArc(centreX, centreY, adjustedRadius - thickness + 1.3, adjustedRadius - thickness + 1.3,
        0, rotaryStartAngle, rotaryStartAngle + juce::MathConstants<double>::twoPi,
        true);

    g.fillPath(filledCircle);

    //g.strokePath(j, juce::PathStrokeType(thickness));

    g.setColour(knobDialTickColour);
    g.fillPath(dialTick, juce::AffineTransform::rotation(sliderAngle + juce::MathConstants<float>::pi)
        .translated(centreX, centreY));
}

void OtherRotaryLookAndFeel::setActivated(bool b) {
    isActivated = b;
}



//======================================================================================================



void MainLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s)
{
    float radius = juce::jmin(width, height) / 2;
    float centreX = x + width / 2;
    float centreY = y + height / 2;

    float sliderAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    float adjustedRadius = radius - (radius / 2.7f);
    auto midAngle = rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle);
    float dialTickSpareAngle = 0.2f;
    auto thickness = 3.0f;
    auto dialTickThickness = 2.0f;

    // in the middle position, neither black lines are touching the dial tick. As such, with just a slight bit of rotation,
    // you would not want their angles to jump immediately and unnaturally meet the dial tick. instead, it should gradually
    // increase its angle more before meeting the dial tick later on.
    // this is the angle of rotation before the dark blue line meets the dial tick. 
    auto blackPathJoinAngle = 0.5f;

    juce::Path bluePath;
    //from the middle to sliderAngle. add thickness to the radius so that it is drawn above the other lines.
    bluePath.addCentredArc(centreX, centreY, adjustedRadius + thickness, adjustedRadius + thickness,
        0, juce::MathConstants<float>::twoPi, sliderAngle, true);
    g.setColour(knobAqua);
    g.strokePath(bluePath, juce::PathStrokeType(thickness));

    juce::Path darkBluePath;
    juce::Path blackPath1;
    juce::Path blackPath2;
    //blackPath1 is the left line, blackPath2 is the right line

    juce::Path dialTick;
    dialTick.addRectangle(-dialTickThickness / 2, dialTickThickness / 2, dialTickThickness, adjustedRadius);

    if (sliderPosProportional > 0.5) {
        auto blueAngle = sliderAngle - juce::MathConstants<float>::twoPi;

        //the furthest blackPath1 can go is to the midpoint, so take the lowest angle (most left) between
        //its normal position (sliderAngle - dialTickSpareAngle) and midAngle
        float blackPath1EndAngle = juce::jmin<float>(sliderAngle - dialTickSpareAngle, midAngle);

        // the dark blue line's end would initially be where the left black line left off 
        // (sliderAngle - dialTickSpareAngle), and then gradually increase until it meets the dial tick.
        // the (blueAngle / blackPathJoinAngle) ensures that it meets the dial tick at an angle of
        // blackPathJoinAngle, and the min() makes sure that the maximum can only be 1.0f so it cannot go beyond
        // the dial tick
        auto darkBlueEndAngle = juce::jmin((blueAngle / blackPathJoinAngle), 1.0f) * dialTickSpareAngle
            + sliderAngle - dialTickSpareAngle;

        blackPath1.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, rotaryStartAngle,
            blackPath1EndAngle,
            true);

        blackPath2.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, sliderAngle + dialTickSpareAngle, rotaryEndAngle, true);

        darkBluePath.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, blackPath1EndAngle, darkBlueEndAngle, true);
    }
    else if (sliderPosProportional <= 0.5)
    {
        //similar logic as above. note that either if-branch handles the case of sliderAngle == midAngle correctly
        auto blueAngle = juce::MathConstants<float>::twoPi - sliderAngle;
        auto blackPath2StartAngle = juce::jmax<float>(sliderAngle + dialTickSpareAngle, midAngle);
        auto darkBlueStartAngle = sliderAngle + dialTickSpareAngle
            - juce::jmin((blueAngle / blackPathJoinAngle), 1.0f) * dialTickSpareAngle;

        blackPath1.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, rotaryStartAngle,
            sliderAngle - dialTickSpareAngle,
            true);

        blackPath2.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, blackPath2StartAngle, rotaryEndAngle, true);

        darkBluePath.addCentredArc(centreX, centreY, adjustedRadius, adjustedRadius,
            0, darkBlueStartAngle, blackPath2StartAngle, true);
    }

    g.setColour(juce::Colour(0xff333154));
    g.strokePath(darkBluePath, juce::PathStrokeType(thickness));

    g.setColour(knobBlack);
    if (sliderAngle - rotaryStartAngle > dialTickSpareAngle)
        g.strokePath(blackPath1, juce::PathStrokeType(thickness));

    if (rotaryEndAngle - sliderAngle > dialTickSpareAngle)
        g.strokePath(blackPath2, juce::PathStrokeType(thickness));

    g.setColour(juce::Colour(knobFilledCircleColour));
    juce::Path filledCircle;
    filledCircle.addCentredArc(centreX, centreY, adjustedRadius - thickness + 1.3, adjustedRadius - thickness + 1.3,
        0, rotaryStartAngle, rotaryStartAngle + juce::MathConstants<double>::twoPi,
        true);

    g.fillPath(filledCircle);

    g.setColour(knobDialTickColour);
    g.fillPath(dialTick, juce::AffineTransform::rotation(sliderAngle + juce::MathConstants<float>::pi)
        .translated(centreX, centreY));

    juce::Path triangle;
    //specify points of the triangle as coordinates
    auto triangleWidth = 16.0f;
    auto triangleHeight = triangleWidth / 3.0f;
    triangle.addTriangle(centreX, centreY - (2 * thickness) - adjustedRadius,
        centreX + triangleWidth / 2, centreY - (2 * thickness) - adjustedRadius - triangleHeight,
        centreX - triangleWidth / 2, centreY - (2 * thickness) - adjustedRadius - triangleHeight);

    if (sliderPosProportional > 0.51 || sliderPosProportional < 0.49) { //if the knob is at 0, set the colour black. else blue
        g.setColour(knobAqua);
    }
    else {
        g.setColour(knobBlack);
    }

    g.fillPath(triangle);
}

void MainLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
    int, int, int, int, juce::ComboBox& box) 
{
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    //g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.setColour(juce::Colour(0xff262730));
    g.fillRect(boxBounds.toFloat());

    //g.setColour(juce::Colour(0xffa9a9a9));
    g.setColour(juce::Colour(0xff8e989b));

    g.drawRect(boxBounds.toFloat(), 1.2f);

    juce::Rectangle<int> arrowZone(width - 30, 0, 20, height);
    juce::Path path;
    path.startNewSubPath((float)arrowZone.getX() + 3.0f, (float)arrowZone.getCentreY() - 2.0f);
    path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f);
    path.lineTo((float)arrowZone.getRight() - 3.0f, (float)arrowZone.getCentreY() - 2.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(0.9f));
    g.fillPath(path);
}

void MainLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    const bool isSeparator, const bool isActive,
    const bool isHighlighted, const bool isTicked,
    const bool hasSubMenu, const juce::String& text,
    const juce::String& shortcutKeyText,
    const juce::Drawable* icon, const juce::Colour* const textColourToUse)
{
    using namespace juce;

    auto textColour = (textColourToUse == nullptr ? findColour(PopupMenu::textColourId)
        : *textColourToUse);

    auto r = area.reduced(1);

    if (isHighlighted && isActive)
    {
        g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
        g.fillRect(r);

        g.setColour(findColour(PopupMenu::highlightedTextColourId));
    }
    else
    {
        g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
    }

    r.reduce(jmin(5, area.getWidth() / 20), 0);

    auto font = getPopupMenuFont();

    auto maxFontHeight = (float)r.getHeight() / 1.3f;

    if (font.getHeight() > maxFontHeight)
        font.setHeight(maxFontHeight);

    g.setFont(font);

    //auto iconArea = r.removeFromLeft(roundToInt(maxFontHeight)).toFloat();

    //if (icon != nullptr)
    //{
    //    icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        //r.removeFromLeft(roundToInt(maxFontHeight * 0.5f));
    //}

    //r.removeFromRight(3);
    g.drawFittedText(text, r, Justification::centredLeft, 1);
}



//===============================================================================================



void LinearSlider::paint(juce::Graphics& g) {
    auto triangleTickHeight = 9.0f;
    auto triangleTickWidth = 8.0f;

    auto gap = 2.5;   //gap between the blue rectangle and the slider outline
    float adjustedWidth = this->getWidth() / 2;
    float adjustedHeight = this->getHeight() - triangleTickWidth / 2;
    auto outlineX = 0.0f;
    auto outlineY = (this->getHeight() - adjustedHeight) / 2.0f;

    //width and height of blue rectangle
    auto blueWidth = adjustedWidth - 2 * gap;
    auto blueHeight = adjustedHeight - 2 * gap;

    //current value of the slider between 0.0 and 1.0
    float adjustedValue = this->getValue() / 1.0f;
    float blueY = blueHeight - (blueHeight * adjustedValue) + gap + outlineY;

    //draw blue rectangle
    g.setColour(knobAqua);
    g.fillRect(outlineX + gap, blueY, blueWidth, blueHeight * adjustedValue);

    //draw outline
    g.setColour(juce::Colour(0xffA9A9A9));
    //g.setColour(juce::Colour(0xffd9d9d9));
    g.drawRect(outlineX, outlineY, adjustedWidth, adjustedHeight, 1.5f);

    juce::Path triangleTick;
    triangleTick.addTriangle(outlineX + adjustedWidth + 1, blueY,
        outlineX + 1 + adjustedWidth + triangleTickHeight, blueY + triangleTickWidth / 2,
        outlineX + 1 + adjustedWidth + triangleTickHeight, blueY - triangleTickWidth / 2);

    //draw triangle tick
    g.fillPath(triangleTick);
}



//==================================================================================================



LFORotaryLookAndFeel::LFORotaryLookAndFeel(OtherRotaryLookAndFeel& pdependentLookAndFeel, juce::Slider& pdependentSlider
    , juce::Colour colour)
    : OtherRotaryLookAndFeel(colour), dependentLookAndFeel(pdependentLookAndFeel), dependentSlider(pdependentSlider)
{}


void LFORotaryLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) 
{
    //draw the knob as normal
    OtherRotaryLookAndFeel::drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, s);

    if (sliderPosProportional == 0) {
        //if this knob is at zero, grey the dependent one out (by deactivating it). else, keep it activated
        dependentLookAndFeel.setActivated(false);
    } else {
        dependentLookAndFeel.setActivated(true);
    }
    dependentSlider.repaint();
}
