/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

//==============================================================================
FFTSpectrumAnalyzerAudioProcessorEditor::FFTSpectrumAnalyzerAudioProcessorEditor (FFTSpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setOpaque(true);
    setSize (1600, 1000);

    startTimer(500);
}

FFTSpectrumAnalyzerAudioProcessorEditor::~FFTSpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================

void FFTSpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll(juce::Colours::black);


    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    //g.setColour(juce::Colours::white);
    //g.setFont(15.0f);

    g.setOpacity(1.0f);
    g.setColour(juce::Colours::white);

    int lineHeight = 10; // Adjust line height as needed
    int yPosition = 10; // Start position from the top
    int xPosition = 10; // Start position from the left

    //g.drawFittedText(str, getLocalBounds(), juce::Justification::centred, 1);
    const int scopeSize = audioProcessor.getScopeSize();
    const float* scopeData = audioProcessor.getScopeData();

    const float* fft = audioProcessor.getFFT();

    int lineHeight = 10; // Adjust line height as needed
    int yPosition = 10; // Start position from the top
    int xPosition = 10; // Start position from the left
    int textOffset = 100;
    int startYPlot1 = 250;
    int startYPlot2 = 750;
    float offsetX = 100; // Offset X position
    float scaleX = 10; // Scaling X increments
    float scaleY = -50; // Scaling Y increments
    float sampleSize = 100; // Adjust the number of samples being displayed as needed
    juce::Path plot1;
    juce::Path plot2;

    juce::Path xAxisPlot1;
    juce::Path xAxisPlot2;
    juce::Path xAxisMarkersPlot1;
    juce::Path xAxisMarkersPlot2;
juce::PathyAxisPlot1juce::PathyAxisPlot2juce::PathyAxisMarkersUpPlot1    juce::Path yAxisMarkersDownPlot1;juce::PathyAxisMarkersUpPlot2;juce::PathyAxisMarkersDownPlot2Axi variableslengthXAxis1    float lengthYAxis = 200;floatscaleXMarker= 50;floatscaleYMarker = 50intnumXMarkers=20intnumYMarkers = 3
    myPath.startNewSubPath(offsetX, offsetY + scopeData[0]); //observe closely
    for (int i = 1; i < sampleSize; i++)
    {
        myPath.lineTo(i * scaleX + offsetX, FlipYAxisValue * scopeData[i] * scaleY + offsetY);
    }

    // Graph plots
    plot1.startNewSubPath(offsetX, startYPlot1 + scopeData[0]);
    plot2.startNewSubPath(offsetX, startYPlot2 + *(scopeData + 1 * scopeSize));
    for (int i = 1; i < sampleSize; i++)
    {
        plot1.lineTo(i * scaleX + offsetX, *((scopeData + i) + 0 * scopeSize) * scaleY + startYPlot1);
        plot2.lineTo(i * scaleX + offsetX, *((scopeData + i) + 1 * scopeSize) * scaleY + startYPlot2);
    }
    g.strokePath(plot1, juce::PathStrokeType(5.0f));
    g.strokePath(plot2, juce::PathStrokeType(5.0f));

    // Plot x-axis
    xAxisPlot1.startNewSubPath(offsetX, startYPlot1);
    xAxisPlot1.lineTo(offsetX + lengthXAxis, startYPlot1);
    xAxisPlot2.startNewSubPath(offsetX, startYPlot2);
    xAxisPlot2.lineTo(offsetX+lengthXAxis, startYPlot2)g.strokePath(xAxisPlot1,juc:PathStrokeType(2.0f));    gstrokePathxAxisPlot2juce:PathStrokeType(2.0f));Ploty-axis    yAxisPlot1.startNewSubPath(offsetX, startYPlot1);    yAxisPlot1.lineTo(offsetX, startYPlot1 + lengthYAxis);  // line drawn up from midpoint
    yAxisPlot1.lineTo(offsetX, startYPlot1 - lengthYAxis);  // line drawn down from midpoint
    yAxisPlot2.startNewSubPath(offsetX, startYPlot2);
    yAxisPlot2.lineTo(offsetX, startYPlot2 + lengthYAxis);  // line drawn up from midpoint
    yAxisPlot2.lineTo(offsetX, startYPlot2 - lengthYAxis);  // line drawn down from midpoint
    g.strokePath(yAxisPlot1, juce::PathStrokeType(2.0f));
    g.strokePath(yAxisPlot2, juce::PathStrokeType(2.0f));

    // Plot X Axis Markers
    for (int i = 1; i < numXMarkers; i++) {
        xAxisMarkersPlot1.startNewSubPath(offsetX + scaleXMarker + (i * scaleXMarker), startYPlot1 - 5);
        xAxisMarkersPlot1.lineTo(offsetX + scaleXMarker + (i * scaleXMarker), startYPlot1 + 5);
        xAxisMarkersPlot2.startNewSubPath(offsetX + scaleXMarker + (i * scaleXMarker), startYPlot2 - 5);
        xAxisMarkersPlot2.lineTo(offsetX + scaleXMarker + (i * scaleXMarker), startYPlot2 + 5);
    }
    g.strokePath(xAxisMarkersPlot1, juce::PathStrokeType(2.0f));
    g.strokePath(xAxisMarkersPlot2, juce::PathStrokeType(2.0f));
    
   
    // Plot Y Axis Markers
    for (int i = 1; i <= numYMarkers; i++) {
        yAxisMarkersUpPlot1.startNewSubPath(offsetX - 5, startYPlot1 - (scaleYMarker * i));
        yAxisMarkersUpPlot1.lineTo(offsetX + 5, startYPlot1 - (scaleYMarker * i));  // drawing line markers moving up from midpoint
        yAxisMarkersDownPlot1.startNewSubPath(offsetX - 5, startYPlot1 + (scaleYMarker * i));
        yAxisMarkersDownPlot1.lineTo(offsetX + 5, startYPlot1 + (scaleYMarker * i));  // drawing line markers moving down from midpoint

        yAxisMarkersUpPlot2.startNewSubPath(offsetX - 5, startYPlot2 - (scaleYMarker * i));
        yAxisMarkersUpPlot2.lineTo(offsetX + 5, startYPlot2 - (scaleYMarker * i));  // drawing line markers moving up from midpoint
        yAxisMarkersDownPlot2.startNewSubPath(offsetX - 5, startYPlot2 + (scaleYMarker * i));
        yAxisMarkersDownPlot2.lineTo(offsetX + 5, startYPlot2 + (scaleYMarker * i));  // drawing line markers moving down from midpoint
    }
    g.strokePath(yAxisMarkersUpPlot1, juce::PathStrokeType(2.0f));
    g.strokePath(yAxisMarkersDownPlot1, juce::PathStrokeType(2.0f));
    g.strokePath(yAxisMarkersUpPlot2, juce::PathStrokeType(2.0f));
    g.strokePath(yAxisMarkersDownPlot2, juce::PathStrokeType(2.0f));
}

void FFTSpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    if (!isRunning && audioProcessor.getProcBlockCalled()){
        isRunning = true;
        audioProcessor.resetProcBlockCalled();
    }
    else if (isRunning && !audioProcessor.getProcBlockCalled()){
        isRunning = false;
        repaint();
        audioProcessor.resetScopeDataIndex();
        audioProcessor.incrementPlotIndex();
    }
}

void FFTSpectrumAnalyzerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
