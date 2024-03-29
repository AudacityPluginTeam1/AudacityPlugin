/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

int FFTSpectrumAnalyzerAudioProcessorEditor::cursorX1 = 0; //mouse
float FFTSpectrumAnalyzerAudioProcessorEditor::cursorY1 = 0.00; //mouse
int FFTSpectrumAnalyzerAudioProcessorEditor::cursorX2 = 0; //mouse
float FFTSpectrumAnalyzerAudioProcessorEditor::cursorY2 = 0.00; //mouse
bool FFTSpectrumAnalyzerAudioProcessorEditor::isRunning = false;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMinPrev = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMin = 0;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMaxPrev = 100;
int FFTSpectrumAnalyzerAudioProcessorEditor::xMax = 100;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMinPrev = -10;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMin = -10;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMaxPrev = 10;
int FFTSpectrumAnalyzerAudioProcessorEditor::yMax = 10;
int FFTSpectrumAnalyzerAudioProcessorEditor::plotIndexSelection = 0;
bool FFTSpectrumAnalyzerAudioProcessorEditor::isVisiblePlot1 = true;
bool FFTSpectrumAnalyzerAudioProcessorEditor::isVisiblePlot2 = true;

int FFTSpectrumAnalyzerAudioProcessorEditor::windowWidth = 1200;
int FFTSpectrumAnalyzerAudioProcessorEditor::windowHeight = 650 + 2;
int FFTSpectrumAnalyzerAudioProcessorEditor::xBuffer = 50;
int FFTSpectrumAnalyzerAudioProcessorEditor::yBuffer = 30;
int FFTSpectrumAnalyzerAudioProcessorEditor::lengthXAxis = 900;  //pixels = unit
int FFTSpectrumAnalyzerAudioProcessorEditor::lengthYAxis = 900;  //pixels = unit
int FFTSpectrumAnalyzerAudioProcessorEditor::yStartXYAxis = yBuffer + lengthYAxis;
int FFTSpectrumAnalyzerAudioProcessorEditor::xStartXYAxis = 100;
int FFTSpectrumAnalyzerAudioProcessorEditor::yStartPlot = yBuffer + lengthYAxis / 2;

