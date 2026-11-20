#include "ddcci_vcp_slave.h"
I2C_HandleTypeDef hi2c = {0};

static uint8_t i2c_xfer_buf[DDCCI_XFER_BUF_SIZE] = {0};
static size_t i2c_xfer_len = 0; 
void HAL_I2C_AddrCallback(I2C_HandleTypeDef* hi2c, uint8_t TransferDirection,
                          uint16_t AddrMatchCode) {
  if (hi2c->Instance != I2C1) return;

  if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
    i2c_xfer_len = 0;
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, i2c_xfer_buf, 1, I2C_FIRST_FRAME);
  } else {
    HAL_I2C_Slave_Seq_Transmit_IT(hi2c, i2c_xfer_buf, sizeof(i2c_xfer_buf),
                                  I2C_FIRST_AND_LAST_FRAME);
  }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance == I2C1) HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance != I2C1) return;
  i2c_xfer_len++;
  static uint8_t discard;
  uint8_t* rx_buf = i2c_xfer_len < sizeof(i2c_xfer_buf)
                        ? i2c_xfer_buf + i2c_xfer_len
                        : &discard;
  HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buf, 1, I2C_NEXT_FRAME);
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance != I2C1) return;
  if (i2c_xfer_len > sizeof(i2c_xfer_buf)) i2c_xfer_len = sizeof(i2c_xfer_buf);
  ddcci_vcp_slave_process_request(i2c_xfer_len);
  i2c_xfer_len = 0;
  HAL_I2C_EnableListen_IT(hi2c);
}

volatile uint16_t brightness = 0;

int32_t ddcci_vcp_slave_get(DDCCI_VCP_CODES vcp_code) {
  switch (vcp_code) {
    case VCP_CODE_BRIGHTNESS:
      return brightness;

    case VCP_CODE_DISPLAY_FIRMWARE_LEVEL:
      return 0x0001;  // VER 1

    case VCP_CODE_VCP_VERSION:
      return 0x0201;  // 2.1

    case VCP_CODE_DISPLAY_CONTROLLER_TYPE:
      return 0x00FF;  // vendor

    default:
      return DDCCI_VCP_INVALID_CODE;
  }
}

int32_t ddcci_vcp_slave_get_max(DDCCI_VCP_CODES vcp_code) {
  switch (vcp_code) {
    case VCP_CODE_BRIGHTNESS:
      return 0xFF;

    case VCP_CODE_DISPLAY_FIRMWARE_LEVEL:
    case VCP_CODE_VCP_VERSION:
    case VCP_CODE_DISPLAY_CONTROLLER_TYPE:
      return 0xFFFF;

    default:
      return DDCCI_VCP_INVALID_CODE;
  }
}

void ddcci_vcp_slave_set(DDCCI_VCP_CODES vcp_code, uint16_t value) {
  switch (vcp_code) {
    case VCP_CODE_BRIGHTNESS:
      brightness = value;
      break;
    default:
      break;
  }
}

void ddcci_vcp_slave_save() {}

const static char cap_str[] =
    "vcp(10)prot(monitor)type(LCD)cmds(01 02 03 "
    "E3 F3)mccs_ver(2.1)mpu_ver(V0.1)model(DDC/CI VCP Slave)";

const static ddcci_vcp_slave_config dvscfg = {
    .capabilities_str = cap_str,
    .capabilities_str_len = sizeof(cap_str),
    .xfer_buf = i2c_xfer_buf,
    .xfer_buf_len = sizeof(i2c_xfer_buf),
    .get = ddcci_vcp_slave_get,
    .set = ddcci_vcp_slave_set,
    .get_max = ddcci_vcp_slave_get_max,
    .save = ddcci_vcp_slave_save,
};

int main() {
  HAL_Init();
  MX_GPIO_Init();
  MX_I2C_Init();
  HAL_I2C_EnableListen_IT(&hi2c);
  ddcci_vcp_slave_init(&dvscfg);
  while (1) {
    printf("brightness = %d\n", brightness);
    HAL_Delay(1000);
  }
}