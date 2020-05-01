//
// Created by mzapp on 4/15/18.
//
#pragma once

#include <JuceHeader.h>
#include <rocor/guts/RoCorAudioProcessor.h>
#include <rocor/gooey/RoCorAudioProcessorEditor.h>
#include "RoCorComponent.h"

namespace CommandIDs
{
#if ! (JUCE_IOS || JUCE_ANDROID)
    static const int open                   = 0x30000;
    static const int save                   = 0x30001;
    static const int saveAs                 = 0x30002;
    static const int newFile                = 0x30003;
#endif
    static const int showAudioSettings      = 0x30100;
}

class RoCorApp
    : public juce::JUCEApplication
{
public:
    RoCorApp() {}

    const juce::String getApplicationName() override { return "RoCor"; }
    const juce::String getApplicationVersion() override { return "0.01"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& commandLine) override
    {
        juce::PropertiesFile::Options options;
        options.applicationName     = "RoCor";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Preferences";

        appProperties.reset(new juce::ApplicationProperties());
        appProperties->setStorageParameters(options);

        mainWindow.reset(new RoCorInterfaceWindow(getApplicationName()));
        mainWindow->setUsingNativeTitleBar(true);

        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (mainWindow.get());
        mainWindow->menuItemsChanged();
    }

    void shutdown() override { mainWindow = nullptr; }
    void anotherInstanceStarted(const juce::String& commandLine) override {}

    class RoCorInterfaceWindow
        : public juce::DocumentWindow
        , public juce::MenuBarModel
        , public juce::ApplicationCommandTarget
    {
    public:
        RoCorInterfaceWindow(juce::String name);

        ~RoCorInterfaceWindow();

        juce::StringArray getMenuBarNames() override;
        juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName) override;
        void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
        juce::ApplicationCommandTarget* getNextCommandTarget() override;
        void getAllCommands (juce::Array<juce::CommandID>&) override;
        void getCommandInfo (juce::CommandID, juce::ApplicationCommandInfo&) override;
        bool perform (const InvocationInfo&) override;

        void showAudioSettings();

        void closeButtonPressed() override
            { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
        juce::AudioDeviceManager m_DeviceManager;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorInterfaceWindow)
    };

    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            juce::JUCEApplicationBase::quit();
    }

    juce::ApplicationCommandManager commandManager;
    std::unique_ptr<juce::ApplicationProperties> appProperties;

private:
    std::unique_ptr<RoCorInterfaceWindow> mainWindow;

};

static RoCorApp& getApp()
    { return *dynamic_cast<RoCorApp*>(juce::JUCEApplication::getInstance()); }
juce::ApplicationProperties& getAppProperties() { return *getApp().appProperties; }
juce::ApplicationCommandManager& getCommandManager() { return getApp().commandManager; }
bool isOnTouchDevice() { return juce::Desktop::getInstance().getMainMouseSource().isTouch(); }

START_JUCE_APPLICATION(RoCorApp);
