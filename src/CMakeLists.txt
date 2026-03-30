add_definitions(-DTRANSLATION_DOMAIN=\"docker-plugin\")

set(docker-plugin_SRCS
    docker-plugin.cpp
    docker-pluginview.cpp
    dockerclient.cpp
)

add_library(docker-plugin MODULE ${docker-plugin_SRCS})

target_link_libraries(docker-plugin
    KF6::TextEditor
    KF6::I18n
    Qt6::Widgets
)

install(TARGETS docker-plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor)
