//这里仅放使用的代码，使用之前记得在main函数中初始化
#include "communication.h"

Communication MyCommunication;

void event_handle() {
    Date_Handle getdata = MyCommunication.data_handle();
    switch (getdata.event) {
        case ServoControl_event: {  //按需求添加
            uint8_t angle = *(getdata.event_data + 4);
        }
            break;
        case FanControl_event: {
            uint8_t rotate_speed = *(getdata.event_data + 4);
        }
            break;
        case RGBControl_event: {
        }
            break;
        case Awaken_event: {
        }
            break;
        default:
            break;
    }
//    My_device.device_Led.set_LedMode(flow);
}
