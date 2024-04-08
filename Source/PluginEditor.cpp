/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

bool FFTSpectrumAnalyzerAudioProcessorEditor::isRunning = false;
bool FFTSpectrumAnalyzerAudioProcessorEditor::isVisiblePlot1 = true;
bool FFTSpectrumAnalyzerAudioProcessorEditor::isVisiblePlot2 = true;
bool FFTSpectrumAnalyzerAudioProcessorEditor::isGraph = false;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMinPrev = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMin = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMaxPrev = 100;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMax = 100;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMinPrev = -1;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMin = -1;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMaxPrev = 1;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMax = 1;
int FFTSpectrumAnalyzerAudioProcessorEditor::plotIndexSelection = 0;

//ROW INDEX STUFF!!!
int FFTSpectrumAnalyzerAudioProcessorEditor::rowSize = 2;
int FFTSpectrumAnalyzerAudioProcessorEditor::rowIndex = 0;

//Processor statics
int FFTSpectrumAnalyzerAudioProcessorEditor::fftS = 1024;
int FFTSpectrumAnalyzerAudioProcessorEditor::numBins = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::maxFreq = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::numFreqBins = 0;

// gui elements
const int width_primaryCategoryLabel = 275;
const int height_primaryCategoryLabel = 25;
const int width_secondaryLabel = 150;
const int height_secondaryLabel = 25;
const int yOffsetPrimary_secondaryLabel = 8;
const int x_componentOffset = 6;
const int y_componentOffset = 6;
const int yOffset_selectionBox = 2;
const int width_selectionBox = 263;
const int height_selectionBox = 90;

// window dimensions 
int FFTSpectrumAnalyzerAudioProcessorEditor::windowWidth = 950;
int FFTSpectrumAnalyzerAudioProcessorEditor::windowHeight = 550 + 2;
int FFTSpectrumAnalyzerAudioProcessorEditor::windowMaxWidth = 2160;
int FFTSpectrumAnalyzerAudioProcessorEditor::windowMaxHeight = 1080;

// cursor elements
int FFTSpectrumAnalyzerAudioProcessorEditor::graphWest = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::graphEast = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::graphNorth = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::graphSouth = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::cursorX = 0;
float FFTSpectrumAnalyzerAudioProcessorEditor::cursorYPeak = 0;

//Processor vectors
std::vector<float> FFTSpectrumAnalyzerAudioProcessorEditor::indexToFreqMap = { 0 };
std::vector< std::vector<float>> FFTSpectrumAnalyzerAudioProcessorEditor::binMag;

