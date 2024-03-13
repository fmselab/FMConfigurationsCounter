project('FMBuilder', 'cpp')

inc = include_directories('include', '/usr/include/meddly')

catch_lib = subproject('catch2').get_variable('catch2_dep')

gmp_lib = meson.get_compiler('cpp').find_library('gmp')
gmp_lib2 = meson.get_compiler('cpp').find_library('gmpxx')

meddly = meson.get_compiler('cpp').find_library('meddly')

src = ['FMBuilder.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']

src_test = ['FMBuilderTest.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']


executable('FMBuilder', src, dependencies : [gmp_lib2, gmp_lib, meddly], include_directories : inc)

FMBuilderTest = executable('FMBuilderTest', src_test, dependencies : [gmp_lib, gmp_lib2, meddly, catch_lib], include_directories : inc)
test('FMBuilderTest', FMBuilderTest)
