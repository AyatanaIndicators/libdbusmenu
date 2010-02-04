find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUSMENUGLIB REQUIRED dbusmenu-glib)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(JSONGLIB REQUIRED json-glib-1.0)

set(glibapp_SRCS
    main.c
    )

include_directories(
    ${DBUSMENUGLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${JSONGLIB_INCLUDE_DIRS}
    )

link_directories(
    ${DBUSMENUGLIB_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
    ${JSONGLIB_LIBRARY_DIRS}
    )

add_executable(dbusmenubench-glibapp ${glibapp_SRCS})

target_link_libraries(dbusmenubench-glibapp
    ${DBUSMENUGLIB_LIBRARIES}
    ${GLIB_LIBARIES}
    ${JSONGLIB_LIBRARIES}
    )
