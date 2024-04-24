/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class FFTSpectrumAnalyzerAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    FFTSpectrumAnalyzerAudioProcessorEditor(FFTSpectrumAnalyzerAudioProcessor&);
    ~FFTSpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void getBounds();
    void setPlotIndex(int plotIndex);
   // void updateToggleState(int plotId);
    void setPlotVisibility(int plotId);
    void handleNewSelection(int numBins, int rowSize, int rowIndex);
    void setWindow(juce::dsp::WindowingFunction<float>::WindowingMethod type);

    //processBlock integration
    void processBuffer();
    void zeroBuffers();
    void zeroBinSelection();

    void mouseMove(const juce::MouseEvent& event) override;
    int findPeak();
    float screenToGraph(float screenCoord);
    float graphToScreen(int graphCoord);
    float getYCoord(int plotNumber, bool log, int index);
    float calculateX(bool log, int index);
    float calculateY(int plotSelection, int index);
    bool inBounds(float x, float y);
    void setFreqData(int fftData);
    juce::Colour setColor(int row);
    void initializeBinMag();
    void setWindowFunction();
    void setBlockSize();
    void setAxisType();
    std::string floatToStringPrecision(float f, int p);


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FFTSpectrumAnalyzerAudioProcessor& audioProcessor;
    static bool isRunning;
    juce::Label cursorPlot;
    juce::Label peakPlot;

    // gui elements start
    juce::Label gui_importAudio;
    juce::Label gui_selectTrace;
    juce::Label gui_zoom;
    juce::Label gui_upper;
    juce::Label gui_lower;
    juce::Label gui_xUpper;
    juce::Label gui_yUpper;
    juce::Label gui_xLower;
    juce::Label gui_yLower;
    juce::Label gui_export;
    juce::TextButton gui_exportButton{ "Export .csv" };
    juce::Label cursorLabel;
    juce::Label peakLabel;
    juce::Label windowLabel;
    juce::Label axisLabel;
    juce::Label sizeLabel;
    juce::ComboBox windowFunction;
    juce::ComboBox axis;
    juce::ComboBox size;
    // gui elements end

    juce::Label inputXmin;
    juce::Label inputXmax;
    juce::Label inputYmin;
    juce::Label inputYmax;

    juce::Label labelPlot1{ "Plot 1" };
    juce::Label labelPlot2{ "Plot 2" };
    juce::Label labelPlot3{ "Plot 3" };
    juce::Label labelPlot4{ "Plot 4" };
    juce::Label labelPlot5{ "Plot 5" };
    juce::Label labelPlot6{ "Plot 6" };
    juce::Label labelPlot7{ "Plot 7" };
   
    juce::TextButton buttonPlot1{ "Selected" };
    juce::TextButton buttonPlot2{ "Select" };
    juce::TextButton buttonPlot3{ "Select" };
    juce::TextButton buttonPlot4{ "Select" };
    juce::TextButton buttonPlot5{ "Select" };
    juce::TextButton buttonPlot6{ "Select" };
    juce::TextButton buttonPlot7{ "Select" };
    
    juce::ToggleButton toggleButtonPlot1;
    juce::ToggleButton toggleButtonPlot2;
    juce::ToggleButton toggleButtonPlot3;
    juce::ToggleButton toggleButtonPlot4;
    juce::ToggleButton toggleButtonPlot5;
    juce::ToggleButton toggleButtonPlot6;
    juce::ToggleButton toggleButtonPlot7;
 
    static bool isVisiblePlot1;
    static bool isVisiblePlot2;
    static float xMinPrev;
    static float xMin;
    static float xMinFrequency;
    static float xMaxPrev;
    static float xMax;
    static float xMaxFrequency;
    static float yMinPrev;
    static float yMin;
    static float yMaxPrev;
    static float yMax;
    static int plotIndexSelection;
    static float cursorX;
    static float cursorY;
    static int cursorIndex;
    static int cursorPeak;
    static float xMaxFrequency;
    static float xMinFrequency;

    static int windowWidth;
    static int windowHeight;
    static int windowMaxWidth;
    static int windowMaxHeight;
    static int fftSize;
    static int numBins;
    // static int sampleRate;
    static int maxFreq;
    static int numFreqBins;
    static int rowIndex;
    static int rowSize;
    static int fftCounter;
    static int stepSize;



    static int count;
    static int countPrev;

   

    struct plotItem {
        bool isVisible;
        juce::Colour color;
        juce::Path path;
        int checkBoxPos;
    };
    
    static plotItem plotInfo[7];

    static bool setToLog;
    static int initialAxisState;
    static bool newSelection;
    static bool displayError;
    static bool conCall;
    static bool blockProcessed;

    //ProcessBlock 
    //static juce::dsp::FFT editFFT;
    static juce::dsp::WindowingFunction<float> window;
    static std::vector<float> bufferRight;
    static std::vector<float> bufferLeft;
    static std::vector<float> windowBufferRight;
    static std::vector<float> windowBufferLeft;
    static std::vector<float> indexToFreqMap;
    static std::vector<std::vector<float>> binMag;
    static std::vector<std::vector<float>> sampleSelections;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTSpectrumAnalyzerAudioProcessorEditor)
};