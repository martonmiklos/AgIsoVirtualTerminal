/*******************************************************************************
** @file       Settings.cpp
** @author     Miklos Marton
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#pragma once

#include "JuceHeader.h"

#include "isobus/isobus/can_stack_logger.hpp"

/**
 * @brief The TargetAreaScaleableComponent class
 * A class to being able to resize child items recursively
 * Used in scaling the working set selector buttons and the softkeys contents to the possible area
 */
class TargetAreaScaleableComponent : public Component
{
public:
  /**
   * @brief getBoundsRecursively
   * @param parent The top level object which bounding rect will be also in the union
   * @param reference The component to which the returned bounding rect will be referenced
   * @param parentBounds the union of the processed bounding rects
   * @return the union of the bounding rect of the children relative to the reference's coordinates
   */
  Rectangle<int> getBoundsRecursively(Component *parent, Component *reference, Rectangle<int> parentBounds)
  {
    auto bounds = parentBounds;
    for (auto child : parent->getChildren())
    {
      auto childBounds = reference->getLocalArea(child, child->getLocalBounds());
      LOG_INFO("%d %d - %d %d",
               childBounds.getX(), childBounds.getY(),
               childBounds.getWidth(), childBounds.getHeight());
      bounds = childBounds.getUnion(parentBounds);
      if (0 < child->getChildren().size())
      {
        bounds = getBoundsRecursively(child, reference, bounds);
      }
    }
    return bounds;
  }

  void scaleChilds(Component *component, Rectangle<int> target, Rectangle<int> bounds, int offsetInParentX = 0, int offsetInParentY = 0)
  {
    auto sourceFloat = bounds.toFloat();
    auto targetFloat = target.toFloat();

    auto scaleX = targetFloat.getWidth() / sourceFloat.getWidth();
    auto scaleY = targetFloat.getHeight() / sourceFloat.getHeight();

    auto offsetX = targetFloat.getX() - sourceFloat.getX() * scaleX;
    auto offsetY = targetFloat.getY() - sourceFloat.getY() * scaleY;

    juce::AffineTransform transform = juce::AffineTransform::scale(scaleX, scaleY)
                                        .translated(offsetX, offsetY);

    for (auto child : component->getChildren())
    {
      child->setTransform(transform);
    }
  }
};
