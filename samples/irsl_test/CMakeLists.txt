set(TARGET_NAME test_irsl)

add_executable(${TARGET_NAME}
  ## ${SHARED_SRC_FILES}
  test_sdl_vr.cpp
)

target_link_libraries(${TARGET_NAME}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${OPENVR_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${EXTRA_LIBS}
)

setTargetOutputDirectory(${TARGET_NAME})
