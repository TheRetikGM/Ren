project('Ren', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'], version: '0.1')

ren_conf = configuration_data()
ren_conf.set('ENGINE_SRC_DIR', meson.project_source_root())
ren_conf.set('PLATFORM_WINDOWS', build_machine.system() == 'windows')
configure_file(
  input : 'engine_config.h.in',
  output : 'engine_config.h',
  configuration : ren_conf)
ren_conf_inc = include_directories('.')

subdir('src')
