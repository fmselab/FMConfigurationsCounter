project('FMConfigurationsCounter', 'cpp')

# import filesytem module
fs = import('fs')

# sometimes is in /usr/include
meddly_include_dir = '/usr/local/include/meddly'

# check if meddly is installed
if not fs.is_dir(meddly_include_dir)
 	message('meddly is not installed - trying to install it')
  	# download meddly
  	r = run_command('git','clone','https://github.com/asminer/meddly.git', check: true)
	output = r.stdout().strip()
	errortxt = r.stderr().strip()
	# TODO give the commands to compile meddly 
	# in the meddly directory
	# see https://github.com/asminer/meddly/blob/master/README
	# 
	# developers/Config_generic.sh --debug
	# 
	# ./autogen.sh
	# ./configure --prefix=/usr/local
	#
	# make
	#
	# make install (with sudo)
	# sudo
	# missing directory
    # cp oper_satur.h  /usr/local/include/meddly/
	
endif

inc = include_directories('include', meddly_include_dir)

catch_lib = subproject('catch2').get_variable('catch2_dep')

gmp_lib = meson.get_compiler('cpp').find_library('gmp')
gmp_lib2 = meson.get_compiler('cpp').find_library('gmpxx')
boost = dependency('boost', modules : ['program_options'])
meddly = meson.get_compiler('cpp').find_library('meddly')

src_experimenter = ['FMBuilderExperimenter.cpp', 'NodeFeatureVisitor.cpp', 'logger.cpp', 'ConstraintVisitor.cpp', 'Util.cpp']

executable('FMBuilderExperimenter', src_experimenter, dependencies : [gmp_lib2, gmp_lib, meddly, boost], include_directories : inc)