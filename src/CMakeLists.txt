add_definitions(-DTRANSLATION_DOMAIN=\"docker-plugin\")

set(docker-plugin_SRCS
    docker-pluginplugin.cpp
    docker-pluginview.cpp
)

add_library(docker-plugin MODULE ${docker-plugin_SRCS})

target_link_libraries(docker-plugin
    KF6::TextEditor
    KF6::I18n
)

install(TARGETS docker-plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor)
