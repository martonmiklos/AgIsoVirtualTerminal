/*******************************************************************************
** @file       InputNumberComponent.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "InputNumberComponent.hpp"

#include <iomanip>
#include <sstream>

InputNumberComponent::InputNumberComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::InputNumber sourceObject) :
  isobus::InputNumber(sourceObject),
  NumberComponent(workingSet)
{
	setSize(get_width(), get_height());
	setSourceObject(this);

	if (get_option(TextualVTObject::Options::Transparent))
	{
		setOpaque(false);
	}
	else
	{
		setOpaque(true);
	}
}

