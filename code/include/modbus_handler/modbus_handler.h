#ifndef MODBUS_HELPER_H_INCLUDED_
#define MODBUS_HELPER_H_INCLUDED_

#include "modbus.h"
#include "errno.h"
#include "sqlite_helper.h"
#include "modbus2mqtt.h"

#include "mbu-common.h"
#include <vector>
#include <map>

class mqtt_pub;
class mqtt_plat;

typedef enum {
    FuncNone =          -1,

    ReadCoils           = 0x01,
    ReadDiscreteInput   = 0x02,
    ReadHoldingRegisters= 0x03,
    ReadInputRegisters  = 0x04,
    WriteSingleCoil     = 0x05,
    WriteSingleRegister = 0x06,
    WriteMultipleCoils  = 0x0f,
    WriteMultipleRegisters  = 0x10
} FuncType;

typedef enum {
    DataInt,
    Data8Array,
    Data16Array,
    DataError
} WriteDataType;

union Data {
    int dataInt;
    uint8_t *data8;
    uint16_t *data16;
};

class modbus_handler
{
public:
    modbus_handler();
    ~modbus_handler();
    int modbus_handler_init();
    int modbus_handler_connect();
    int modbus_data_process();
    int modbus_start();

private:
    int modbus_read_holdingdata(modbus_t* ctx, int nodeid, int startaddr, uint16_t *dest);
    std::vector<Data> m_datavec;
    std::vector<WriteDataType> m_wdatetypevec;
    std::vector<BackendParams*> m_backendvec;
    std::vector<modbus_t*> m_mb_vec;
    int m_timeout_ms;
    std::vector<int> m_slaveAddrvec;
    std::vector<FuncType> m_fTypevec;
    db_helper *m_dbhelper;
    std::vector<bool> m_bConnected;
    
    mqtt_pub *m_mqttpub;
    mqtt_plat *m_mqttplat;
    std::map<std::string, float> m_data_map;
    std::map<int, std::string> m_nodeid_portaddr;
    // 校验位，用于check接入设备是否有变更
    int m_checknum;
    modbus2mqtt m_mdb2mqtt;
    // TODO!!!!!!!!!
    // 类型映射，1表示modbus,2表示lora,暂定
    std::string m_protocol_type;
};

#endif