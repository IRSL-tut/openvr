set(TARGET_NAME test_irsl)

## find_package(OpenCV REQUIRED)

add_executable(${TARGET_NAME}
  ## ${SHARED_SRC_FILES}
  test_sdl_vr.cpp
)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  )
target_link_libraries(${TARGET_NAME}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${OPENVR_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ## ${OpenCV_LIBRARIES}
  ${EXTRA_LIBS}
)

setTargetOutputDirectory(${TARGET_NAME})
