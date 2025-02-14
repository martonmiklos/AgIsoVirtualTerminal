/*******************************************************************************
** @file       OutputEllipseComponent.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "OutputEllipseComponent.hpp"
#include "JuceManagedWorkingSetCache.hpp"

OutputEllipseComponent::OutputEllipseComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::OutputEllipse sourceObject) :
  isobus::OutputEllipse(sourceObject),
  parentWorkingSet(workingSet)
{
	setSize(get_width(), get_height());
}

void OutputEllipseComponent::paint(Graphics &g)
{
	bool fillNeeded = false;
	isobus::VTColourVector fillColour;
	// Ensure we fill first, then draw the outline if needed
	if (isobus::NULL_OBJECT_ID != get_fill_attributes())
	{
		auto child = get_object_by_id(get_fill_attributes(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::FillAttributes == child->get_object_type()))
		{
			auto fill = std::static_pointer_cast<isobus::FillAttributes>(child);
			fillColour = parentWorkingSet->get_colour(fill->get_background_color());
			fillNeeded = true;
		}
	}

	if (isobus::NULL_OBJECT_ID != get_line_attributes())
	{
		auto child = get_object_by_id(get_line_attributes(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::LineAttributes == child->get_object_type()))
		{
			auto line = std::static_pointer_cast<isobus::LineAttributes>(child);

			auto vtColour = parentWorkingSet->get_colour(line->get_background_color());

			float centerX = get_width() / 2.0f;
			float centerY = get_height() / 2.0f;

			if (get_ellipse_type() == isobus::OutputEllipse::EllipseType::Closed)
			{
				if (get_start_angle() == get_end_angle() && get_ellipse_type() != isobus::OutputEllipse::EllipseType::Closed)
				{
					/* If type = closed ellipse segment and start and end angle are the same, a
					 * single line with width = border width shall be drawn from the centre point to the
					 * point on the border defined by the start and end angles.*/
					auto angleRadians = degreesToRadians(-((get_start_angle() * 2.0) - 90));
					if (angleRadians < 0)
						angleRadians += 2 * M_PI;

					auto pointX = (centerX + (centerX - (line->get_width() / 2.0f)) * std::cos(angleRadians));
					auto pointY = (centerY - (centerY - (line->get_height() / 2.0f)) * std::sin(angleRadians));

					g.setColour(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
					g.drawLine(centerX, centerY, pointX, pointY, line->get_width());
				}
				else
				{
					if (fillNeeded)
					{
						g.setColour(Colour::fromFloatRGBA(fillColour.r, fillColour.g, fillColour.b, 1.0f));
						g.fillEllipse(0, 0, get_width(), get_height());
					}
					/* If type > 0 (!= Closed) and start and end angles are the same, the ellipse is drawn closed. */
					g.setColour(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
					g.drawEllipse(line->get_width() / 2.0f, line->get_width() / 2.0f, get_width() - line->get_width(), get_height() - line->get_width() - 1, line->get_width());
				}
			}
			else
			{
				// Juce coordinate system 0° is at the Y axis positive, calculating clockwise
				// IsoBus coordinate system 0° is at the X axis positive, calculating counter-clockwise
				float startAngle = juce::degreesToRadians(-((get_start_angle() * 2.0f) - 90));
				float endAngle = juce::degreesToRadians(-((get_end_angle() * 2.0f) - 90));

				if (endAngle > startAngle)
					startAngle += 2 * M_PI;

				juce::Path arcPath;
				if (get_ellipse_type() == isobus::OutputEllipse::EllipseType::ClosedEllipseSegment)
				{
					// segment: the ellipse section endpoints connected to the center with two lines
					arcPath.startNewSubPath(centerX, centerY);
				}

				arcPath.addArc(line->get_width() / 2.0f, line->get_width() / 2.0f, get_width() - line->get_width() / 2.0f, get_height() - line->get_height() / 2.0f, startAngle, endAngle, get_ellipse_type() != isobus::OutputEllipse::EllipseType::ClosedEllipseSegment);

				if (get_ellipse_type() == isobus::OutputEllipse::EllipseType::ClosedEllipseSegment)
				{
					// segment: the ellipse section endpoints connected to the center with two lines
					arcPath.lineTo(centerX, centerY);
					if (fillNeeded)
					{
						g.setColour(Colour::fromFloatRGBA(fillColour.r, fillColour.g, fillColour.b, 1.0f));
						g.fillPath(arcPath);
					}

					g.setColour(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
					g.strokePath(arcPath, juce::PathStrokeType(line->get_width()));
				}
				else if (get_ellipse_type() == isobus::OutputEllipse::EllipseType::ClosedEllipseSection)
				{
					// section: the ellipse section endpoints connected with a straight line
					arcPath.closeSubPath();
					if (fillNeeded)
					{
						g.setColour(Colour::fromFloatRGBA(fillColour.r, fillColour.g, fillColour.b, 1.0f));
						g.fillPath(arcPath);
					}

					g.setColour(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
					g.strokePath(arcPath, juce::PathStrokeType(line->get_width()));
				}
				else if (get_ellipse_type() == isobus::OutputEllipse::EllipseType::OpenDefinedByStartEndAngles)
				{
					g.setColour(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
					g.strokePath(arcPath, juce::PathStrokeType(line->get_width()));
				}
			}
		}
	}
}
