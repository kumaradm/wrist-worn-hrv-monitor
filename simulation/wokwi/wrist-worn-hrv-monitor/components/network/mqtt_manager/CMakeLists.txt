idf_component_register(
    SRCS "mqtt_manager.c"
    INCLUDE_DIRS "include"
    EMBED_TXTFILES "certs/aws_root_ca.pem" "certs/wrist_worn_hrv_monitor.cert.pem" "certs/wrist_worn_hrv_monitor.private.key"
    REQUIRES mqtt
)