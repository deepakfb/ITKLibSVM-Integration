cmake_minimum_required (VERSION 2.6)
project(ClassifyPixels)

set(SOURCEFILES
  ImageReaderWriter.cxx
  ClassifyPixels.cxx
)

include_directories ("${PROJECT_SOURCE_DIR}/libSVM")
add_subdirectory (libSVM)

add_executable(ClassifyPixels ${SOURCEFILES})
target_link_libraries (ClassifyPixels libSVM)

FIND_PACKAGE ( ITK )
IF ( ITK_FOUND )
         INCLUDE( ${ITK_USE_FILE} )
ENDIF( ITK_FOUND )


TARGET_LINK_LIBRARIES ( ClassifyPixels ${ITK_LIBRARIES})
