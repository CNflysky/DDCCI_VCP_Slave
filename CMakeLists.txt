add_library(ddcci_vcp_slave
    ${CMAKE_CURRENT_LIST_DIR}/ddcci_vcp_slave.c
)

target_include_directories(ddcci_vcp_slave PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
)

target_compile_definitions(ddcci_vcp_slave PUBLIC
    DDCCI_XFER_BUF_SIZE=40
)