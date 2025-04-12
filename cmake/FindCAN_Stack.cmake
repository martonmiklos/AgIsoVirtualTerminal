if(NOT TARGET isobus::isobus)
  include(FetchContent)
  FetchContent_Declare(
    CAN_Stack
    GIT_REPOSITORY https://github.com/martonmiklos/AgIsoStack-plus-plus.git
    GIT_TAG libusb_gs_can)
  FetchContent_MakeAvailable(CAN_Stack)
endif()
