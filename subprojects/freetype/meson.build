project('FreeType dep', 'c')

cc = meson.get_compiler('c')
ft_dep = declare_dependency(
  dependencies : cc.find_library('freetype', dirs : meson.current_source_dir()),
  include_directories : include_directories('include'))