//==============================================================================
FFTSpectrumAnalyzerAudioProcessorEditor::FFTSpectrumAnalyzerAudioProcessorEditor(FFTSpectrumAnalyzerAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setOpaque(true);
	startTimer(500);
	setSize(windowWidth, windowHeight);

	setPlotIndex(plotIndexSelection);

	// new gui elements start
	addAndMakeVisible(gui_importAudio);
	gui_importAudio.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
	gui_importAudio.setText("Import Audio", juce::dontSendNotification);
	gui_importAudio.setColour(juce::Label::backgroundColourId, juce::Colours::green);

	addAndMakeVisible(gui_selectTrace);
	gui_selectTrace.setText("Selected Traces", juce::dontSendNotification);
	gui_selectTrace.setFont(juce::Font(16.0f));
	
	addAndMakeVisible(gui_zoom);
	gui_zoom.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
	gui_zoom.setText("Zoom", juce::dontSendNotification);
	gui_zoom.setColour(juce::Label::backgroundColourId, juce::Colours::green);

	addAndMakeVisible(gui_upper);
	gui_upper.setText("Upper", juce::dontSendNotification);
	gui_upper.setFont(juce::Font(16.0f));

	addAndMakeVisible(gui_lower);
	gui_lower.setText("Lower", juce::dontSendNotification);
	gui_lower.setFont(juce::Font(16.0f));

	addAndMakeVisible(gui_x);
	gui_x.setText("X", juce::dontSendNotification);
	gui_x.setFont(juce::Font(16.0f));

	addAndMakeVisible(gui_y);
	gui_y.setText("Y", juce::dontSendNotification);
	gui_y.setFont(juce::Font(16.0f));

	// new gui elements end

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

	// toggle button for plot 1
	addAndMakeVisible(toggleButtonPlot1);
	toggleButtonPlot1.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
	toggleButtonPlot1.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::lightgrey);
	if (isVisiblePlot1 == true)
	{
		toggleButtonPlot1.setToggleState(true, true);
	}
	toggleButtonPlot1.onClick = [this] { updateToggleState(1); };
	toggleButtonPlot1.setClickingTogglesState(true);

	// toggle button for plot 2
	addAndMakeVisible(toggleButtonPlot2);
	toggleButtonPlot2.setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
	toggleButtonPlot2.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::lightgrey);
	if (isVisiblePlot2 == true)
	{
		toggleButtonPlot2.setToggleState(true, true);
	}
	toggleButtonPlot2.onClick = [this] { updateToggleState(2); };
	toggleButtonPlot2.setClickingTogglesState(true);

	addAndMakeVisible(cursorPlot1); //mouse
	addAndMakeVisible(cursorPlot2); //mouse
	addAndMakeVisible(cursorLabel0); //mouse
	addAndMakeVisible(cursorLabel1); //mouse
	addAndMakeVisible(cursorLabel2); //mouse
	cursorPlot1.setEditable(false); //mouse
	cursorPlot2.setEditable(false); //mouse
	cursorLabel0.setEditable(false); //mouse
	cursorLabel1.setEditable(false); //mouse
	cursorLabel2.setEditable(false); //mouse
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
	cursorPlot1.setText("(" + floatToStringPrecision(cursorX1, 1) + ", " + floatToStringPrecision(cursorY1, 2) + ")", juce::dontSendNotification);
	cursorPlot2.setText("(" + floatToStringPrecision(cursorX2, 1) + ", " + floatToStringPrecision(cursorY2, 2) + ")", juce::dontSendNotification);
	cursorLabel0.setText("Cursor:", juce::dontSendNotification);
	cursorLabel1.setText("Plot 1", juce::dontSendNotification);
	cursorLabel2.setText("Plot 2", juce::dontSendNotification);
	//inputXmax.setJustificationType(juce::Justification::centred);
	//inputYmax.setJustificationType(juce::Justification::centred);
	inputXmin.setText(std::to_string(xMin), juce::dontSendNotification);
	inputXmax.setText(std::to_string(xMax), juce::dontSendNotification);
	inputYmin.setText(std::to_string(yMin), juce::dontSendNotification);
	inputYmax.setText(std::to_string(yMax), juce::dontSendNotification);
	labelPlot1.setText("Plot 1", juce::dontSendNotification);
	labelPlot1.setColour(juce::Label::textColourId, juce::Colours::darkgrey);
	labelPlot2.setText("Plot 2", juce::dontSendNotification);
	labelPlot2.setColour(juce::Label::textColourId, juce::Colours::darkgrey);
	inputXmin.setColour(juce::Label::backgroundColourId, juce::Colours::black);
	inputXmax.setColour(juce::Label::backgroundColourId, juce::Colours::black);
	inputYmin.setColour(juce::Label::backgroundColourId, juce::Colours::black);
	inputYmax.setColour(juce::Label::backgroundColourId, juce::Colours::black);
	inputXmin.onTextChange = [this] { getBounds(); };
	inputXmax.onTextChange = [this] { getBounds(); };
	inputYmin.onTextChange = [this] { getBounds(); };
	inputYmax.onTextChange = [this] { getBounds(); };
}

