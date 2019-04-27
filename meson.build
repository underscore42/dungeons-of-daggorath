project('dungeons-of-daggorath', 'cpp', version: '0.6-dev', default_options: ['cpp_std=c++17'])

conf_data = configuration_data()
conf_data.set('dod_version', meson.project_version())
conf_data.set('dod_asset_path', join_paths(get_option('prefix'), get_option('datadir')))

configure_file(input : 'config.h.in',
               output: 'config.h',
               configuration: conf_data)

top_inc = include_directories('.')

subdir('src')

install_subdir('sound', install_dir: join_paths(conf_data.get('dod_asset_path'), meson.project_name()))
