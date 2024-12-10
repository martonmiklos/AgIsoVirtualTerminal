/*******************************************************************************
** @file       OutputNumberComponent.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "OutputNumberComponent.hpp"

#include <iomanip>
#include <sstream>

OutputNumberComponent::OutputNumberComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::OutputNumber sourceObject) :
  isobus::OutputNumber(sourceObject),
  NumberComponent(workingSet)
{
	setSize(get_width(), get_height());

	if (get_option(Options::Transparent))
	{
		setOpaque(false);
	}
	else
	{
		setOpaque(true);
	}
}

void OutputNumberComponent::paint(Graphics &g)
{
	if (isOpaque())
	{
		auto vtColour = parentWorkingSet->get_colour(backgroundColor);
		g.fillAll(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
	}

	float scaledValue = (get_value() + get_offset()) * get_scale();
	g.setColour(getLookAndFeel().findColour(ListBox::textColourId));

	// Get font data
	if (isobus::NULL_OBJECT_ID != get_font_attributes())
	{
		auto child = get_object_by_id(get_font_attributes(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::FontAttributes == child->get_object_type()))
		{
			auto font = std::static_pointer_cast<isobus::FontAttributes>(child);

			auto colour = parentWorkingSet->get_colour(font->get_colour());
			Font juceFont(Font::getDefaultMonospacedFontName(), font->get_font_height_pixels(), Font::FontStyleFlags::plain);
			auto fontWidth = juceFont.getStringWidthFloat("1");
			juceFont.setHorizontalScale(static_cast<float>(font->get_font_width_pixels()) / fontWidth);
			g.setColour(Colour::fromFloatRGBA(colour.r, colour.g, colour.b, 1.0f));
			g.setFont(juceFont);
		}
	}

	if (isobus::NULL_OBJECT_ID != get_variable_reference())
	{
		auto child = get_object_by_id(get_variable_reference(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::NumberVariable == child->get_object_type()))
		{
			std::int64_t offsetValue = static_cast<std::int64_t>(std::static_pointer_cast<isobus::NumberVariable>(child)->get_value()) + get_offset();
			scaledValue = offsetValue * get_scale();
		}
	}

	std::ostringstream valueText;
	valueText << std::fixed << std::setprecision(get_number_of_decimals()) << scaledValue;
	g.drawText(valueText.str(), 0, 0, get_width(), get_height(), convert_justification(get_horizontal_justification(), get_vertical_justification()), false);
}