//==============================================================================
FFTSpectrumAnalyzerAudioProcessorEditor::FFTSpectrumAnalyzerAudioProcessorEditor(FFTSpectrumAnalyzerAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setOpaque(true);
	startTimer(500);
	setSize(windowWidth, windowHeight);
	setResizable(true, true);
	setResizeLimits(windowWidth, windowHeight, windowMaxWidth, windowMaxHeight);

	juce::dsp::WindowingFunction<float>::WindowingMethod windowType = juce::dsp::WindowingFunction<float>::WindowingMethod::hann;
	audioProcessor.setWindow(windowType);

	// new gui elements
	addAndMakeVisible(gui_importAudio);
	gui_importAudio.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
	gui_importAudio.setText("Import Audio", juce::dontSendNotification);
	gui_importAudio.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);

	addAndMakeVisible(gui_selectTrace);
	gui_selectTrace.setText("Selected Traces", juce::dontSendNotification);
	gui_selectTrace.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_zoom);
	gui_zoom.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
	gui_zoom.setText("Zoom", juce::dontSendNotification);
	gui_zoom.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);

	addAndMakeVisible(gui_upper);
	gui_upper.setText("Upper", juce::dontSendNotification);
	gui_upper.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_lower);
	gui_lower.setText("Lower", juce::dontSendNotification);
	gui_lower.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_xUpper);
	gui_xUpper.setText("X", juce::dontSendNotification);
	gui_xUpper.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_yUpper);
	gui_yUpper.setText("Y", juce::dontSendNotification);
	gui_yUpper.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_xLower);
	gui_xLower.setText("X", juce::dontSendNotification);
	gui_xLower.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_yLower);
	gui_yLower.setText("Y", juce::dontSendNotification);
	gui_yLower.setFont(juce::Font(17.0f));

	addAndMakeVisible(gui_export);
	gui_export.setText("Export", juce::dontSendNotification);
	gui_export.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
	gui_export.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);

	addAndMakeVisible(gui_exportButton);
	gui_exportButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
	gui_exportButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
	gui_exportButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

	addAndMakeVisible(cursorLabel);
	cursorLabel.setText("Cursor", juce::dontSendNotification);
	cursorLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
	cursorLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	addAndMakeVisible(peakLabel);
	peakLabel.setText("Peak", juce::dontSendNotification);
	peakLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
	peakLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	addAndMakeVisible(windowLabel);
	windowLabel.setText("Function", juce::dontSendNotification);
	windowLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
	windowLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	addAndMakeVisible(axisLabel);
	axisLabel.setText("Axis", juce::dontSendNotification);
	axisLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
	axisLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	addAndMakeVisible(sizeLabel);
	sizeLabel.setText("Size", juce::dontSendNotification);
	sizeLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
	sizeLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	addAndMakeVisible(windowFunction);
	windowFunction.addItem("Blackman window", 1);
	windowFunction.addItem("Blackman-Harris window", 2);
	windowFunction.addItem("Flatop window", 3);
	windowFunction.addItem("Hamming window", 4);
	windowFunction.addItem("Hann window", 5);
	windowFunction.addItem("Kaiser", 6);
	windowFunction.addItem("Rectangular window", 7);
	windowFunction.addItem("Triangular window", 8);
	windowFunction.setSelectedId(5);
	windowFunction.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
	windowFunction.setColour(juce::ComboBox::textColourId, juce::Colours::black);
	windowFunction.setColour(juce::ComboBox::arrowColourId, juce::Colours::darkgrey);
	windowFunction.onChange = [this] { setWindowFunction(); };

	addAndMakeVisible(axis);
	axis.addItem("Linear Frequency", 1);
	axis.addItem("Log Frequency", 2);
	axis.setSelectedId(2);
	axis.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
	axis.setColour(juce::ComboBox::textColourId, juce::Colours::black);
	axis.setColour(juce::ComboBox::arrowColourId, juce::Colours::darkgrey);
	axis.onChange = [this] { setAxisScale(); };

	addAndMakeVisible(size);
	size.addItem("128", 1);
	size.addItem("256", 2);
	size.addItem("512", 3);
	size.addItem("1024", 4);
	size.addItem("2048", 5);
	size.addItem("4096", 6);
	size.addItem("8192", 7);
	size.addItem("16384", 8);
	size.addItem("32768", 9);
	size.addItem("65536", 10);
	size.addItem("131072", 11);
	size.setSelectedId(4);
	size.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
	size.setColour(juce::ComboBox::textColourId, juce::Colours::black);
	size.setColour(juce::ComboBox::arrowColourId, juce::Colours::darkgrey);
	size.onChange = [this] { setSampleSize(); };

	addAndMakeVisible(cursorFunction);
	cursorFunction.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	cursorFunction.setColour(juce::Label::textColourId, juce::Colours::black);

	addAndMakeVisible(peakFunction);
	peakFunction.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	peakFunction.setColour(juce::Label::textColourId, juce::Colours::black);

	// old gui elements
	addAndMakeVisible(buttonPlot1);
	buttonPlot1.setClickingTogglesState(true);
	buttonPlot1.onClick = [&]()
		{
			plotIndexSelection = 0;
			setPlotIndex(0);
		};
	addAndMakeVisible(buttonPlot2);
	buttonPlot2.setClickingTogglesState(true);
	buttonPlot2.onClick = [&]()
		{
			plotIndexSelection = 1;
			setPlotIndex(1);
		};

	addAndMakeVisible(toggleButtonPlot1);
	toggleButtonPlot1.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
	toggleButtonPlot1.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::lightgrey);
	if (isVisiblePlot1 == true)
	{
		toggleButtonPlot1.setToggleState(true, true);
	}
	toggleButtonPlot1.onClick = [this] { updateToggleState(1); };
	toggleButtonPlot1.setClickingTogglesState(true);

	addAndMakeVisible(toggleButtonPlot2);
	toggleButtonPlot2.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
	toggleButtonPlot2.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::lightgrey);
	if (isVisiblePlot2 == true)
	{
		toggleButtonPlot2.setToggleState(true, true);
	}
	toggleButtonPlot2.onClick = [this] { updateToggleState(2); };
	toggleButtonPlot2.setClickingTogglesState(true);

	addAndMakeVisible(inputXmin);
	addAndMakeVisible(inputXmax);
	addAndMakeVisible(inputYmin);
	addAndMakeVisible(inputYmax);

	addAndMakeVisible(labelPlot1);
	addAndMakeVisible(labelPlot2);

	inputXmin.setEditable(true);
	inputXmax.setEditable(true);
	inputYmin.setEditable(true);
	inputYmax.setEditable(true);
	labelPlot1.setEditable(false);
	labelPlot2.setEditable(false);

	inputXmin.setText(std::to_string(xMin), juce::dontSendNotification);
	inputXmax.setText(std::to_string(xMax), juce::dontSendNotification);
	inputYmin.setText(std::to_string(yMin), juce::dontSendNotification);
	inputYmax.setText(std::to_string(yMax), juce::dontSendNotification);

	labelPlot1.setText("Plot 1", juce::dontSendNotification);
	labelPlot1.setColour(juce::Label::textColourId, juce::Colours::darkgrey);
	labelPlot2.setText("Plot 2", juce::dontSendNotification);
	labelPlot2.setColour(juce::Label::textColourId, juce::Colours::darkgrey);

	inputXmin.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	inputXmin.setColour(juce::Label::textColourId, juce::Colours::black);
	inputXmin.setColour(juce::Label::textWhenEditingColourId, juce::Colours::black);

	inputXmax.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	inputXmax.setColour(juce::Label::textColourId, juce::Colours::black);
	inputXmax.setColour(juce::Label::textWhenEditingColourId, juce::Colours::black);

	inputYmin.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	inputYmin.setColour(juce::Label::textColourId, juce::Colours::black);
	inputYmin.setColour(juce::Label::textWhenEditingColourId, juce::Colours::black);

	inputYmax.setColour(juce::Label::backgroundColourId, juce::Colours::white);
	inputYmax.setColour(juce::Label::textColourId, juce::Colours::black);
	inputYmax.setColour(juce::Label::textWhenEditingColourId, juce::Colours::black);

	inputXmin.onTextChange = [this] { getBounds(); };
	inputXmax.onTextChange = [this] { getBounds(); };
	inputYmin.onTextChange = [this] { getBounds(); };
}

