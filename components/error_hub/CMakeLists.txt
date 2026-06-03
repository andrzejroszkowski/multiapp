idf_component_register(SRCS "error_hub.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES freertos)

# idf_component_register_test(SRCS "test/test_error_hub.cpp"
#                             INCLUDE_DIRS "include"
#                             REQUIRES freertos error_hub)
