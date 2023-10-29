#include "communication.h"

Communication::Communication() {

}

void Communication::Communication_init(GetData_Interface *Get_data, SendData_Interface *Send_data) {
    GetDataInterface = Get_data;
    GetDataInterface->Get_data_init();
    SendDataInterface = Send_data;
    SendDataInterface->Send_data_init();
}

// 获取接收的数据
uint8_t *Communication::Get_Data(void) {
    return (uint8_t *) GetDataInterface->data_buffer;
}

// 获取命令长度
uint8_t Communication::Get_Data_Length(void) {
    return GetDataInterface->data_length;
}

// 获取命令标志
uint8_t Communication::Get_DataAccept_Flag(void) {
    return GetDataInterface->data_accept_flag;
}

// 获取事件的标志
uint8_t Communication::Get_DataEvent_Id(void) {
    return GetDataInterface->data_event_id;
}

// 清除命令数据和相关标志
void Communication::Clear_Data(void) {
    for (uint8_t i = 0; i < GetDataInterface->data_length; i++) {
        GetDataInterface->data_buffer[i] = 0;
    }
    GetDataInterface->data_length = 0;
    GetDataInterface->data_accept_flag = 0;
    GetDataInterface->data_event_id = 0;
}

// 处理得到的数据，并返回标志位
Date_Handle Communication::data_handle() {
    Date_Handle Date_Handle_Temp = {no_event, nullptr};
    //没有接收到数据就退出
    if (Get_DataAccept_Flag()) {
        //如果数据校正失败也退出
        Data_Buffer data_temp = {0, nullptr};
        data_temp.data = Get_Data();
        data_temp.length = Get_Data_Length();
        if (!(GetDataInterface->Data_correction(data_temp)))
            return Date_Handle_Temp;
    } else {
        return Date_Handle_Temp;
    }

    Date_Handle_Temp.event_data = GetDataInterface->data_buffer;

    switch (Get_DataEvent_Id()) {
        /******************这里根据自己的通信协议写不同的返回值********************/

    }
    return Date_Handle_Temp;
}

void Communication::Send_protocol_data(Data_Buffer send_data, Send_Data_Type SendDataType) {
#if use_dma == 0
    if (SendDataType != SendData_normal){
        SendDataInterface->Add_protocol(send_data, SendDataType);
        SendDataInterface->Send_data(SendDataInterface->data_head);
    }
#endif
    SendDataInterface->Send_data(send_data);
#if use_dma == 0
    if (SendDataType != SendData_normal){
        SendDataInterface->Send_data(SendDataInterface->data_end);
    }
#endif
}