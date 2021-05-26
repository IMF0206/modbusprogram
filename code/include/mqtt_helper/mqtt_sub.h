#ifndef MQTT_SUB_H_INCLUDED_
#define MQTT_SUB_H_INCLUDED_

// #ifdef __cplusplus
// extern "C" {
// #endif

#include <stdio.h>

class mqtt_sub
{
public:
    mqtt_sub()
    {}
    virtual ~mqtt_sub(){}
    int mqtt_init();
};


// #ifdef __cplusplus
// }
// #endif

#endif /* MQTT_SUB_H_INCLUDED_ */