FFTSpectrumAnalyzerAudioProcessorEditor::~FFTSpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void FFTSpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
	g.setOpacity(1.0f);
	g.setColour(juce::Colours::white);

	//const int scopeSize = audioProcessor.getScopeSize();
	//const int plotSize = audioProcessor.getPlotSize();
	//const int plotIndex = audioProcessor.getPlotIndex();
	//const float* scopeData = audioProcessor.getScopeData();
	//const float* fft = audioProcessor.getFFT();

	//PROCESSOR CLASS CODE!!!!!!!!!
	rowIndex = 0;
	rowSize = 2;

	int sampleRate = audioProcessor.getBlockSampleRate();
	setFreqData(fftS, sampleRate);
	audioProcessor.setFFTSize(fftS);
	audioProcessor.zeroAllSelections(numBins, rowSize);      //this is used for the hardcoded 2 selections
	audioProcessor.prepSelection(numBins, rowSize, rowIndex);


	//std::string rate = std::to_string(sampleRate);
	setFreqData(fftS, sampleRate);

	juce::dsp::WindowingFunction<float>::WindowingMethod windowType = juce::dsp::WindowingFunction<float>::WindowingMethod::hann;
	audioProcessor.setWindow(windowType);

	//x variable for labeling
	for (int i = 0; i < numBins; i++) {
		indexToFreqMap[i] = i * ((float)maxFreq / (float)numFreqBins);
	}

	int fftCounter = audioProcessor.getFFTCounter();
	//std::string counter = std::to_string(fftCounter);

	binMag = audioProcessor.getBinMag();

	for (int i = 0; i < numBins; i++) {
		binMag[rowIndex][i] /= fftCounter;
	}

	juce::Path plot1;
	juce::Path plot2;
	juce::Path xAxis;
	juce::Path xAxisMarkers;
	juce::Path yAxis;
	juce::Path yAxisMarkersUp;
	juce::Path yAxisMarkersDown;
	juce::Path zeroTick;

	//** graph scaling variables **//
	float border_xBuffer = getWidth() * 0.295;
	float border_yBuffer = y_componentOffset;
	float widthBorder = getWidth() - x_componentOffset;
	float heightBorder = getHeight() - 240;
	float xBuffer = border_xBuffer + 2;
	float yBuffer = border_yBuffer + 12;
	float lengthXAxis = widthBorder;
	float lengthYAxis = heightBorder * .95;
	float yStartXYAxis = yBuffer + lengthYAxis - 1;
	float xStartXYAxis = xBuffer - 3;
	float yStartPlot = (yBuffer + lengthYAxis) / 2;
	float xAxisScale_plotAndMarker = 0.702;

	/*
	// Paint values for plotting
	float xBuffer = getWidth() * 0.10;
	float yBuffer = getHeight() * 0.05;
	float lengthXAxis = getWidth() * 0.80;
	float lengthYAxis = getHeight() * 0.80;
	float yStartXYAxis = yBuffer + lengthYAxis;
	float xStartXYAxis = xBuffer;
	float yStartPlot = yBuffer + lengthYAxis / 2;
	*/

	float xDiff = xMax - xMin;
	if (xDiff <= 0)  // handles divide by zero errors
	{
		xMax = xMaxPrev;
		xMin = xMinPrev;
		xDiff = xMaxPrev - xMinPrev;
		inputXmin.setText(std::to_string(xMinPrev), juce::dontSendNotification);
		inputXmax.setText(std::to_string(xMaxPrev), juce::dontSendNotification);
	}
	else
	{
		xMaxPrev = xMax;
		xMinPrev = xMin;
	}
	float scaleX = lengthXAxis / xDiff;  // Scaling X increments; pixels shown per sample
	float xShift = -xMin * scaleX;

	float yDiff = yMax - yMin;
	if (yDiff <= 0)  // handles divide by zero errors
	{
		yMax = yMaxPrev;
		yMin = yMinPrev;
		yDiff = yMaxPrev - yMinPrev;
		inputYmin.setText(std::to_string(yMinPrev), juce::dontSendNotification);
		inputYmax.setText(std::to_string(yMaxPrev), juce::dontSendNotification);
	}
	else
	{
		yMaxPrev = yMax;
		yMinPrev = yMin;
	}
	float scaleY = -lengthYAxis / yDiff;  // Scaling Y increments; pixels shown per sample
	float yShift = (yDiff - 2.0f * yMax) * scaleY / 2.0f;

	float plotYShift = yStartPlot + yShift;

	// Graph plots

	plot2.startNewSubPath(xStartXYAxis + xShift, yStartPlot + (binMag[1][0]) * scaleY + yShift);
	//plot2.startNewSubPath(xStartXYAxis + xShift, yStartPlot + *(scopeData + 1 * scopeSize) * scaleY + yShift);
	plot1.startNewSubPath(xStartXYAxis + xShift, yStartPlot + binMag[0][0] * scaleY + yShift);  // Xmin needs to be the new startXPlot; this will be reset by the bounds read in to xMin textEntry box
	for (int i = 1; i <= sampleSize; i++)
	{
		if (isVisiblePlot2 == true) {
			plot2.lineTo(xAxisScale_plotAndMarker * i * scaleX + xStartXYAxis + xShift, (binMag[1][0]) * scaleY + plotYShift);
			//plot2.lineTo(i * scaleX + xStartXYAxis + xShift, *((scopeData + i) + 1 * scopeSize) * scaleY + plotYShift);
		}
		if (isVisiblePlot1 == true) {
			plot1.lineTo(xAxisScale_plotAndMarker * i * scaleX + xStartXYAxis + xShift, (binMag[0][0] + 0 * numBins) * scaleY + plotYShift);
			//plot1.lineTo(i * scaleX + xStartXYAxis + xShift, *((scopeData + i) + 0 * scopeSize) * scaleY + plotYShift);
		}
	}

	g.setColour(juce::Colours::lightgreen);
	g.strokePath(plot2, juce::PathStrokeType(3.0f));
	g.setColour(juce::Colours::cornflowerblue);
	g.strokePath(plot1, juce::PathStrokeType(3.0f));

	// Axis variables
	int numXMarkers = xDiff;
	int numYMarkers = yDiff;

	// Plot X Axis Markers
	for (int i = 1; i <= numXMarkers; i++) {
		xAxisMarkers.startNewSubPath(xStartXYAxis + (xAxisScale_plotAndMarker * i * scaleX), yStartXYAxis - 5);
		xAxisMarkers.lineTo(xStartXYAxis + (xAxisScale_plotAndMarker * i * scaleX), yStartXYAxis + 5);
	}
	g.setColour(juce::Colours::white);
	g.strokePath(xAxisMarkers, juce::PathStrokeType(2.0f));

	// Plot Y Axis Markers
	for (int i = 1; i <= numYMarkers; i++) {
		yAxisMarkersUp.startNewSubPath(xStartXYAxis - 5, yStartPlot + (scaleY * i) + yShift);
		yAxisMarkersUp.lineTo(xStartXYAxis + 5, yStartPlot + (scaleY * i) + yShift);  // drawing line markers moving up from midpoint
		yAxisMarkersDown.startNewSubPath(xStartXYAxis - 5, yStartPlot - (scaleY * i) + yShift);
		yAxisMarkersDown.lineTo(xStartXYAxis + 5, yStartPlot - (scaleY * i) + yShift);  // drawing line markers moving up from midpoint
	}
	g.setColour(juce::Colours::white);
	g.strokePath(yAxisMarkersUp, juce::PathStrokeType(2.0f));
	g.strokePath(yAxisMarkersDown, juce::PathStrokeType(2.0f));

	//Plot zero on Y-axis
	zeroTick.startNewSubPath(xStartXYAxis - 15, yStartPlot + yShift);
	zeroTick.lineTo(xStartXYAxis + 15, yStartPlot + yShift);
	g.strokePath(zeroTick, juce::PathStrokeType(3.0f));

	// Plot x-axis
	xAxis.startNewSubPath(xStartXYAxis, yStartXYAxis);
	xAxis.lineTo(xStartXYAxis + lengthXAxis, yStartXYAxis);
	g.setColour(juce::Colours::white);
	g.strokePath(xAxis, juce::PathStrokeType(2.0f));

	// Plot y-axis
	yAxis.startNewSubPath(xStartXYAxis, yStartXYAxis);
	yAxis.lineTo(xStartXYAxis, yStartXYAxis - lengthYAxis);
	g.setColour(juce::Colours::white);
	g.strokePath(yAxis, juce::PathStrokeType(2.0f));

	// NEW STUFF

	//** cursor variables **//
	graphWest = border_xBuffer;
	graphEast = widthBorder;
	graphNorth = border_yBuffer;
	graphSouth = heightBorder;

	//** draw graph border **//
	juce::Path graphBoundary;
	graphBoundary.startNewSubPath(border_xBuffer, border_yBuffer);
	graphBoundary.lineTo(widthBorder, border_yBuffer);
	graphBoundary.lineTo(widthBorder, heightBorder);
	graphBoundary.lineTo(border_xBuffer, heightBorder);
	graphBoundary.lineTo(border_xBuffer, border_yBuffer);
	g.setColour(juce::Colours::slategrey);
	g.strokePath(graphBoundary, juce::PathStrokeType(1.0f));

	//** draw boxes to hide out of bound plots **//
	int x_LeftBoxOffset = 0;
	int y_LeftBoxOffset = 0;
	int width_LeftBox = border_xBuffer;
	int height_LeftBox = getHeight();

	int x_TopBoxOffset = 0;
	int y_TopBoxOffset = 0;
	int width_TopBox = getWidth();
	int height_TopBox = border_yBuffer;

	int x_RightBoxOffset = widthBorder + 0.5;
	int y_RightBoxOffset = 0;
	int width_RightBox = getWidth();
	int height_RightBox = getHeight();

	int x_BottonBoxOffset = 0;
	int y_BottomBoxOffset = heightBorder;
	int width_BottomBox = getWidth();
	int height_BottomBox = getHeight();

	juce::Rectangle<int> leftPanel(x_LeftBoxOffset, y_LeftBoxOffset, width_LeftBox, height_LeftBox);
	juce::Rectangle<int> rightPanel(x_RightBoxOffset, y_RightBoxOffset, width_RightBox, height_RightBox);
	juce::Rectangle<int> topPanel(x_TopBoxOffset, y_TopBoxOffset, width_TopBox, height_TopBox);
	juce::Rectangle<int> bottomPanel(x_BottonBoxOffset, y_BottomBoxOffset, width_BottomBox, height_BottomBox);
	g.setColour(juce::Colours::black);
	g.fillRect(leftPanel);
	g.fillRect(rightPanel);
	g.fillRect(topPanel);
	g.fillRect(bottomPanel);

	//** line to seperate left-side components and right-side components **//
	g.setColour(juce::Colours::darkgrey);
	g.fillRect(width_primaryCategoryLabel, 0, 1, windowMaxHeight);

	//** white area of plot selection in IMPORT AUDIO**//
	int yMargin_selectionBox = height_primaryCategoryLabel + yOffsetPrimary_secondaryLabel + height_secondaryLabel + yOffset_selectionBox;
	int xMargin_checkboxFill = 16;
	int yMargin_checkboxFill1 = 74;
	int yMargin_checkboxFill2 = 120;

	// draw white box
	g.setColour(juce::Colours::white);
	g.fillRoundedRectangle(x_componentOffset, yMargin_selectionBox, width_selectionBox, height_selectionBox, 3);

	// fill in checkboxes
	if (isVisiblePlot1 == true) {
		g.setColour(juce::Colours::dodgerblue);
		g.fillRoundedRectangle(xMargin_checkboxFill, yMargin_checkboxFill1, 16, 16, 4);
	}
	if (isVisiblePlot2 == true) {
		g.setColour(juce::Colours::dodgerblue);
		g.fillRoundedRectangle(xMargin_checkboxFill, yMargin_checkboxFill2, 16, 16, 4);
	}

	// draw line to seperate plot selections
	int xMargin_selectionBoundary = 2.5 * x_componentOffset;
	int yMargin_selectionBoundary = height_primaryCategoryLabel + yOffsetPrimary_secondaryLabel + height_secondaryLabel + (23 * yOffset_selectionBox);
	g.setColour(juce::Colours::lightgrey);
	g.fillRect(xMargin_selectionBoundary, yMargin_selectionBoundary, 243, 1);

	//** line to seperate upper and lower x/y bounds in ZOOM **//
	int xMargin_zoomBoundary = 2.5 * x_componentOffset;
	int yMargin_zoomBoundary = (119.5 * yOffset_selectionBox);
	g.setColour(juce::Colours::darkgrey);
	g.fillRect(xMargin_zoomBoundary, yMargin_zoomBoundary, 245, 1);

	//** white box for cursor label **//
	int xMargin_cursorBox = xStartXYAxis + 138;
	int yMargin_cursorBox = heightBorder + 55;
	int width_cursorBox = 180;
	int height_cursorBox = 26;

	//** white box for peak label **//
	int xMargin_peakBox = xMargin_cursorBox + 205;
	int yMargin_peakBox = yMargin_cursorBox;
	int width_peakBox = 180;
	int height_peakBox = 26;

	// ** Peak ** //
	/*
	int cursorPeak = findPeak(cursorX);
	g.setColour(juce::Colours::red);
	//if cursor is in bounds & peak is in bounds
	if (isGraph && graphToScreen(cursorPeak) > xStartXYAxis && graphToScreen(cursorPeak) < 1000) {
		juce::Rectangle<int> peak(graphToScreen(findPeak(cursorX)), graphNorth, 1, graphSouth - graphNorth);
		g.fillRect(peak);
		g.setColour(juce::Colours::white);
		if (isVisiblePlot1 == true) {
			peakFunction.setText("(" + floatToStringPrecision(cursorPeak, 1) + ", " + floatToStringPrecision(cursorYPeak, 2) + ")", juce::dontSendNotification);
		}
		if (isVisiblePlot2 == true) {
			peakFunction.setText("(" + floatToStringPrecision(cursorPeak, 1) + ", " + floatToStringPrecision(cursorYPeak, 2) + ")", juce::dontSendNotification);
		}
	}
	else {
		g.setColour(juce::Colours::white);
		peakFunction.setText("(0.0, 0.00)", juce::dontSendNotification);
	}
	*/
}

void FFTSpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
	if (!isRunning && audioProcessor.getProcBlockCalled()) {
		isRunning = true;
		audioProcessor.resetProcBlockCalled();
	}
	else if (isRunning && !audioProcessor.getProcBlockCalled()) {
		isRunning = false;
		repaint();
		//audioProcessor.resetScopeDataIndex();
	}
}

void FFTSpectrumAnalyzerAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..

	//** graph scaling variables **//
	float border_xBuffer = getWidth() * 0.295;
	float border_yBuffer = y_componentOffset;
	float widthBorder = getWidth() - x_componentOffset;
	float heightBorder = getHeight() - 240;
	float xBuffer = border_xBuffer + 2;
	float yBuffer = border_yBuffer + 12;
	float lengthXAxis = widthBorder;
	float lengthYAxis = heightBorder * .95;
	float yStartXYAxis = yBuffer + lengthYAxis - 1;
	float xStartXYAxis = xBuffer - 3;
	float yStartPlot = (yBuffer + lengthYAxis) / 2;

	//** margins for primary labels **//
	int yMargin_selectTraceLabel = height_primaryCategoryLabel + yOffsetPrimary_secondaryLabel;
	int yMargin_zoomLabel = yMargin_selectTraceLabel + (22.5 * y_componentOffset);
	int yMargin_exportLabel = yMargin_selectTraceLabel + (42 * y_componentOffset);

	// secondary gui element width
	int width_toggleButton = 30;
	int width_plotLabel = 50;
	int width_selectButton = 90;
	int width_inputTextbox = 60;
	int width_exportButton = 95;
	int width_comboBox = 160;

	// secondary gui element height
	int heightControlWidget = 24;
	int height_toggleButton = heightControlWidget;
	int height_plotLabel = heightControlWidget;
	int height_selectButton = heightControlWidget;
	int height_inputTextbox = heightControlWidget - 2;
	int height_exportButton = heightControlWidget + 4;
	int height_cursorPeak = heightControlWidget - 4;
	int height_comboBox = heightControlWidget + 6;

	//** margins for combo

	//** cursor *//
	// label
	int xMargin_cursorLabel = xStartXYAxis + 133;
	int yMargin_cursorLabel = heightBorder + 30;
	// combobox
	int xMargin_cursorCombo = xMargin_cursorLabel + 4;
	int yMargin_cursorCombo = yMargin_cursorLabel + 22;


	//** peak **//
	// label
	int xMargin_peakLabel = xMargin_cursorLabel + 205;
	int yMargin_peaklabel = yMargin_cursorLabel;
	// combobox
	int xMargin_peakCombo = xMargin_peakLabel + 4;
	int yMargin_peakCombo = yMargin_cursorCombo;


	//** window function **//
	//label
	int xMargin_windowLabel = xStartXYAxis + 65;
	int yMargin_windowLabel = yMargin_peaklabel + 62;
	// combobox
	int xMargin_winCombo = xMargin_windowLabel + 4;
	int yMargin_winCombo = yMargin_windowLabel + 22;

	//** axis **//
	// label
	int xMargin_axisLabel = xMargin_winCombo + 180;
	int yMargin_axisLabel = yMargin_windowLabel;
	// combobox
	int xMargin_axisCombo = xMargin_axisLabel + 4;
	int yMargin_axisCombo = yMargin_winCombo;


	//** size **//
	// label
	int xMargin_sizeLabel = xMargin_axisLabel + 180;
	int yMargin_sizeLabel = yMargin_axisLabel;
	// combobox
	int xMargin_sizeCombo = xMargin_sizeLabel + 4;
	int yMargin_sizeCombo = yMargin_winCombo;

	//** plot 1 **//
	// toggle button 1
	int xMargin_toggleButton1 = 2 * x_componentOffset;
	int yMargin_toggleButton1 = height_primaryCategoryLabel + yOffsetPrimary_secondaryLabel + height_secondaryLabel + (6 * yOffset_selectionBox);
	// plot label 1
	int xMargin_plotLabel1 = 4 * xMargin_toggleButton1;
	int yMargin_plotLabel1 = yMargin_toggleButton1;
	// selection button 1
	int xMargin_selectButton1 = 3.5 * xMargin_plotLabel1;
	int yMargin_selectButton1 = yMargin_toggleButton1;

	//** plot 2 **//
	// toggle button 2
	int xMargin_toggleButton2 = xMargin_toggleButton1;
	int yMargin_toggleBotton2 = yMargin_toggleButton1 + (23 * yOffset_selectionBox);
	// plot label 2
	int xMargin_plotLabel2 = xMargin_plotLabel1;
	int yMargin_plotLabel2 = yMargin_toggleBotton2;
	// selection button 2
	int xMargin_selectButton2 = xMargin_selectButton1;
	int yMargin_selectButton2 = yMargin_toggleBotton2;

	//** upper bounds **//
	// upper label
	int yMargin_upperLabel = yMargin_zoomLabel + height_primaryCategoryLabel + yOffsetPrimary_secondaryLabel;

	// xMax input
	int xMargin_xMax = 10 * x_componentOffset;
	int yMargin_xMax = yMargin_upperLabel + 2;

	// x label
	int xMargin_xMaxLabel = 20.5 * x_componentOffset;
	int yMargin_xMaxLabel = yMargin_upperLabel;

	// yMax input
	int xMargin_yMax = 30 * x_componentOffset;
	int yMargin_yMax = yMargin_upperLabel + 2;

	// y label
	int xMargin_yMaxLabel = 40.5 * x_componentOffset;;
	int yMargin_yMaxLabel = yMargin_upperLabel;

	//** lower bounds **//
	// lower label
	int yMargin_lowerLabel = yMargin_upperLabel + (8 * y_componentOffset);

	// xMin input
	int xMargin_xMin = xMargin_xMax;
	int yMargin_xMin = yMargin_lowerLabel + 2;

	// x label
	int xMargin_xMinLabel = xMargin_xMaxLabel;
	int yMargin_xMinLabel = yMargin_lowerLabel;

	// yMin input
	int xMargin_yMin = xMargin_yMax;
	int yMargin_yMin = yMargin_lowerLabel + 2;

	// y label
	int xMargin_yMinLabel = xMargin_yMaxLabel;
	int yMargin_yMinLabel = yMargin_lowerLabel;

	//** export button **//
	int xMargin_exportButton = x_componentOffset;
	int yMargin_exportButton = yMargin_exportLabel + height_primaryCategoryLabel + (1.5 * yOffsetPrimary_secondaryLabel);

	//** Set bounds for right side elements **//
	cursorLabel.setBounds(xMargin_cursorLabel, yMargin_cursorLabel, width_secondaryLabel, height_cursorPeak);
	peakLabel.setBounds(xMargin_peakLabel, yMargin_peaklabel, width_secondaryLabel, height_cursorPeak);
	windowLabel.setBounds(xMargin_windowLabel, yMargin_windowLabel, width_secondaryLabel, height_secondaryLabel);
	axisLabel.setBounds(xMargin_axisLabel, yMargin_axisLabel, width_secondaryLabel, height_secondaryLabel);
	sizeLabel.setBounds(xMargin_sizeLabel, yMargin_sizeLabel, width_secondaryLabel, height_secondaryLabel);

	cursorFunction.setBounds(xMargin_cursorCombo, yMargin_cursorCombo, width_comboBox, height_comboBox);
	peakFunction.setBounds(xMargin_peakCombo, yMargin_peakCombo, width_comboBox, height_comboBox);
	windowFunction.setBounds(xMargin_winCombo, yMargin_winCombo, width_comboBox, height_comboBox);
	axis.setBounds(xMargin_axisCombo, yMargin_axisCombo, width_comboBox, height_comboBox);
	size.setBounds(xMargin_sizeCombo, yMargin_sizeCombo, width_comboBox, height_comboBox);

	//** set bounds for GUI elements **//
	gui_importAudio.setBounds(0, 0, width_primaryCategoryLabel, height_primaryCategoryLabel);
	gui_selectTrace.setBounds(0, yMargin_selectTraceLabel, width_secondaryLabel, height_secondaryLabel);
	gui_zoom.setBounds(0, yMargin_zoomLabel, width_primaryCategoryLabel, height_primaryCategoryLabel);
	gui_export.setBounds(0, yMargin_exportLabel, width_primaryCategoryLabel, height_primaryCategoryLabel);

	//** set bounds for secondary GUI elements **//
	toggleButtonPlot1.setBounds(xMargin_toggleButton1, yMargin_toggleButton1, width_toggleButton, height_toggleButton);
	toggleButtonPlot2.setBounds(xMargin_toggleButton2, yMargin_toggleBotton2, width_toggleButton, height_toggleButton);

	labelPlot1.setBounds(xMargin_plotLabel1, yMargin_plotLabel1, width_plotLabel, height_plotLabel);
	labelPlot2.setBounds(xMargin_plotLabel2, yMargin_plotLabel2, width_plotLabel, height_plotLabel);

	buttonPlot1.setBounds(xMargin_selectButton1, yMargin_selectButton1, width_selectButton, height_selectButton);
	buttonPlot2.setBounds(xMargin_selectButton2, yMargin_selectButton2, width_selectButton, height_selectButton);

	gui_upper.setBounds(0, yMargin_upperLabel, width_secondaryLabel, height_secondaryLabel);
	inputXmax.setBounds(xMargin_xMax, yMargin_xMax, width_inputTextbox, height_inputTextbox);
	gui_xUpper.setBounds(xMargin_xMaxLabel, yMargin_xMaxLabel, width_secondaryLabel, height_secondaryLabel);
	inputYmax.setBounds(xMargin_yMax, yMargin_yMax, width_inputTextbox, height_inputTextbox);
	gui_yUpper.setBounds(xMargin_yMaxLabel, yMargin_yMaxLabel, width_inputTextbox, height_inputTextbox);

	gui_lower.setBounds(0, yMargin_lowerLabel, width_secondaryLabel, height_secondaryLabel);
	inputXmin.setBounds(xMargin_xMin, yMargin_xMin, width_inputTextbox, height_inputTextbox);
	gui_xLower.setBounds(xMargin_xMinLabel, yMargin_xMinLabel, width_secondaryLabel, height_secondaryLabel);
	inputYmin.setBounds(xMargin_yMax, yMargin_yMin, width_inputTextbox, height_inputTextbox);
	gui_yLower.setBounds(xMargin_yMinLabel, yMargin_yMinLabel, width_secondaryLabel, height_secondaryLabel);

	gui_exportButton.setBounds(xMargin_exportButton, yMargin_exportButton, width_exportButton, height_exportButton);
}