FFTSpectrumAnalyzerAudioProcessorEditor::~FFTSpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void FFTSpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.setOpacity(1.0f);
	g.setColour(juce::Colours::white);

	// gui elements start
	const int gui_secondaryComponentMarginX = 6;
	const int gui_secondaryComponentMarginY = 60;
	// plot selection width has a difference of 12 vs category label width
	const int widthPlotSelectionBox = 265;
	const int heightPlotSelectionBox = 88;
	const int labelMarginX = widthPlotSelectionBox + 12;

	// gui elements end

	const int scopeSize = audioProcessor.getScopeSize();
	const int plotSize = audioProcessor.getPlotSize();
	const int plotIndex = audioProcessor.getPlotIndex();
	const float* scopeData = audioProcessor.getScopeData();
	const float* fft = audioProcessor.getFFT();

	juce::Path plot1;
	juce::Path plot2;
	juce::Path xAxis;
	juce::Path xAxisMarkers;
	juce::Path yAxis;
	juce::Path yAxisMarkersUp;
	juce::Path yAxisMarkersDown;
	juce::Path zeroTick;

	int sampleSize = 100;  // Adjust the number of samples being displayed as needed

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
	/*
	plot2.startNewSubPath(xStartXYAxis + xShift, yStartPlot + *(scopeData + 1 * scopeSize) * scaleY + yShift);
	plot1.startNewSubPath(xStartXYAxis + xShift, yStartPlot + scopeData[0] * scaleY + yShift);  // Xmin needs to be the new startXPlot; this will be reset by the bounds read in to xMin textEntry box
	for (int i = 1; i <= sampleSize; i++)
	{
		if (isVisiblePlot2 == true) {
			plot2.lineTo(i * scaleX + xStartXYAxis + xShift, *((scopeData + i) + 1 * scopeSize) * scaleY + plotYShift);
		}
		if (isVisiblePlot1 == true) {
			plot1.lineTo(i * scaleX + xStartXYAxis + xShift, *((scopeData + i) + 0 * scopeSize) * scaleY + plotYShift);
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
		xAxisMarkers.startNewSubPath(xStartXYAxis + (i * scaleX), yStartXYAxis - 5);
		xAxisMarkers.lineTo(xStartXYAxis + (i * scaleX), yStartXYAxis + 5);
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
	*/
	
	// Draw background boxes
	//box 1
	g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.fillRect(0, 0, 100, 950);
	//box 2
	g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.fillRect(xStartXYAxis + lengthXAxis, 0, 600, 950);
	//box 3
	g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.fillRect(0, 0, 1200, 30);
	//box 4
	g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.fillRect(0, yBuffer + lengthYAxis, 1200, 400);
	
	// white area for selected traces
	g.setColour(juce::Colours::white);
	g.fillRoundedRectangle(gui_secondaryComponentMarginX, gui_secondaryComponentMarginY, widthPlotSelectionBox, heightPlotSelectionBox, 3);

	// line to seperate plot selection
	g.setColour(juce::Colours::lightgrey);
	g.fillRect(15, 104, 247, 1);

	// fill in check boxes
	if (isVisiblePlot1 == true) {
		g.setColour(juce::Colours::dodgerblue);
		g.fillRoundedRectangle(16, 74, 16, 16, 4);
	}

	if (isVisiblePlot2 == true) {
		g.setColour(juce::Colours::dodgerblue);
		g.fillRoundedRectangle(16, 118, 16, 16, 4);
	}

	// line to seperate left side from graphing side
	g.setColour(juce::Colours::white);
	g.fillRect(labelMarginX, 0, 1, windowHeight);

	/*
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
		audioProcessor.resetScopeDataIndex();
	}
}

void FFTSpectrumAnalyzerAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..

	// new gui elemetns start
	int widthLabel1Category = 277;
	int heightLabel1Category = 25;
	int widthSecondaryLabel = 175;
	int heightSecondaryLabel = 20;
	int categoryLabelSpace = 10;

	gui_importAudio.setBounds(0, 0, widthLabel1Category, heightLabel1Category);
	gui_selectTrace.setBounds(0, heightLabel1Category + categoryLabelSpace, widthSecondaryLabel, heightSecondaryLabel);

	gui_zoom.setBounds(0, 165, widthLabel1Category, heightLabel1Category);

	// new gui elements end

	int widgetOffsetVertical = 10;
	//int widgetOffsetHorizontal = 10;
	int widgetOffsetHorizontal = 5;

	int widthLabel = 50;
	int widthPlotLabel = 50;
	int widthToggleButton = 30;
	int widthButton = 90;

	int leftMarginXmin = xStartXYAxis;
	int leftMarginXmax = lengthXAxis + widthLabel;
	int leftMarginYmin = xBuffer;
	int leftMarginYmax = xBuffer;
	int topMarginXMinMax = lengthYAxis + yBuffer * 2;
	int topMarginYmin = lengthYAxis + yBuffer;
	int topMarginYmax = yBuffer;

	int heightControlWidget = 24;
	int heightPlotLabel = heightControlWidget;
	int heightToggleButton = heightControlWidget;
	int heightButton = heightControlWidget;

	int leftMarginToggleButton = xBuffer * 4 + lengthXAxis;
	int leftMarginPlotLabel = leftMarginToggleButton + widthToggleButton + widgetOffsetHorizontal;
	int leftMarginButton = leftMarginPlotLabel + widthPlotLabel + widgetOffsetHorizontal;

	int topMarginControlWidget = yBuffer * 2;
	int topMarginToggleButton1 = topMarginControlWidget;
	int topMarginToggleButton2 = topMarginControlWidget + heightToggleButton + widgetOffsetVertical;
	int topMarginPlot1Label = topMarginControlWidget;
	int topMarginPlot2Label = topMarginControlWidget + heightPlotLabel + widgetOffsetVertical;
	int topMarginButton1 = topMarginControlWidget;
	int topMarginButton2 = topMarginControlWidget + heightButton + widgetOffsetVertical;

	// new margins for buttons and text
	int labelToButtonOffet = 80;
	int lineSeparationOffset = 10;

	int toggleButtonMarginX = 12;
	int toggleButton1MarginY = 70;
	int toggleButton2MarginY = toggleButton1MarginY + heightButton + widgetOffsetVertical + lineSeparationOffset; 
	 
	int plotLabel1MarginX = toggleButtonMarginX + widthToggleButton; 
	int plotLabel1MarginY = toggleButton1MarginY;

	int plotLabel2MarginX = toggleButtonMarginX + widthToggleButton;
	int plotLabel2MarginY = toggleButton2MarginY;
	
	int buttonSelect1MarginX = plotLabel1MarginX + widthLabel + labelToButtonOffet;
	int buttonSelect1MarginY = plotLabel1MarginY;

	int buttonSelect2MarginX = plotLabel2MarginX + widthLabel + labelToButtonOffet;
	int buttonSelect2MarginY = plotLabel2MarginY; 

	int xy_inputlabel = 60;

	//inputXmin.setBounds(leftMarginXmin, topMarginXMinMax, widthLabel, heightControlWidget);
	//inputXmax.setBounds(leftMarginXmax, topMarginXMinMax, widthLabel, heightControlWidget);
	//inputXmin.setBounds(leftMarginXmin, topMarginXMinMax, widthLabel, heightControlWidget);
	//inputXmax.setBounds(leftMarginXmax, topMarginXMinMax, widthLabel, heightControlWidget);

	cursorPlot1.setBounds(500, topMarginXMinMax - 20, 70, 24); //mouse
	cursorPlot2.setBounds(500, topMarginXMinMax, 70, 24); //mouse
	cursorLabel0.setBounds(450, topMarginXMinMax - 20, 60, 24); //mouse
	cursorLabel1.setBounds(570, topMarginXMinMax - 20, 60, 24); //mouse
	cursorLabel2.setBounds(570, topMarginXMinMax, 60, 24); //mouse
	//inputYmin.setBounds(leftMarginYmin, topMarginYmin, widthLabel, heightControlWidget);
	//inputYmax.setBounds(leftMarginYmax, topMarginYmax, widthLabel, heightControlWidget); 
	
	//labelPlot1.setBounds(leftMarginPlotLabel, topMarginPlot1Label, widthPlotLabel, heightPlotLabel);
	//labelPlot2.setBounds(leftMarginPlotLabel, topMarginPlot2Label, widthPlotLabel, heightPlotLabel);
	labelPlot1.setBounds(plotLabel1MarginX, plotLabel1MarginY, widthPlotLabel, heightPlotLabel);
	labelPlot2.setBounds(plotLabel2MarginX, plotLabel2MarginY, widthPlotLabel, heightPlotLabel); 

	//toggleButtonPlot1.setBounds(leftMarginToggleButton, topMarginToggleButton1, widthToggleButton, heightToggleButton);
	//toggleButtonPlot2.setBounds(leftMarginToggleButton, topMarginToggleButton2, widthToggleButton, heightToggleButton);
	toggleButtonPlot1.setBounds(toggleButtonMarginX, toggleButton1MarginY, widthToggleButton, heightToggleButton);
	toggleButtonPlot2.setBounds(toggleButtonMarginX, toggleButton2MarginY, widthToggleButton, heightToggleButton);

	//buttonPlot1.setBounds(leftMarginButton, topMarginButton1, widthButton, heightButton);
	//buttonPlot2.setBounds(leftMarginButton, topMarginButton2, widthButton, heightButton);
	buttonPlot1.setBounds(buttonSelect1MarginX, buttonSelect1MarginY, widthButton, heightButton); 
	buttonPlot2.setBounds(buttonSelect2MarginX, buttonSelect2MarginY, widthButton, heightButton); 

	// new margins for input x/y max label
	int upperMarginY = toggleButton2MarginY + heightLabel1Category + (6 * categoryLabelSpace);

	// label upper
	gui_upper.setBounds(0, upperMarginY, widthSecondaryLabel, heightSecondaryLabel);

	//x max input
	inputXmax.setBounds((10 * widgetOffsetHorizontal) + widgetOffsetHorizontal, upperMarginY, xy_inputlabel, heightControlWidget);

	// label x
	gui_x.setBounds(115, upperMarginY, widthSecondaryLabel, heightSecondaryLabel);

	// y max input
	inputYmax.setBounds((38 * widgetOffsetHorizontal) + widgetOffsetHorizontal, upperMarginY, xy_inputlabel, heightControlWidget);

	// label y
	gui_y.setBounds(255, upperMarginY, widthSecondaryLabel, heightSecondaryLabel);


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
	audioProcessor.setPlotIndex(plotIndexSelection);
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
	cursorX1 = event.getMouseDownX();
	cursorY1 = event.getMouseDownY();
	//invalid bounds
	if (cursorX1 < 100 || cursorX1 > 1000 || cursorY1 < 30 || cursorY1 > 930) {
		cursorX1 = cursorX2 = 0.0;
		cursorY1 = cursorY2 = 0.00;
		cursorPlot1.setText("(" + floatToStringPrecision(cursorX1, 1) + ", " + floatToStringPrecision(cursorY1, 2) + ")", juce::dontSendNotification); //mouse
		cursorPlot2.setText("(" + floatToStringPrecision(cursorX2, 1) + ", " + floatToStringPrecision(cursorY2, 2) + ")", juce::dontSendNotification); //mouse
	}
	else {
		//offset xCoord [xCoord / (ratio of x-axis length to bounds)]
		int xScale = lengthXAxis / (xMax - xMin);

		cursorX1 += (xMin * xScale);
		cursorX1 -= xStartXYAxis;
		cursorX1 /= xScale;
		cursorX2 = cursorX1;

		//plot 1
		if (audioProcessor.getPlotIndex() == 0 && isVisiblePlot1) {
			const float* scopeData = audioProcessor.getScopeData();
			cursorPlot1.setText("(" + floatToStringPrecision(cursorX1, 1) + ", " + floatToStringPrecision(scopeData[cursorX1], 2) + ")", juce::dontSendNotification);
		}
		//plot 2
		else if (audioProcessor.getPlotIndex() == 1 && isVisiblePlot2) {
			const float* scopeData2 = audioProcessor.getScopeData() + audioProcessor.getScopeSize();
			cursorPlot2.setText("(" + floatToStringPrecision(cursorX2, 1) + ", " + floatToStringPrecision(scopeData2[cursorX2], 2) + ")", juce::dontSendNotification);
		}
	}
	repaint();
}

std::string FFTSpectrumAnalyzerAudioProcessorEditor::floatToStringPrecision(float f, int p)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(p) << f;
	return oss.str();
}