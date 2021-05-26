#include "modbus_handler.h"
#include "sqlite_helper.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    modbus_handler *modbushandler = new modbus_handler();
    modbushandler->modbus_handler_init();
    printf("start\n");
    modbushandler->modbus_start();
}