idf_component_register(SRCS "telemetry_app.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES app_interface msg_bus ui_engine hw_locks error_hub freertos lvgl
                      )