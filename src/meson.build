ren_src = files(
  'GameCore.cpp',
  'GameLauncher.cpp',
  'ResourceManager.cpp',
  'stb_image.cpp',
  'stb_image_write.cpp',
)

subdir('Renderer')

ren_inc = include_directories('include', '../include')

glad_proj = subproject('glad')
glad_dep = glad_proj.get_variable('glad_dep')
glad_inc = glad_proj.get_variable('glad_inc') # Should be temporary. For now I provide GLAD includes even to the app using this engine (Ren), so it can run GL functions.
glfw_dep = dependency('glfw3', fallback : ['glfw', 'glfw_dep'])
imgui_proj = subproject('imgui')
imgui_dep = imgui_proj.get_variable('imgui_dep')
box2d_dep = subproject('box2d').get_variable('box2d_dep')

cc = meson.get_compiler('c')
if build_machine.system() == 'linux'
  ren_depends = [
    dependency('freetype2'),
    dependency('GL'), 
    dependency('threads'),
    cc.find_library('Xrandr'),
    cc.find_library('Xi'),
    dependency('dl'),
    cc.find_library('m'),
    cc.find_library('z'),
    dependency('X11')]
elif build_machine.system() == 'windows'
  ren_depends = [
    dependency('freetype', fallback : ['freetype', 'ft_dep']),
    cc.find_library('opengl32')]
endif

ren_depends = [ren_depends, glad_dep, glfw_dep, imgui_dep, box2d_dep]

ren_lib = library('ren',
  ren_src,
  include_directories : [ren_inc, ren_conf_inc],
  dependencies : ren_depends)

ren_dep = declare_dependency(
  include_directories : [ren_inc, ren_conf_inc, glad_inc],
  link_with : ren_lib)
