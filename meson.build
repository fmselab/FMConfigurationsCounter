project('FMBuilder', 'cpp')

inc = include_directories('include')

catch_lib = subproject('catch2').get_variable('catch2_dep')

gmp_lib = meson.get_compiler('cpp').find_library('gmp')

meddly = meson.get_compiler('cpp').find_library('meddly')

src = ['FMBuilder.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']

src_test = ['FMBuilderTest.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']


executable('FMBuilder', src, dependencies : [meddly], include_directories : inc)

FMBuilderTest = executable('FMBuilderTest', src_test, dependencies : [meddly, catch_lib, gmp_lib], include_directories : inc)
test('FMBuilderTest', FMBuilderTest)
