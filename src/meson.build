ren_src = files(
  'Framebuffer.cpp',
  'GameCore.cpp',
  'GameLauncher.cpp',
  'Renderbuffer.cpp',
  'TextRenderer.cpp',
  'basic_renderer.cpp',
  'resource_manager.cpp',
  'shader.cpp',
  'sprite_renderer.cpp',
  'stb_image.cpp',
  'stb_image_write.cpp',
  'texture.cpp'
)

ren_inc = include_directories('include', '../include')

glad_proj = subproject('glad')
glad_dep = glad_proj.get_variable('glad_dep')
glad_inc = glad_proj.get_variable('glad_inc')
glfw_dep = dependency('glfw3', fallback : ['glfw', 'glfw_dep'])
imgui_proj = subproject('imgui')
imgui_dep = imgui_proj.get_variable('imgui_dep')
imgui_inc = imgui_proj.get_variable('imgui_inc')

cc = meson.get_compiler('c')
if build_machine.system() == 'linux'
  ren_depends = [
    dependency('freetype2'),
    glad_dep,
    glfw_dep,
    imgui_dep,
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
    glad_dep,
    glfw_dep,
    imgui_dep,
    cc.find_library('opengl32')]
endif

ren_lib = library('ren',
  ren_src,
  include_directories : [ren_inc, ren_conf_inc, imgui_inc],
  dependencies : ren_depends)

ren_dep = declare_dependency(
  include_directories : [ren_inc, ren_conf_inc, imgui_inc, glad_inc],
  link_with : ren_lib)
