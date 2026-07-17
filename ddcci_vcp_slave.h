/*
    Copyright (C) CNflysky. All rights reserved.
    License: GPL-V3
*/

#ifndef _DDCCI_VCP_SLAVE_H_
#define _DDCCI_VCP_SLAVE_H_
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define DDCCI_I2C_ADDRESS 0x37

#define DDCCI_OPCODE_GET 0x01
#define DDCCI_OPCODE_REPLY 0x02
#define DDCCI_OPCODE_SET 0x03

#define DDCCI_OPCODE_SAVE 0x0C
#define DDCCI_OPCODE_CAP 0xF3  // capabilities request

#define VCP_CODE_BRIGHTNESS 0x10
#define VCP_CODE_VERSION 0xDF
#define VCP_CODE_DISP_CTRLER_TYPE 0xC8
#define VCP_CODE_FWVER 0xC9

#ifndef DDCCI_XFER_BUF_SIZE
#define DDCCI_XFER_BUF_SIZE 40
#endif

#ifndef DDCCI_CAP_STR
#define DDCCI_CAP_STR                           \
  "vcp(10)prot(monitor)type(LCD)cmds(01 02 03 " \
  "07)mccs_ver(2.1)mpu_ver(V0.1)model(DDC/CI VCP Slave)"
#endif

typedef struct {
  uint8_t fixed_0x51;
  uint8_t length;
  uint8_t opcode;
  uint8_t vcpcode;
} ddcci_msg_header;

typedef struct __packed {
  uint8_t fixed_0x6e;
  uint8_t length;
  uint8_t opcode;
  uint8_t result;
  uint8_t vcpcode;
  uint8_t type;
  uint16_t vcp_max_be;
  uint16_t vcp_cur_be;
  uint8_t checksum;
} ddcci_vcp_reply;

typedef struct __packed {
  uint8_t fixed_0x6e;
  uint8_t length;
  uint8_t cap_reply_0xe3;
  uint16_t offset_be;
  uint8_t data[32];
  // uint8_t cksum;
} ddcci_cap_reply;

void ddcci_process_request();
void ddcci_write_byte(uint8_t byte);
uint8_t* ddcci_get_xfer_buf();
uint8_t* ddcci_get_xfer_len();
uint16_t ddcci_get_brightness();
void ddcci_set_brightness(uint16_t val);

#endif