void FFTSpectrumAnalyzerAudioProcessorEditor::setFreqData(int fftData, int sampleRate) {
	fftS = fftData;
	numBins = fftS / 2 + 1;
	maxFreq = sampleRate / 2;
	numFreqBins = fftS / 2;
	indexToFreqMap.resize(numBins);
	binMag.resize(1, std::vector<float>(numBins, 0));
}

void FFTSpectrumAnalyzerAudioProcessorEditor::getBounds()
{
	int minVal = -1000;
	int maxVal = 1000;
	juce::String temp = inputXmin.getText(false);
	int val = std::atoi(temp.toStdString().c_str());
	if (val >= minVal && val <= maxVal)
	{
		xMin = val;
	}
	else { inputXmin.setText(std::to_string(xMin), juce::dontSendNotification); }

	temp = inputXmax.getText(false);
	val = std::atoi(temp.toStdString().c_str());
	if (val >= minVal && val <= maxVal)
	{
		xMax = val;
	}
	else { inputXmax.setText(std::to_string(xMax), juce::dontSendNotification); }

	temp = inputYmin.getText(false);
	val = std::atoi(temp.toStdString().c_str());
	if (val >= minVal && val <= maxVal)
	{
		yMin = val;
	}
	else { inputYmin.setText(std::to_string(yMin), juce::dontSendNotification); }

	temp = inputYmax.getText(false);
	val = std::atoi(temp.toStdString().c_str());
	if (val >= minVal && val <= maxVal)
	{
		yMax = val;
	}
	else { inputYmax.setText(std::to_string(yMax), juce::dontSendNotification); }
	repaint();
}


