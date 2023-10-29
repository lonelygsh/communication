#ifndef COMMINUCATION_H
#define COMMINUCATION_H

#include "communication_denfintion.h"

class Communication {
public:
    Communication();
    void Communication_init(GetData_Interface *Get_data, SendData_Interface *Send_data);
    Date_Handle data_handle();
    void Send_protocol_data(Data_Buffer send_data, Send_Data_Type SendDataType);
private:
    GetData_Interface *GetDataInterface;
    SendData_Interface *SendDataInterface;
    uint8_t *Get_Data();
    uint8_t Get_Data_Length();
    uint8_t Get_DataAccept_Flag();
    uint8_t Get_DataEvent_Id();
    void Clear_Data();
};

#endif //COMMUNICATION_COMMINUCATION_H
