#ifndef CMD_H
#define CMD_H
#include <cstdint>

namespace cmd {

const uint8_t T_POSITION = 0x01;
const uint8_t R_POSITION = 0x02;
const uint8_t T_PRESSURE = 0x03;
const uint8_t PRESSURE_1 = 0x04;
const uint8_t PRESSURE_2 = 0x05;

//void send_cmd(uint16_t id, uint16_t cmd, uint16_t* data, uint16_t size);
//void send_ack();
//void send_nck();

const uint16_t CMD_CONNECT = 0x0101;   // 通信连接
const uint16_t CMD_INIT = 0x0102;      // 初始化
const uint16_t CMD_DI_FETCH = 0x0103;  // DI查询
const uint16_t CMD_VERSION = 0x0104;   // 版本查询
const uint16_t CMD_RESET = 0x0105;     // 复位
const uint16_t CMD_ERROR = 0x0106;     // 错误重置

const uint16_t CMD_DO_SET = 0x0201;     // DO设置
const uint16_t CMD_DI_CHANGE = 0x0301;  // DI变化

}  // namespace cmd

#endif // CMD_H
