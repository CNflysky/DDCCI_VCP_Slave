/*
    Copyright (C) CNflysky. All rights reserved.
    License: GPL-V3
*/
#include "ddcci_vcp_slave.h"

static uint8_t xfer_buf[DDCCI_XFER_BUF_SIZE] = {0};
static uint8_t xfer_len = 0;
static uint16_t brightness = 127;
const static uint8_t capabilities_str[] = DDCCI_CAP_STR;

uint8_t* ddcci_get_xfer_buf() { return xfer_buf; }
uint8_t* ddcci_get_xfer_len() { return &xfer_len; }
uint16_t ddcci_get_brightness() { return brightness; }
void ddcci_set_brightness(uint16_t val) { brightness = val; }
void ddcci_write_byte(uint8_t byte) {
  if (xfer_len < sizeof(xfer_buf)) xfer_buf[xfer_len] = byte;
  xfer_len++;
}

static void ddcci_encode_vcp_reply(uint8_t result, uint8_t type, uint8_t vcp,
                                   uint16_t vcp_max, uint16_t vcp_val) {
  ddcci_vcp_reply* reply = (ddcci_vcp_reply*)xfer_buf;
  reply->fixed_0x6e = 0x6e;
  reply->length = 0x88;
  reply->opcode = DDCCI_OPCODE_REPLY;
  reply->result = result;
  reply->vcpcode = vcp;
  reply->type = type;
  reply->vcp_max_be = __builtin_bswap16(vcp_max);
  reply->vcp_cur_be = __builtin_bswap16(vcp_val);
  reply->checksum = 0x50;  // monitor address = 0x50
  for (uint8_t i = 0; i < 10; i++) xfer_buf[10] ^= xfer_buf[i];
}

static void ddcci_encode_cap_reply(uint16_t pos) {
  ddcci_cap_reply* reply = (ddcci_cap_reply*)xfer_buf;
  reply->fixed_0x6e = 0x6e;
  uint8_t length =
      (sizeof(capabilities_str) - pos > 32 ? 32
                                           : sizeof(capabilities_str) - pos);
  reply->length = (length + 3) | 0x80;
  reply->cap_reply_0xe3 = 0xe3;
  reply->offset_be = __builtin_bswap16(pos);
  memcpy(xfer_buf + 5, capabilities_str + pos, length);
  xfer_buf[length + 5] = 0x50;
  for (uint8_t i = 0; i < length + 5; i++) xfer_buf[length + 5] ^= xfer_buf[i];
}

inline static void ddcci_reply_vcp(uint8_t vcp, uint16_t max, uint16_t val) {
  ddcci_encode_vcp_reply(0, 0, vcp, max, val);
}

inline static void ddcci_reply_invalid_vcp(uint8_t vcp) {
  ddcci_encode_vcp_reply(1, 0, vcp, 0, 0);
}

void ddcci_process_request() {
  if (!xfer_len || xfer_len > sizeof(xfer_buf)) return;
  // verify checksum
  uint8_t cksum = 0x6e;
  for (uint8_t i = 0; i < xfer_len - 1; i++) cksum ^= xfer_buf[i];
  if (cksum != xfer_buf[xfer_len - 1]) return;

  ddcci_msg_header* req = (ddcci_msg_header*)xfer_buf;
  // uint8_t payload_len = req->length & 0x7f;
  switch (req->opcode) {
    case DDCCI_OPCODE_GET:
      switch (req->vcpcode) {
        case VCP_CODE_BRIGHTNESS:
          ddcci_reply_vcp(VCP_CODE_BRIGHTNESS, 0xFF, brightness);
          break;
        case VCP_CODE_VERSION:
          ddcci_reply_vcp(VCP_CODE_VERSION, 0xFFFF, 0x0201);  // 2.1
          break;
        case VCP_CODE_DISP_CTRLER_TYPE:
          ddcci_reply_vcp(VCP_CODE_DISP_CTRLER_TYPE, 0xFFFF,
                          0x00FF);  // vendor ids
          break;
        case VCP_CODE_FWVER:
          ddcci_reply_vcp(VCP_CODE_FWVER, 0xFFFF,
                          0x0001);  // version 1.0
          break;
        default:
          ddcci_reply_invalid_vcp(req->vcpcode);
          break;
      };
      break;

    case DDCCI_OPCODE_SET:
      switch (req->vcpcode) {
        case VCP_CODE_BRIGHTNESS:
          brightness = __builtin_bswap16(*(uint16_t*)&xfer_buf[4]);
          break;
      };
      break;

    case DDCCI_OPCODE_SAVE:
      break;

    case DDCCI_OPCODE_CAP:
      ddcci_encode_cap_reply(xfer_buf[3] << 8 | xfer_buf[4]);
      break;

    default:
      break;
  }
  xfer_len = 0;
}
