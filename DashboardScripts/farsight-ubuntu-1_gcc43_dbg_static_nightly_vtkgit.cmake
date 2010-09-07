SET(CTEST_SOURCE_NAME farsight-trunk-nightly)
SET(CTEST_BINARY_NAME farsight-rel-static-vtkgit-nightly)
SET(CTEST_DASHBOARD_ROOT "/Dashboards")
SET(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_SOURCE_NAME}")
SET(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_BINARY_NAME}")

SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

SET(CTEST_COMMAND
  "/usr/local/bin/ctest -V -VV -D Experimental -A ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}"
  )

SET(CTEST_CMAKE_COMMAND
  "/usr/local/bin/cmake"
  )

SET(CTEST_INITIAL_CACHE "
SITE:STRING=farsight-ubuntu-1
BUILDNAME:STRING=gcc43-x86_64-vtkgit-debug
CMAKE_GENERATOR:INTERNAL=Unix Makefiles
MAKECOMMAND:STRING=/usr/bin/make -j9 -i
CMAKE_BUILD_TYPE:STRING=Debug
BUILD_SHARED_LIBS:BOOL=OFF
ITK_DIR:PATH=/Dashboards/ITK-3.16-static
VTK_DIR:PATH=/Dashboards/VTK-git-static
Boost_INCLUDE_DIR:PATH=/Dashboards/boost_1_41_0
FARSIGHT_DATA_ROOT:PATH=/Dashboards/farsight-data
")

SET(CTEST_CVS_COMMAND "/usr/bin/svn")
SET(CTEST_EXTRA_UPDATES_1 "/Dashboards/farsight-data" "--non-interactive")
