//================================================================================================
/// @file SoftKeyMaskRenderAreaComponent.hpp
///
/// @brief A component to hold all the data mask render components.
/// @author Adrian Del Grosso
///
/// @copyright 2023 Adrian Del Grosso
//================================================================================================
#ifndef SOFT_KEY_MASK_RENDER_AREA_COMPONENT_HPP
#define SOFT_KEY_MASK_RENDER_AREA_COMPONENT_HPP

#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"
#include "isobus/isobus/isobus_virtual_terminal_server_managed_working_set.hpp"

#include "JuceHeader.h"

class ServerMainComponent;

class SoftKeyMaskRenderAreaComponent : public Component
{
public:
	SoftKeyMaskRenderAreaComponent(ServerMainComponent &parentServer);

	void on_change_active_mask(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet);

	void on_working_set_disconnect(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet);

	void paint(Graphics &g) override;

	// Used to calculate button press events
	void mouseDown(const MouseEvent &event) override;

	// Used to calculate button release events
	void mouseUp(const MouseEvent &event) override;

private:
	/// @brief Finds the object at the given point within a soft key mask, resolving any
	/// ObjectPointer indirection.
	/// @param[in] object The object to search, and recurse into
	/// @param[in] x The x coordinate to check, relative to object's origin
	/// @param[in] y The y coordinate to check, relative to object's origin
	/// @param[out] positionOut Set to the on-screen soft key position (the index of the clicked
	/// object among the soft key mask's own direct children - the same index
	/// SoftKeyMaskComponent::on_content_changed() assigns via KeyComponent::setKeyPosition())
	/// when a Key or an ObjectPointer resolving to one is found; left untouched otherwise.
	/// A previous version instead tried to read this back afterwards via
	/// `static_pointer_cast<KeyComponent>(clickedObject)->getKeyPosition()`, but the object this
	/// function returns always comes from the working set's object model
	/// (get_object_tree()/get_object_by_id()), never from the actual rendered KeyComponent tree
	/// that setKeyPosition() was called on - those are different C++ objects entirely, so that
	/// cast reinterpreted a plain isobus::Key as the unrelated, differently-laid-out KeyComponent
	/// type and read past the end of the real allocation. Computing the position directly here,
	/// where it is already known from the same loop index used at layout time, avoids the cast
	/// entirely.
	/// @returns The object clicked on, or nullptr if the click did not land on anything
	std::shared_ptr<isobus::VTObject> getClickedChildRecursive(std::shared_ptr<isobus::VTObject> object, int x, int y, std::uint8_t &positionOut);
	static bool objectCanBeClicked(std::shared_ptr<isobus::VTObject> object);
	static bool isClickWithinBounds(int clickXRelative, int clickYRelative, int objectX, int objectY, int objectWidth, int objectHeight);

	std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> parentWorkingSet;
	std::vector<std::shared_ptr<Component>> childComponents;
	ServerMainComponent &ownerServer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoftKeyMaskRenderAreaComponent)
};

#endif // SOFT_KEY_MASK_RENDER_AREA_COMPONENT_HPP
