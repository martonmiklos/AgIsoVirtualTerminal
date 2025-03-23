/*******************************************************************************
** @file       Main.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "isobus/hardware_integration/available_can_drivers.hpp"

#include <JuceHeader.h>
#include "ASCIILogFile.hpp"
#include "AppImages.h"
#include "ServerMainComponent.hpp"
#include "isobus/hardware_integration/can_hardware_interface.hpp"
#include "isobus/isobus/can_internal_control_function.hpp"
#include "isobus/isobus/can_network_manager.hpp"
#if JUCE_ANDROID
#include "jni.h"
#include "juce_core/native/juce_JNIHelpers_android.h"
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK)\
     METHOD (addFlags, "addFlags", "(I)V") \
     METHOD (clearFlags, "clearFlags", "(I)V") \

DECLARE_JNI_CLASS (Window, "android/view/Window")
#undef JNI_CLASS_MEMBERS

#endif
//==============================================================================
class AgISOVirtualTerminalApplication : public juce::JUCEApplication
{
public:
	//==============================================================================
	AgISOVirtualTerminalApplication() {}

	const juce::String getApplicationName() override
	{
		return ProjectInfo::projectName;
	}
	const juce::String getApplicationVersion() override
	{
		return ProjectInfo::versionString;
	}
	bool moreThanOneInstanceAllowed() override
	{
		return true;
	}

	//==============================================================================
	void initialise(const juce::String &) override
	{
		// This method is where you should put your application's initialisation code..

		mainWindow.reset(new MainWindow(getApplicationName()));
	}

	void shutdown() override
	{
		// Add your application's shutdown code here..

		mainWindow = nullptr; // (deletes our window)
	}

	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted(const juce::String &) override
	{
		// When another instance of the app is launched while this one is running,
		// this method is invoked, and the commandLine parameter tells you what
		// the other instance's command-line arguments were.
	}

	//==============================================================================
	/*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
	class MainWindow : public juce::DocumentWindow
	{
	public:
		MainWindow(juce::String name) :
		  DocumentWindow(name,
		                 juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
		                 DocumentWindow::allButtons)
		{
#ifdef JUCE_WINDOWS
			canDrivers.push_back(std::make_shared<isobus::PCANBasicWindowsPlugin>(static_cast<WORD>(PCAN_USBBUS1)));
#ifdef ISOBUS_WINDOWSINNOMAKERUSB2CAN_AVAILABLE
			canDrivers.push_back(std::make_shared<isobus::InnoMakerUSB2CANWindowsPlugin>(0));
#else
			canDrivers.push_back(nullptr);
#endif
			canDrivers.push_back(std::make_shared<isobus::TouCANPlugin>(static_cast<std::int16_t>(0), 0));
			canDrivers.push_back(std::make_shared<isobus::SysTecWindowsPlugin>());
#elif defined(JUCE_MAC)
			canDrivers.push_back(std::make_shared<isobus::MacCANPCANPlugin>(PCAN_USBBUS1));
#else
			canDrivers.push_back(std::make_shared<isobus::SocketCANInterface>("can0"));
#endif

			jassert(!canDrivers.empty()); // You need some kind of CAN interface to run this program!
			isobus::CANHardwareInterface::set_number_of_can_channels(1);

#ifndef JUCE_WINDOWS
			isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, canDrivers.at(0));
#endif

			isobus::NAME serverNAME(0);
			serverNAME.set_arbitrary_address_capable(true);
			serverNAME.set_function_code(static_cast<std::uint8_t>(isobus::NAME::Function::VirtualTerminal));
			serverNAME.set_industry_group(2);
			serverNAME.set_manufacturer_code(1407);
			serverInternalControlFunction = isobus::CANNetworkManager::CANNetwork.create_internal_control_function(serverNAME, 0, 0x26);
			setUsingNativeTitleBar(true);
			setContentOwned(new ServerMainComponent(serverInternalControlFunction, canDrivers), true);

#if JUCE_IOS || JUCE_ANDROID
			setFullScreen(true);
#if JUCE_ANDROID
            if (auto* env = juce::getEnv())
            {
                auto activity = juce::getMainActivity();
                if (activity.get() == nullptr)
                {
                    return;
                }

                jclass activityClass = env->GetObjectClass(activity.get());
                jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
                if (getWindowMethod == nullptr)
                {
                    return;
                }

                jobject window = env->CallObjectMethod(activity.get(), getWindowMethod);
                if (window == nullptr)
                {
                    return;
                }

                jclass windowClass = env->GetObjectClass(window);
                jmethodID addFlagsMethod = env->GetMethodID(windowClass, "addFlags", "(I)V");
                if (addFlagsMethod == nullptr)
                {
                    return;
                }

                env->CallVoidMethod(window, addFlagsMethod, 0x00000080);
            }
#endif
#else
			setResizable(true, true);
			centreWithSize(getWidth(), getHeight());
#endif

			setIcon(ImageCache::getFromMemory(AppImages::logosmall_png, AppImages::logosmall_pngSize));
#if JUCE_LINUX
			// this hack is needed on Linux
			ComponentPeer *peer = getPeer();
			if (peer)
			{
				peer->setIcon(ImageCache::getFromMemory(AppImages::logosmall_png, AppImages::logosmall_pngSize));
			}
#endif
			setVisible(true);
		}

		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens, but you can change this to do
			// whatever you need.
			isobus::CANHardwareInterface::stop();
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

		/* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

	private:
		std::shared_ptr<isobus::InternalControlFunction> serverInternalControlFunction;
		std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> canDrivers;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

private:
	std::unique_ptr<MainWindow> mainWindow;
	ASCIILogFile logFile;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(AgISOVirtualTerminalApplication)
