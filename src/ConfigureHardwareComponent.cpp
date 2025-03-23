/*******************************************************************************
** @file       ConfigureHardwareComponent.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "ConfigureHardwareComponent.hpp"

#include "ConfigureHardwareWindow.hpp"
#include "ServerMainComponent.hpp"
#include "isobus/isobus/can_stack_logger.hpp"
#include "isobus/utility/to_string.hpp"

#ifdef JUCE_WINDOWS
#include "isobus/hardware_integration/toucan_vscp_canal.hpp"
#elif JUCE_LINUX
#include "isobus/hardware_integration/gs_can_libusb.hpp"
#include "isobus/hardware_integration/socket_can_interface.hpp"
#endif

ConfigureHardwareComponent::ConfigureHardwareComponent(ConfigureHardwareWindow &parent, std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> &canDrivers) :
  okButton("OK"),
  parentCANDrivers(canDrivers)
{
	setSize(400, 280);
	okButton.setSize(100, 30);
	okButton.setTopLeftPosition(getWidth() / 2 - okButton.getWidth() / 2, 200);
	addAndMakeVisible(okButton);

	hardwareInterfaceSelector.setName("Hardware Interface");
	hardwareInterfaceSelector.setTextWhenNothingSelected("Select Hardware Interface");

#if JUCE_WINDOWS
#ifdef ISOBUS_WINDOWSINNOMAKERUSB2CAN_AVAILABLE
	hardwareInterfaceSelector.addItemList({ "PEAK PCAN USB", "Innomaker2CAN", "TouCAN", "SysTec" }, 1);
#else
	hardwareInterfaceSelector.addItemList({ "PEAK PCAN USB", "Innomaker2CAN (not supported with mingw)", "TouCAN", "SysTec" }, 1);
#endif
#elif JUCE_LINUX
	hardwareInterfaceSelector.addItemList({ "SocketCAN", "GS USB" }, 1);
#endif
	int selectedID = 1;

	for (std::uint8_t i = 0; i < parentCANDrivers.size(); i++)
	{
		if ((nullptr != parentCANDrivers.at(i)) &&
		    (parentCANDrivers.at(i) == isobus::CANHardwareInterface::get_assigned_can_channel_frame_handler(0)))
		{
			selectedID = i + 1;
			break;
		}
	}
	hardwareInterfaceSelector.setSelectedId(selectedID);
	hardwareInterfaceSelector.setSize(getWidth() - 20, 30);
	hardwareInterfaceSelector.setTopLeftPosition(10, 80);

#ifdef JUCE_WINDWOS
	hardwareInterfaceSelector.onChange = [this]() {
		if (3 == hardwareInterfaceSelector.getSelectedId())
		{
			touCANSerialEditor.setVisible(true);
		}
		else
		{
			touCANSerialEditor.setVisible(false);
		}
		repaint();
	};
#elif JUCE_LINUX
#endif
	addAndMakeVisible(hardwareInterfaceSelector);

#ifdef JUCE_WINDWOS
	auto inputFilter = new TextEditor::LengthAndCharacterRestriction(10, "1234567890");
	touCANSerialEditor.setName("TouCAN Serial Number");
	touCANSerialEditor.setText(isobus::to_string(std::static_pointer_cast<isobus::TouCANPlugin>(parentCANDrivers.at(2))->get_serial_number()));
	touCANSerialEditor.setSize(getWidth() - 20, 30);
	touCANSerialEditor.setTopLeftPosition(10, 140);
	touCANSerialEditor.setInputFilter(inputFilter, true);
	addChildComponent(touCANSerialEditor);
#elif JUCE_LINUX
	socketCanDeviceNameEditor.setName("SocketCAN Interface Name");
	socketCanDeviceNameEditor.setText(std::static_pointer_cast<isobus::SocketCANInterface>(parentCANDrivers.at(0))->get_device_name());
	socketCanDeviceNameEditor.setSize(getWidth() - 20, 30);
	socketCanDeviceNameEditor.setTopLeftPosition(10, 140);
	addAndMakeVisible(socketCanDeviceNameEditor);
#endif
	okButton.onClick = [this, &parent]() {
		parent.setVisible(false);

#ifdef JUCE_WINDOWS
		if (3 == hardwareInterfaceSelector.getSelectedId()) // TouCAN
		{
			int serial = touCANSerialEditor.getText().trim().getIntValue();
			std::static_pointer_cast<isobus::TouCANPlugin>(parentCANDrivers.at(hardwareInterfaceSelector.getSelectedId() - 1))->reconfigure(0, static_cast<std::uint32_t>(serial));
		}

		if (nullptr != isobus::CANHardwareInterface::get_assigned_can_channel_frame_handler(0))
		{
			isobus::CANHardwareInterface::unassign_can_channel_frame_handler(0);
		}
		isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, parentCANDrivers.at(hardwareInterfaceSelector.getSelectedId() - 1));
		isobus::CANStackLogger::info("Updated assigned CAN driver.");
#elif JUCE_LINUX
		if (0 == hardwareInterfaceSelector.getSelectedId())
		{
			std::static_pointer_cast<isobus::SocketCANInterface>(parentCANDrivers.at(0))->set_name(socketCanDeviceNameEditor.getText().toStdString());
			LOG_INFO("Updated socket CAN interface name to: " + socketCanDeviceNameEditor.getText().toStdString());
		}
		else if (1 == hardwareInterfaceSelector.getSelectedId())
		{
			std::static_pointer_cast<isobus::GS_CAN_Interface>(parentCANDrivers.at(1))->set_serial(socketCanDeviceNameEditor.getText().toStdString());
			LOG_INFO("Updated GS CAN target serial to: " + socketCanDeviceNameEditor.getText().toStdString());
		}

#endif
		parent.parentServer.save_settings();
	};
}

void ConfigureHardwareComponent::paint(Graphics &graphics)
{
	auto bounds = getLocalBounds();
	graphics.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	graphics.setColour(getLookAndFeel().findColour(Label::textColourId));
	graphics.setFont(16.0f);
#if defined(JUCE_WINDOWS) || defined(JUCE_LINUX)
	graphics.drawFittedText("Select the CAN driver to use", 10, 10, bounds.getWidth() - 20, 54, Justification::centredTop, 3);
#endif

	graphics.setFont(12.0f);

#if defined(JUCE_WINDOWS) || defined(JUCE_LINUX)
	graphics.drawFittedText("Hardware Driver", hardwareInterfaceSelector.getBounds().getX(), hardwareInterfaceSelector.getBounds().getY() - 14, hardwareInterfaceSelector.getBounds().getWidth(), 12, Justification::centredLeft, 1);
#endif

#ifdef JUCE_WINDOWS
	if (3 == hardwareInterfaceSelector.getSelectedId())
	{
		graphics.drawFittedText("TouCAN Serial Number", touCANSerialEditor.getBounds().getX(), touCANSerialEditor.getBounds().getY() - 14, touCANSerialEditor.getBounds().getWidth(), 12, Justification::centredLeft, 1);
	}
#elif JUCE_LINUX
	if (0 == hardwareInterfaceSelector.getSelectedId())
	{
		graphics.drawFittedText("Socket CAN Interface Name", socketCanDeviceNameEditor.getBounds().getX(), socketCanDeviceNameEditor.getBounds().getY() - 14, socketCanDeviceNameEditor.getBounds().getWidth(), 12, Justification::centredLeft, 1);
	}
	else if (1 == hardwareInterfaceSelector.getSelectedId())
	{
		graphics.drawFittedText("GS CAN Interface Serial number", socketCanDeviceNameEditor.getBounds().getX(), socketCanDeviceNameEditor.getBounds().getY() - 14, socketCanDeviceNameEditor.getBounds().getWidth(), 12, Justification::centredLeft, 1);
	}
#endif
}

void ConfigureHardwareComponent::resized()
{
}
