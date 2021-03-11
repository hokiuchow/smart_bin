set(COMPONENT_SRCS "camera.c" "motion.c" )
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_REQUIRES main esp32-camera)

register_component()