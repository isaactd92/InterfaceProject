# Install script for directory: /home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/swig

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7" TYPE MODULE FILES "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/swig/_InterfaceJRO_7_swig.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so"
         OLD_RPATH "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/lib:/usr/local/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7/_InterfaceJRO_7_swig.so")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7" TYPE FILE FILES "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/swig/InterfaceJRO_7_swig.py")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages/InterfaceJRO_7" TYPE FILE FILES
    "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/swig/InterfaceJRO_7_swig.pyc"
    "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/swig/InterfaceJRO_7_swig.pyo"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/InterfaceJRO_7/InterfaceJRO_7/swig" TYPE FILE FILES
    "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/swig/InterfaceJRO_7_swig.i"
    "/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/swig/InterfaceJRO_7_swig_doc.i"
    )
endif()

