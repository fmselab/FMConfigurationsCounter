project('FMBuilder', 'cpp')

inc = include_directories('include', '/usr/include/meddly')

catch_lib = subproject('catch2').get_variable('catch2_dep')

gmp_lib = meson.get_compiler('cpp').find_library('gmp')
gmp_lib2 = meson.get_compiler('cpp').find_library('gmpxx')
boost = dependency('boost', modules : ['program_options'])
meddly = meson.get_compiler('cpp').find_library('meddly')

src_experimenter = ['FMBuilderExperimenter.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']

executable('FMBuilderExperimenter', src_experimenter, dependencies : [gmp_lib2, gmp_lib, meddly, boost], include_directories : inc)