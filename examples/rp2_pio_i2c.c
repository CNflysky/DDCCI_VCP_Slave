#include "ddcci_vcp_slave.h"
// add this library to your project:
// https://github.com/dgatf/I2C-slave-multi-address-RP2040
#include "i2c_multi.h"

static PIO pio = pio0;
uint8_t pio_i2c_sda = 18;

static uint8_t i2c_xfer_buf[DDCCI_XFER_BUF_SIZE] = {0};
static size_t i2c_xfer_len = 0;

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

void i2c_receive_handler(uint8_t data, bool is_address) {
  if (!is_address) i2c_xfer_buf[i2c_xfer_len++] = data;
}

void i2c_stop_handler(uint8_t length) {
  ddcci_process_request();
  i2c_xfer_len = 0;
}

int main() {
  stdio_init_all();
  i2c_multi_init(pio, pio_i2c_sda);
  i2c_multi_enable_address(DDCCI_I2C_ADDRESS);
  i2c_multi_set_receive_handler(i2c_receive_handler);
  i2c_multi_set_stop_handler(i2c_stop_handler);
  i2c_multi_set_write_buffer(i2c_xfer_buf);

  ddcci_vcp_slave_init(&dvscfg);

  while (1) {
    printf("brightness = %d\n", ddcci_get_brightness());
    sleep_ms(1000);
  }
}