void FFTSpectrumAnalyzerAudioProcessorEditor::setPlotIndex(int plotIndex)
{
	plotIndexSelection = plotIndex;
	//audioProcessor.setPlotIndex(plotIndexSelection);
	if (plotIndex == 0)
	{
		buttonPlot1.setButtonText("Selected");
		buttonPlot2.setButtonText("Select");
	}
	else if (plotIndex == 1)
	{
		buttonPlot2.setButtonText("Selected");
		buttonPlot1.setButtonText("Select");
	}
}

void FFTSpectrumAnalyzerAudioProcessorEditor::updateToggleState(int plotId)
{
	if (plotId == 1)
	{
		setVisibility(1);
	}
	else
		setVisibility(2);
	repaint();
}


void FFTSpectrumAnalyzerAudioProcessorEditor::setVisibility(int plotId)
{
	if (plotId == 1)
	{
		if (isVisiblePlot1 == false) {
			isVisiblePlot1 = true;
		}
		else if (isVisiblePlot1 == true) {
			isVisiblePlot1 = false;
		}
	}
	else
	{
		if (isVisiblePlot2 == false) {
			isVisiblePlot2 = true;
		}
		else if (isVisiblePlot2 == true) {
			isVisiblePlot2 = false;
		}
	}
}

void FFTSpectrumAnalyzerAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
	cursorX = event.getMouseDownX();
	int cursorY = event.getMouseDownY();

	// invalid bounds
	if (cursorX < graphWest || cursorX > graphEast || cursorY < graphNorth || cursorY > graphSouth) {
		isGraph = false;
		cursorFunction.setText("(0.0, 0.00)", juce::dontSendNotification);
	}
	else {
		isGraph = true;
		cursorX = screenToGraph(cursorX);

		//plot 1
		if (rowIndex == 0 && isVisiblePlot1) {
			const std::vector<std::vector<float>> scopeData = audioProcessor.getBinMag();
			cursorPlot1.setText("(" + floatToStringPrecision(cursorX, 1) + ", " + floatToStringPrecision(scopeData[rowIndex][cursorX], 2) + ")", juce::dontSendNotification);
		}
		//plot 2
		else {
			const std::vector<std::vector<float>> scopeData2 = audioProcessor.getBinMag();
			cursorPlot2.setText("(" + floatToStringPrecision(cursorX, 1) + ", " + floatToStringPrecision(scopeData2[rowIndex][cursorX], 2) + ")", juce::dontSendNotification);
		}
	}
	repaint();
}


