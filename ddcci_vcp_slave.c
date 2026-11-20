/*
    Copyright (C) CNflysky. All rights reserved.
    License: GPL-V3
*/
#include "ddcci_vcp_slave.h"

static const ddcci_vcp_slave_config* config;

static void ddcci_encode_vcp_reply(uint8_t result, uint8_t type, uint8_t vcp,
                                     uint16_t vcp_max, uint16_t vcp_val) {
  uint8_t* xfer_buf = config->xfer_buf;
  ddcci_vcp_reply* reply = (ddcci_vcp_reply*)xfer_buf;
  reply->fixed_0x6e = 0x6e;
  reply->length = 0x88;
  reply->opcode = DDCCI_CMD_GET_VCP_FEATURE_REPLY;
  reply->result = result;
  reply->vcpcode = vcp;
  reply->type = type;
  reply->vcp_max_be = __builtin_bswap16(vcp_max);
  reply->vcp_cur_be = __builtin_bswap16(vcp_val);
  reply->checksum = 0x50;  // monitor address = 0x50
  for (uint8_t i = 0; i < 10; i++) xfer_buf[10] ^= xfer_buf[i];
}

static void ddcci_encode_cap_reply(uint16_t pos) {
  uint8_t* xfer_buf = config->xfer_buf;
  ddcci_cap_reply* reply = (ddcci_cap_reply*)xfer_buf;
  reply->fixed_0x6e = 0x6e;
  uint8_t length = 0;
  if (pos < config->capabilities_str_len) {
    size_t remain = config->capabilities_str_len - pos;
    length = remain > 32 ? 32 : (uint8_t)remain;
    memcpy(xfer_buf + 5, config->capabilities_str + pos, length);
  }
  reply->length = (length + 3) | 0x80;
  reply->cap_reply_0xe3 = DDCCI_CMD_CAPABILITIES_REPLY;
  reply->offset_be = __builtin_bswap16(pos);
  xfer_buf[length + 5] = 0x50;
  for (uint8_t i = 0; i < length + 5; i++) xfer_buf[length + 5] ^= xfer_buf[i];
}

inline static void ddcci_reply_vcp(uint8_t vcp, uint16_t max, uint16_t val) {
  ddcci_encode_vcp_reply(0, 0, vcp, max, val);
}

inline static void ddcci_reply_invalid_vcp(uint8_t vcp) {
  ddcci_encode_vcp_reply(1, 0, vcp, 0, 0);
}

void ddcci_vcp_slave_init(const ddcci_vcp_slave_config* cfg) {
  if (!cfg || !cfg->xfer_buf || cfg->xfer_buf_len < DDCCI_XFER_BUF_SIZE ||
      !cfg->capabilities_str || !cfg->capabilities_str_len || !cfg->get ||
      !cfg->get_max) {
    config = NULL;
    return;
  }
  config = cfg;
}

void ddcci_vcp_slave_process_request(size_t len) {
  if (!config) return;
  if (!len || len > config->xfer_buf_len) return;
  if (len < sizeof(ddcci_msg_header)) return;

  uint8_t* xfer_buf = config->xfer_buf;
  // verify checksum
  uint8_t cksum = 0x6e;
  for (uint8_t i = 0; i < len - 1; i++) cksum ^= xfer_buf[i];
  if (cksum != xfer_buf[len - 1]) return;

  ddcci_msg_header* req = (ddcci_msg_header*)xfer_buf;
  // uint8_t payload_len = req->length & 0x7f;
  switch (req->opcode) {
    case DDCCI_CMD_GET_VCP_FEATURE: {
      if (len < 5) return;
      int32_t cur = config->get(req->vcpcode);
      int32_t max = config->get_max(req->vcpcode);
      if (cur < 0 || max < 0)
        ddcci_reply_invalid_vcp(req->vcpcode);
      ddcci_reply_vcp(req->vcpcode, max & 0xFFFF, cur & 0xFFFF);
      break;
    }

    case DDCCI_CMD_SET_VCP_FEATURE:
      if (len < 7) return;
      if (config->set)
        config->set(req->vcpcode, xfer_buf[4] << 8 | xfer_buf[5]);
      break;

    case DDCCI_CMD_SAVE_CURRENT_SETTINGS:
      if (config->save) config->save();
      break;

    case DDCCI_CMD_CAPABILITIES_REQUEST: 
      if (len < 6) break;
      ddcci_encode_cap_reply(xfer_buf[3] << 8 | xfer_buf[4]);

    default:
      break;
  }
}
