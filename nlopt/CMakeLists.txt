#///////////////////////////////////////////////////////////////////////////
#//
#//    Copyright 2010
#//
#//    This file is part of rootpwa
#//
#//    rootpwa is free software: you can redistribute it and/or modify
#//    it under the terms of the GNU General Public License as published by
#//    the Free Software Foundation, either version 3 of the License, or
#//    (at your option) any later version.
#//
#//    rootpwa is distributed in the hope that it will be useful,
#//    but WITHOUT ANY WARRANTY; without even the implied warranty of
#//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//    GNU General Public License for more details.
#//
#//    You should have received a copy of the GNU General Public License
#//    along with rootpwa.  If not, see <http://www.gnu.org/licenses/>.
#//
#///////////////////////////////////////////////////////////////////////////
#//-------------------------------------------------------------------------
#//
#// Description:
#//      build file for RootPwa library
#//
#//
#// Author List:
#//      Sebastian Neubert    TUM            (original author)
#//
#//
#//-------------------------------------------------------------------------


message_setup_this_dir()


# set include directories
set(INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_SOURCE_DIR}
	${RPWA_UTILITIES_INCLUDE_DIR}
	${RPWA_DECAYAMPLITUDE_INCLUDE_DIR}
	${RPWA_PARTIALWAVEFIT_INCLUDE_DIR}
	${RPWA_PARTICLEDATA_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	SYSTEM
	${Boost_INCLUDE_DIRS}
	${Libconfig_INCLUDE_DIR}
	${ROOT_INCLUDE_DIR}
	${NLopt_INCLUDE_DIR}
	)
include_directories(${INCLUDE_DIRECTORIES})

# executables
make_executable(pwaNloptFit   pwaNloptFit.cc       ${RPWA_PARTIALWAVEFIT_LIB} ${NLopt_LIBS} ${RPWA_UTILITIES_LIB})
