/*******************************************************************************
** @file       SoftKeyMaskComponent.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "SoftKeyMaskComponent.hpp"
#include "JuceManagedWorkingSetCache.hpp"
#include "KeyComponent.hpp"
#include "ObjectPointerComponent.hpp"

SoftKeyMaskComponent::SoftKeyMaskComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::SoftKeyMask sourceObject, SoftKeyMaskDimensions dimensions) :
  isobus::SoftKeyMask(sourceObject),
  parentWorkingSet(workingSet),
  dimensionInfo(dimensions)
{
	setOpaque(true);
	setBounds(0, 0, dimensions.total_width(), dimensions.height);
	on_content_changed(true);
}

void SoftKeyMaskComponent::on_content_changed(bool initial)
{
	int row = 0;
	int x = dimensionInfo.PADDING + (dimensionInfo.columnCount - 1) * (dimensionInfo.PADDING + dimensionInfo.keyWidth);
	int y = dimensionInfo.PADDING;

	for (std::uint16_t i = 0; i < this->get_number_children(); i++)
	{
		auto child = get_object_by_id(get_child_id(i), parentWorkingSet->get_object_tree());

		if (nullptr != child)
		{
			childComponents.push_back(JuceManagedWorkingSetCache::create_component(parentWorkingSet, child));

			if (isobus::VirtualTerminalObjectType::ObjectPointer == child->get_object_type())
			{
				childComponents.back()->setSize(dimensionInfo.keyWidth, dimensionInfo.keyHeight);
				// Note: no position is recorded on whatever key this pointer resolves to. That
				// object would come from the working set's object model, not from this rendered
				// component tree, so casting it to KeyComponent to call setKeyPosition() on it
				// wrote past the end of a plain isobus::Key allocation. The click handlers in
				// SoftKeyMaskRenderAreaComponent derive the position from this same child index
				// at click time instead, which covers pointer indirection for free.
			}
			else if (isobus::VirtualTerminalObjectType::Key == child->get_object_type())
			{
				// Safe: childComponents.back() really is the KeyComponent just created above
				std::static_pointer_cast<KeyComponent>(childComponents.back())->setKeyPosition(i);
			}

			if (nullptr != childComponents.back())
			{
				addAndMakeVisible(*childComponents.back());
				childComponents.back()->setTopLeftPosition(x, y);
				y += (dimensionInfo.PADDING + dimensionInfo.keyWidth);

				row++;
				if (row >= dimensionInfo.rowCount)
				{
					row = 0;
					x -= (dimensionInfo.PADDING + dimensionInfo.keyWidth);
					y = dimensionInfo.PADDING;
				}
			}
		}
	}

	if (!initial)
	{
		repaint();
	}
}

void SoftKeyMaskComponent::paint(Graphics &g)
{
	auto vtColour = parentWorkingSet->get_colour(backgroundColor);

	g.fillAll(Colour::fromFloatRGBA(vtColour.r, vtColour.g, vtColour.b, 1.0f));
}

int SoftKeyMaskDimensions::key_count() const
{
	return columnCount * rowCount;
}

int SoftKeyMaskDimensions::total_width() const
{
	return PADDING + (columnCount * (keyWidth + PADDING));
}

int SoftKeyMaskDimensions::total_height() const
{
	return PADDING + (rowCount * (keyHeight + PADDING));
}
