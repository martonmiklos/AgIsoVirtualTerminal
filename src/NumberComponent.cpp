/*******************************************************************************
** @file       NumberComponent.cpp
** @author     Miklos Marton
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "NumberComponent.hpp"

#include <iomanip>
#include <sstream>

NumberComponent::NumberComponent(
	std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet) :
	parentWorkingSet(workingSet)
{

}

Justification NumberComponent::convert_justification(
	isobus::Number::HorizontalJustification horizontalJustification,
	isobus::Number::VerticalJustification verticalJustification)
{
	Justification retVal = Justification::topLeft;

	switch (horizontalJustification)
	{
		case isobus::Number::HorizontalJustification::PositionLeft:
			{
				switch (verticalJustification)
				{
					case isobus::Number::VerticalJustification::PositionTop:
						{
							retVal = Justification::topLeft;
						}
						break;

					case isobus::Number::VerticalJustification::PositionMiddle:
						{
							retVal = Justification::centredLeft;
						}
						break;

					case isobus::Number::VerticalJustification::PositionBottom:
						{
							retVal = Justification::bottomLeft;
						}
						break;

					case isobus::Number::VerticalJustification::Reserved:
					default:
						break;
				}
			}
			break;

		case isobus::Number::HorizontalJustification::PositionMiddle:
			{
				switch (verticalJustification)
				{
					case isobus::Number::VerticalJustification::PositionTop:
						{
							retVal = Justification::centredTop;
						}
						break;

					case isobus::Number::VerticalJustification::PositionMiddle:
						{
							retVal = Justification::centred;
						}
						break;

					case isobus::Number::VerticalJustification::PositionBottom:
						{
							retVal = Justification::centredBottom;
						}
						break;

					case isobus::Number::VerticalJustification::Reserved:
					default:
						break;
				}
			}
			break;

		case isobus::Number::HorizontalJustification::PositionRight:
			{
				switch (verticalJustification)
				{
					case isobus::Number::VerticalJustification::PositionTop:
						{
							retVal = Justification::topRight;
						}
						break;

					case isobus::Number::VerticalJustification::PositionMiddle:
						{
							retVal = Justification::centredRight;
						}
						break;

					case isobus::Number::VerticalJustification::PositionBottom:
						{
							retVal = Justification::bottomRight;
						}
						break;

					case isobus::Number::VerticalJustification::Reserved:
					default:
						break;
				}
			}
			break;

		default:
			{
			}
			break;
	}
	return retVal;
}

void NumberComponent::convert_font_attribute(isobus::FontAttributes &font_attribute,
											 Font *destination_font)
{

}
