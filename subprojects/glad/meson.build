project('GLAD dep', 'c')

glad_inc = include_directories('include')

glad = library('glad', 'glad.c', include_directories : glad_inc)
glad_dep = declare_dependency(include_directories : glad_inc, link_with : glad)
