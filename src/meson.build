
libgl         = dependency('GL')
libsdl2       = dependency('SDL2')
libsdl2_mixer = dependency('SDL2_mixer')

executable('dungeons-of-daggorath',
           'creature.cpp',
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
           'utils.cpp',
           'viewer.cpp',
           implicit_include_directories: false,
           include_directories: top_inc,
           dependencies: [libgl, libsdl2, libsdl2_mixer],
           install: true)
