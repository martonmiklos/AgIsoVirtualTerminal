/*******************************************************************************
** @file       Main.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/

#include "ASCIILogFile.hpp"
#include "AppImages.h"
#include "Main.hpp"
#include "ServerMainComponent.hpp"
#include "Settings.hpp"
#include "git.h"
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

#include "isobus/hardware_integration/gs_can_libusb.hpp"

std::shared_ptr<isobus::GS_CAN_Interface> gs_usb_interface;

class ScalingContainer : public juce::Component
{
public:
    ScalingContainer (std::unique_ptr<juce::Component> childToOwn,
                      int originalW, int originalH)
            : child (std::move(childToOwn)),
              origW (originalW), origH (originalH)
    {
        jassert (child != nullptr);
        addAndMakeVisible (*child);
        child->setBounds (0, 0, origW, origH);
    }

    void resized() override
    {
        auto W = (float) getWidth();
        auto H = (float) getHeight();

        float scale = H / (float) origH;

        const float targetW = (origW * scale);
        const float targetH = (origH * scale);
        const float offsetX = (W - targetW) * 0.5f;
        const float offsetY = (H - targetH) * 0.5f;

        child->setTransform (juce::AffineTransform::scale (scale, scale)
                                     .translated (offsetX / scale,
                                                  offsetY / scale));
    }

private:
    std::unique_ptr<juce::Component> child;
    const int   origW, origH;
};

#endif

AgISOVirtualTerminalApplication::MainWindow::MainWindow(juce::String name, int vtNumberCmdLineArg) :
  DocumentWindow(name,
                 juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                 DocumentWindow::allButtons)
{
	int vtNumber = vtNumberCmdLineArg;
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
	canDrivers.push_back(std::make_shared<isobus::MacCANPCANPlugin>(PCAN_USBBUS1)
#elif defined(JUCE_ANDROID)
    gs_usb_interface = std::make_shared<isobus::GS_CAN_Interface>();
	canDrivers.push_back(gs_usb_interface);
#elif defined(JUCE_LINUX)
	canDrivers.push_back(std::make_shared<isobus::SocketCANInterface>("can0"));
	canDrivers.push_back(std::make_shared<isobus::GS_CAN_Interface>());
#else
	LOG_WARNING("Unsupported platform, no CAN drivers available");
#endif

	jassert(!canDrivers.empty()); // You need some kind of CAN interface to run this program!
	isobus::CANHardwareInterface::set_number_of_can_channels(1);

	auto config = isobus::CANNetworkManager::CANNetwork.get_configuration();
	config.set_max_number_transport_protocol_sessions(256);
	config.set_number_of_packets_per_dpo_message(255);
	config.set_number_of_packets_per_cts_message(255);

	isobus::NAME serverNAME(0);

	Settings settings;
	if (!settings.load_settings())
	{
		{
			isobus::CANStackLogger::info("Config file not found, using defaults.");
#ifdef JUCE_LINUX
			std::static_pointer_cast<isobus::SocketCANInterface>(canDrivers.at(0))->set_name("can0");
			isobus::CANStackLogger::warn("Socket CAN interface name not yet configured. Using default of \"can0\"");
#endif
		}
	}
	else
	{
		if (0 == vtNumberCmdLineArg)
		{
			// no command line argument provided -> use the saved setting
			serverNAME.set_function_instance(settings.vt_number() - 1);
			vtNumber = settings.vt_number();
		}
		else
		{
			// VT number provided from the vtNumberCmdLineArg line
			serverNAME.set_function_instance(vtNumberCmdLineArg - 1);
			vtNumber = vtNumberCmdLineArg;
		}
	}

	serverNAME.set_arbitrary_address_capable(true);
	serverNAME.set_function_code(static_cast<std::uint8_t>(isobus::NAME::Function::VirtualTerminal));
	serverNAME.set_industry_group(2);
	serverNAME.set_manufacturer_code(1407);
	serverInternalControlFunction = isobus::CANNetworkManager::CANNetwork.create_internal_control_function(serverNAME, 0, 0x26);
	setUsingNativeTitleBar(true);

#if JUCE_ANDROID
    auto server = std::make_unique<ServerMainComponent>(
            serverInternalControlFunction, canDrivers, settings.settingsValueTree(), vtNumber);
    auto scaling = std::make_unique<ScalingContainer>(std::move(server),
                                                      server->getWidth(), server->minimum_height());
    setContentOwned(scaling.release(), false);
#else
	setContentOwned(new ServerMainComponent(serverInternalControlFunction, canDrivers, settings.settingsValueTree(), vtNumber), true);
#endif


#if defined(JUCE_IOS) || defined(JUCE_ANDROID)
	setFullScreen(true);
    Desktop::setScreenSaverEnabled(false);
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
#endif // JUCE_ANDROID
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

void AgISOVirtualTerminalApplication::MainWindow::closeButtonPressed()
{
	// This is called when the user tries to close this window. Here, we'll just
	// ask the app to quit when this happens, but you can change this to do
	// whatever you need.
	isobus::CANHardwareInterface::stop();
	JUCEApplication::getInstance()->systemRequestedQuit();
}

std::string AgISOVirtualTerminalApplication::getApplicationBuildInfo()
{
	std::string gitDescribe = std::string(git::Describe());
	if (gitDescribe.length() > 0)
	{
		return gitDescribe + (git::AnyUncommittedChanges() ? "-dirty" : "");
	}
	return ProjectInfo::versionString;
}

std::string AgISOVirtualTerminalApplication::getApplicationNameWithBuildInfo()
{
	std::string name = ProjectInfo::projectName;
	auto buildInfo = getApplicationBuildInfo();
	if (buildInfo.length() > 0)
	{
		name.append(" - " + buildInfo);
	}
	return name;
}

START_JUCE_APPLICATION(AgISOVirtualTerminalApplication)

#if JUCE_ANDROID
extern "C"
JNIEXPORT void JNICALL
Java_com_rmsl_juce_Java_set_1gs_1can_1usb_1descriptor(JNIEnv *env, jclass clazz, jint descriptor)
{
    gs_usb_interface->set_file_descriptor((int)descriptor);
}
#endif
extern "C"
JNIEXPORT void JNICALL
Java_com_rmsl_juce_Java_close_1gs_1can(JNIEnv *env, jclass clazz)
{
    if (gs_usb_interface) {
        gs_usb_interface->close();
    }
}
