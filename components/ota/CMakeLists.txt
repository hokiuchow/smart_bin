set(COMPONENT_SRCS "ota.c" "ota_listener.c" )
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_REQUIRES main esp_http_client)
set(COMPONENT_EMBED_TXTFILES "../../cert/github.cer")

register_component()