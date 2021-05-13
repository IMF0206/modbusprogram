#ifndef MODBUS_HELPER_H_INCLUDED_
#define MODBUS_HELPER_H_INCLUDED_

#include "modbus.h"
#include "errno.h"
#include "sqlite_helper.h"

#include "mbu-common.h"
#include <vector>

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
    Data16Array
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
    int modbus_init();
    int modbus_data_process();
    int modbus_start();

private:
    std::vector<Data> m_datavec;
    std::vector<WriteDataType> m_wdatetypevec;
    std::vector<BackendParams*> m_backendvec;
    int m_timeout_ms;
    std::vector<int> m_slaveAddrvec;
    std::vector<int> m_fTypevec;
    db_helper *m_dbhelper;
};

#endif