int FFTSpectrumAnalyzerAudioProcessorEditor::screenToGraph(int screenCoord) {
	int graphCoord;
	int xScale = (graphEast - graphWest) / (xMax - xMin);

	screenCoord += (xMin * xScale);
	screenCoord -= graphWest;
	screenCoord /= xScale;
	graphCoord = screenCoord;
	return graphCoord;
}
int FFTSpectrumAnalyzerAudioProcessorEditor::graphToScreen(int graphCoord) {
	int screenCoord;
	int xScale = (graphEast - graphWest) / (xMax - xMin);

	graphCoord *= xScale;
	graphCoord += graphWest;
	graphCoord -= (xMin * xScale);
	screenCoord = graphCoord;
	return screenCoord;
}

void FFTSpectrumAnalyzerAudioProcessorEditor::setWindowFunction() {
	juce::dsp::WindowingFunction<float>::WindowingMethod newWindow;
	switch (windowFunction.getSelectedId()) {
	case 1:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::blackman;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 2:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::blackmanHarris;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 3:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::flatTop;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 4:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::hamming;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 5:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::hann;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 6:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::kaiser;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 7:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::rectangular;
		audioProcessor.setWindow(newWindow);
		repaint();
	case 8:
		newWindow = juce::dsp::WindowingFunction<float>::WindowingMethod::triangular;
		audioProcessor.setWindow(newWindow);
		repaint();
	}
}
void FFTSpectrumAnalyzerAudioProcessorEditor::setSampleSize() {
	auto selection = size.getText();
	fftS = selection.getIntValue();
	repaint();
}
void FFTSpectrumAnalyzerAudioProcessorEditor::setAxisScale() {
	// to fill in later
}

std::string FFTSpectrumAnalyzerAudioProcessorEditor::floatToStringPrecision(float f, int p)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(p) << f;
	return oss.str();
}