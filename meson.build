project('FMBuilder', 'cpp')

inc = include_directories('include')

meddly = meson.get_compiler('cpp').find_library('meddly')

src = ['FMBuilder.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']

executable('FMBuilder', src, dependencies : [meddly], include_directories : inc)

