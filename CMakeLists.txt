if(CONFIG_ZMK_RGB_UNDERGLOW)
  target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/layer_colors.c)
endif()
