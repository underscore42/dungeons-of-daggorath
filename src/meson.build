
libgl         = dependency('GL')
libsdl2       = dependency('SDL2')
libsdl2_mixer = dependency('SDL2_mixer')

executable('dod', 'creature.cpp',
                  'dod.cpp',
                  'dodgame.cpp',
                  'dungeon.cpp',
                  'enhanced.cpp',
                  'object.cpp',
                  'oslink.cpp',
                  'parser.cpp',
                  'player.cpp',
                  'rng.cpp',
                  'sched.cpp',
                  'viewer.cpp',
                  implicit_include_directories : false,
                  dependencies: [libgl, libsdl2, libsdl2_mixer],
                  cpp_args: ['-DLINUX'])

