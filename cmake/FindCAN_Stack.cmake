if(NOT TARGET isobus::isobus)
  include(FetchContent)
  FetchContent_Declare(
    CAN_Stack
    GIT_REPOSITORY https://github.com/Open-Agriculture/AgIsoStack-plus-plus.git
    GIT_TAG 3499b10bda3d5164315b9ef0b646907269bac902)
  FetchContent_MakeAvailable(CAN_Stack)
endif()
