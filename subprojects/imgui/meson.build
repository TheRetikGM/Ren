project('Dear ImGui', 'cpp', version : '1.87')

glfw_dep = dependency('glfw3', ['glfw', 'glfw_dep'])

imgui_inc = include_directories(
  'src',
  'src/backends')
imgui_src = files(
  'src/imgui.cpp',
  'src/imgui_demo.cpp',
  'src/imgui_draw.cpp',
  'src/imgui_impl_glfw.cpp',
  'src/imgui_impl_opengl3.cpp',
  'src/imgui_stdlib.cpp',
  'src/imgui_tables.cpp',
  'src/imgui_widgets.cpp')

imgui_lib = library('imgui', 
	imgui_src, 
	include_directories : [imgui_inc],
        dependencies : glfw_dep)
	
imgui_dep = declare_dependency(
	include_directories : imgui_inc,
	link_with : imgui_lib)
