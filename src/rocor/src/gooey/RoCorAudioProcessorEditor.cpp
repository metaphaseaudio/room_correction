/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <rocor/guts/RoCorAudioProcessor.h>
#include <rocor/gooey/RoCorAudioProcessorEditor.h>

using namespace juce;

//==============================================================================
RoCorAudioProcessorEditor::RoCorAudioProcessorEditor(RoCorAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , m_NewTest("New IR")
    , m_StartTest("Start Test")
    , m_StartCalc("Run IR Calc")
    , m_Save("Save IR")
    , processor(p)
    , m_ImpulseView(1, 1)
    , m_LabelChanCount("Chan Count Label", "Channel Arrangement")
    , m_ChanCount("Chan Count")
    , m_CapturesModel(processor.getCaptures())
    , m_ImpulsesModel(processor.getIRCalc().getCalculatedIRMap())
    , m_Captures("Captures", &m_CapturesModel)
    , m_Impulses("Impulses", &m_ImpulsesModel)
    , m_Progress(processor.getIRCalc().getProgress())
    , m_SemiModalMask(juce::Colours::black.withAlpha(0.5f))
    , m_Tabs(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    m_FormatMgr.registerBasicFormats();
    setSize(400, 650);
    processor.addChangeListener(this);
    processor.getIRCalc().addChangeListener(this);

    addAndMakeVisible(&processor.m_InputView);

    addAndMakeVisible(m_StartTest); m_StartTest.addListener(this);
    addAndMakeVisible(m_StartCalc); m_StartCalc.addListener(this);
    addAndMakeVisible(m_Save);      m_Save.addListener(this);
    addAndMakeVisible(m_SaveAll);   m_SaveAll.addListener(this);
    addAndMakeVisible(m_Load);      m_Load.addListener(this);
    addAndMakeVisible(m_LoadAll);   m_LoadAll.addListener(this);

    m_Tabs.addTab("Map", juce::Colours::black, &m_PlacementView, false);
    m_Tabs.addTab("Captures", juce::Colours::black, &m_Captures, false);
    m_Captures.setRowHeight(60);
    m_Impulses.setRowHeight(60);
    m_Tabs.addTab("IRs", juce::Colours::black, &m_Impulses, false);
    m_PlacementView.addChangeListener(this);
    addAndMakeVisible(m_Tabs);

    addAndMakeVisible(m_ImpulseView);
	m_ImpulseView.setClip(processor.getImpulse(), processor.getImpulse().getNumSamples());

	addAndMakeVisible(m_LabelChanCount);
    addAndMakeVisible(m_ChanCount);

	m_ChanCount.addItem("Mono", ChanCounts::MONO);
    m_ChanCount.addItem("Stereo", ChanCounts::STEREO);
    m_ChanCount.addItem("Quadraphonic", ChanCounts::QUAD);
    m_ChanCount.addItem("5/5.1", ChanCounts::FIVE);
    m_ChanCount.addItem("Octophonic", ChanCounts::OCTO);
    m_ChanCount.addListener(this);
    m_ChanCount.setSelectedId(processor.getImpulse().getNumChannels(), juce::NotificationType::dontSendNotification);

    addChildComponent(m_SemiModalMask);
    addChildComponent(m_Progress);
}

RoCorAudioProcessorEditor::~RoCorAudioProcessorEditor() { processor.removeChangeListener(this); }

//==============================================================================
void RoCorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void RoCorAudioProcessorEditor::resized()
{
    const auto bezel = getLocalBounds().reduced(5);
    auto control_space = bezel.reduced(5);
    auto top_controls = control_space.removeFromTop(44);
    auto bottom_buttons = control_space.removeFromBottom(50);

    m_SemiModalMask.setBounds(bezel);
    m_Progress.setBounds(juce::Rectangle<int>(bezel.getWidth() - 10, std::min(bezel.getHeight(), 20)));
    m_Progress.setCentrePosition(bezel.getCentre());

    m_LabelChanCount.setBounds(top_controls.removeFromTop(22));
    m_ChanCount.setBounds(top_controls);
    control_space.removeFromTop(5);

    control_space.removeFromBottom(5);
    m_StartTest.setBounds(bottom_buttons.removeFromLeft(100));
    bottom_buttons.removeFromLeft(5);
    m_Save.setBounds(bottom_buttons.removeFromLeft(100));
    m_StartCalc.setBounds(bottom_buttons.removeFromRight(100));

    processor.m_InputView.setBounds(control_space.removeFromBottom(100));
    m_ImpulseView.setBounds(control_space.removeFromTop(100));
    control_space.removeFromTop(5);
    control_space.removeFromBottom(5);

    m_Tabs.setBounds(control_space);
//    m_PlacementView.setBounds(control_space);

}

void RoCorAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &processor)
    {
        m_PlacementView.setPositionComplete(processor.getCapturePosition(), true);
        m_PlacementView.selectNext();
    }

    if (source == &processor.getIRCalc())
    {
        m_SemiModalMask.setVisible(false);
        m_Progress.setVisible(false);
        repaint();
    }
}

void RoCorAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &m_StartTest)
    {
        auto selected = m_PlacementView.getSelected();
        processor.startCapture(selected);
    }
    else if (b == &m_StartCalc)
    {
        m_SemiModalMask.setVisible(true);
        m_Progress.setVisible(true);
        repaint();
        processor.startImpulseCalc();
    }

    // TODO: These all need save UI handling
    else if (b == &m_Save)
    {
        juce::FileChooser chooser ("Select directory");
        if (chooser.browseForDirectory())
            { processor.getIRCalc().saveIndividualImpulses(m_FormatMgr.getDefaultFormat(), chooser.getResult(), processor.getSampleRate()); }
    }
    else if (b == &m_SaveAll) {}
    else if (b == &m_Load) {}
    else if (b == &m_LoadAll) {}
}

void RoCorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* changed)
{
    if (changed == & m_ChanCount) { processor.setImpulseChans(changed->getSelectedId()); }
}
