project('GLFW dep', 'c')

glfw_inc = include_directories('include')

cc = meson.get_compiler('c')
glfw_dep = declare_dependency(
  dependencies : cc.find_library('glfw3', dirs : meson.current_source_dir()),
  include_directories : glfw_inc)
