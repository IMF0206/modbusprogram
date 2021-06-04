# modbusprogram

./modbus_client --debug -mtcp -t0x03 -r0 -p1052 172.16.20.156 -c3
./modbus_client --debug -mtcp -t0x10 -r0 -p1052 172.16.20.156 0x01 0x02 0x03 0x04
./modbus_server -mtcp 172.16.20.156 -p1052