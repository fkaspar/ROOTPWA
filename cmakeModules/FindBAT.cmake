#//-------------------------------------------------------------------------
#//
#// Description:
#//      cmake module for finding BAT installation
#//      BAT installation location is defined by environment variable
#//      $BATINSTALLDIR
#//
#//      following variables are defined:
#//      BAT_VERSION        - BAT version
#//      BAT_ROOT_DIR       - BAT installation directory
#//      BAT_INCLUDE_DIR    - BAT header directory
#//      BAT_LIBRARIES      - BAT libraries
#//      BAT_LINKER_FLAGS   - extra linker flags required to link against
#//                           BAT
#//
#//      Example usage:
#//          find_package(BAT 0.9.3 REQUIRED)
#//
#//
#//-------------------------------------------------------------------------


set(BAT_FOUND        FALSE)
set(BAT_ERROR_REASON "")
set(BAT_VERSION      )
set(BAT_ROOT_DIR     "")
set(BAT_INCLUDE_DIR  "")
set(BAT_LIBRARIES    )
set(BAT_LINKER_FLAGS "")


set(BAT_ROOT_DIR $ENV{BATINSTALLDIR})
if(BAT_ROOT_DIR)

	set(BAT_FOUND TRUE)

	set(BAT_INCLUDE_DIR "${BAT_ROOT_DIR}/include")
	if(NOT EXISTS "${BAT_INCLUDE_DIR}")
		set(BAT_FOUND FALSE)
		set(BAT_ERROR_REASON "${BAT_ERROR_REASON} Directory '${BAT_INCLUDE_DIR}' does not exist.")
	endif()

	set(_BAT_LIBRARY_DIR "${BAT_ROOT_DIR}/lib")
	if(NOT EXISTS "${_BAT_LIBRARY_DIR}")
		set(BAT_FOUND FALSE)
		set(BAT_ERROR_REASON "${BAT_ERROR_REASON} Directory '${_BAT_LIBRARY_DIR}' does not exist.")
	else()
		set(_BAT_LIBRARY_NAME "BAT")
		find_library(BAT_LIBRARIES
			NAMES ${_BAT_LIBRARY_NAME}
			PATHS ${_BAT_LIBRARY_DIR}
			NO_DEFAULT_PATH)
		if(NOT BAT_LIBRARIES)
			set(BAT_FOUND FALSE)
			set(BAT_ERROR_REASON "${BAT_ERROR_REASON} Cannot find libconfig library '${_BAT_LIBRARY_NAME}' in '${_BAT_LIBRARY_DIR}'.")
		endif()
		unset(_BAT_LIBRARY_NAME)
	endif()
	unset(_BAT_LIBRARY_DIR)

	set(_BAT_CONFIG_HEADER_FILE_NAME "${BAT_ROOT_DIR}/config.h")
	if(NOT EXISTS "${_BAT_CONFIG_HEADER_FILE_NAME}")
		set(BAT_FOUND FALSE)
		set(BAT_ERROR_REASON "${BAT_ERROR_REASON} BAT configuration '${_BAT_CONFIG_HEADER_FILE_NAME}' does not exist.")
	else()
		# parse version string
		file(STRINGS ${_BAT_CONFIG_HEADER_FILE_NAME} _BAT_VERSION
			REGEX " VERSION ")
		# there are two versions of the version string around,
		# the first (and more recent) contains four parts of
		# numbers divided by dots
		string(REGEX REPLACE
			"#define VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)\""
			"\\1" BAT_VERSION "${_BAT_VERSION}")
		if(BAT_VERSION STREQUAL _BAT_VERSION)
			# otherwise try to extract the version from only
			# three parts
			string(REGEX REPLACE
				"#define VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\""
				"\\1" BAT_VERSION "${_BAT_VERSION}")
		endif()
		if(BAT_VERSION STREQUAL _BAT_VERSION)
			set(BAT_FOUND FALSE)
			set(BAT_ERROR_REASON "${BAT_ERROR_REASON} Could not extract BAT version from version string '${_BAT_VERSION}'.")
		endif()
		unset(_BAT_VERSION)

		# build with OpenMP support
		file(STRINGS ${_BAT_CONFIG_HEADER_FILE_NAME} _BAT_OPENMP
			REGEX " THREAD_PARALLELIZATION ")
		string(REGEX REPLACE
			"#define THREAD_PARALLELIZATION ([01])"
			"\\1" _BAT_OPENMP_STATUS "${_BAT_OPENMP}")
		if(_BAT_OPENMP_STATUS STREQUAL _BAT_OPENMP)
			set(BAT_FOUND FALSE)
			set(BAT_ERROR_REASON "${BAT_ERROR_REASON} Could not extract whether BAT was build with threading support from string '${_BAT_OPENMP}'.")
		endif()
		if(_BAT_OPENMP_STATUS)
			set(BAT_LINKER_FLAGS "${BAT_LINKER_FLAGS} -fopenmp")
		endif()
		unset(_BAT_OPENMP)
		unset(_BAT_OPENMP_STATUS)
	endif()
	unset(_BAT_CONFIG_HEADER_FILE_NAME)

	# compare version
	if(BAT_FIND_VERSION_EXACT)
		if(NOT BAT_VERSION VERSION_EQUAL BAT_FIND_VERSION)
			set(BAT_FOUND FALSE)
			set(BAT_ERROR_REASON "${BAT_ERROR_REASON} BAT version ${BAT_VERSION} does not match requested version ${BAT_FIND_VERSION}.")
		endif()
	else()
		if(BAT_VERSION VERSION_LESS BAT_FIND_VERSION)
			set(BAT_FOUND FALSE)
			set(BAT_ERROR_REASON "${BAT_ERROR_REASON} BAT version ${BAT_VERSION} is lower than requested version ${BAT_FIND_VERSION}.")
		endif()
	endif()

endif()


# make variables changeable
mark_as_advanced(
	BAT_INCLUDE_DIR
	BAT_LIBRARIES
	)


# report result
if(BAT_FOUND)
	message(STATUS "Found BAT version ${BAT_VERSION} in '${BAT_ROOT_DIR}'.")
	message(STATUS "Using BAT include directory '${BAT_INCLUDE_DIR}'.")
	message(STATUS "Using BAT libraries '${BAT_LIBRARIES}'.")
	message(STATUS "Using extra LINKER_FLAGS for BAT '${BAT_LINKER_FLAGS}'.")
else()
	if(BAT_FIND_REQUIRED)
		message(FATAL_ERROR "Unable to find requested BAT installation:${BAT_ERROR_REASON}")
	else()
		if(NOT BAT_FIND_QUIETLY)
			message(STATUS "BAT version ${BAT_FIND_VERSION}+ was not found:${BAT_ERROR_REASON}")
		endif()
	endif()
endif()
