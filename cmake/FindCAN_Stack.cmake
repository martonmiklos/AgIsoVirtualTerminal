if(NOT TARGET isobus::isobus)
  include(FetchContent)
  FetchContent_Declare(
    CAN_Stack
    GIT_REPOSITORY https://github.com/Open-Agriculture/AgIsoStack-plus-plus.git
    GIT_TAG 5c52293ee2c426c29edb5f454fee6d227f71d6d2)
  FetchContent_MakeAvailable(CAN_Stack)
endif()
