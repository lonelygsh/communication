#include "communication.h"
#include "usart.h"


void Upper_Uart2_Receive(uint8_t Rx_Temp);

extern "C" {
//STM32串口中断，接受一个字符放入数据缓存区
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        Upper_Uart2_Receive(Rxd2.data_temp);
        HAL_UART_Receive_IT(&huart2, &Rxd2.data_temp, 1);
    }
}
}

/*****************************************************************************接收数据相关************************************************************************/
void Rxd2_receive_init(void);
uint8_t Rxd2_data_correction(Data_Buffer receive_data);
GetData_Interface Rxd2(&Rxd2_receive_init, &Rxd2_data_correction);


//这里写自己通讯的初始化函数
void Rxd2_receive_init(void) {
    HAL_UART_Receive_IT(&huart2, &Rxd2.data_temp, 1);
}

//对中断中接收到的数据进行处理，根据自己通讯协议的帧头帧尾判断得到一帧的数据，并存放到数据缓存区数组buffer中
void Upper_Uart2_Receive(uint8_t Rx_Temp) {
    switch (Rxd2.data_getting_flag) {
        case 0:
            if (Rx_Temp == FRAME_HEAD) {
                Rxd2.data_buffer[0] = FRAME_HEAD;
                Rxd2.data_getting_flag = 1;
            }
            break;
        case 1:
            if (Rx_Temp == FRAME_DEVICE_ID) {
                Rxd2.data_buffer[1] = FRAME_DEVICE_ID;
                Rxd2.data_getting_flag = 2;
                Rxd2.data_index = 2;
            } else {
                Rxd2.data_getting_flag = 0;
                Rxd2.data_buffer[0] = 0x0;
            }
            break;
        case 2:
            Rxd2.data_length = Rx_Temp + 2;
            if (Rxd2.data_length >= DATA_BUF_LEN) {
                Rxd2.data_index = 0;
                Rxd2.data_getting_flag = 0;
                Rxd2.data_buffer[0] = 0;
                Rxd2.data_buffer[1] = 0;
                Rxd2.data_length = 0;
                break;
            }
            Rxd2.data_buffer[Rxd2.data_index] = Rx_Temp;
            Rxd2.data_index++;
            Rxd2.data_getting_flag = 3;
            break;
        case 3:
            if (Rxd2.data_index == 3) {
                Rxd2.data_event_id = Rx_Temp;
            }
            Rxd2.data_buffer[Rxd2.data_index] = Rx_Temp;
            Rxd2.data_index++;
            if (Rxd2.data_index >= Rxd2.data_length) {
                Rxd2.data_accept_flag = 1;
                Rxd2.data_index = 0;
                Rxd2.data_getting_flag = 0;
            }
            break;
        default:
            break;
    }
}

//数据校验函数，根据自己通讯协议的校验位对数据进行校验，校验成功返回1，校验失败返回0
//输入：data_buf为数据内容，num为数据长度
//输出：是否校验成功，1为成功，0为失败
uint8_t Rxd2_data_correction(Data_Buffer receive_data) {
    int sum = 0;
    for (uint16_t i = 2; i < (receive_data.length - 1); i++)
        sum += *(receive_data.data + i);
    sum = sum & 0xFF;
    uint8_t recvSum = *(receive_data.data + receive_data.length - 1);
    if (!(sum == recvSum)) {
        return 0;
    }
    return 1;
}

/*****************************************************************************发送数据相关************************************************************************/
#if use_dma == 1
void Txd2_send_init();
void Txd2_Send_data(Data_Buffer send_data);
SendData_Interface Txd2(&Txd2_send_init, &Txd2_Send_data);
//发送初始化,dma初始化等
void Txd2_send_init() {
}
//添加自己dma发送数据的函数
void Txd2_Send_data(Data_Buffer send_data) {
}

#elif use_dma == 0
void Txd2_send_init();
void Txd2_Send_data(Data_Buffer send_data);
void Txd2_add_protocol(Data_Buffer send_data, Send_Data_Type SendDataType);
SendData_Interface Txd2(&Txd2_send_init, &Txd2_Send_data, &Txd2_add_protocol);

//发送初始化
void Txd2_send_init() {

}

//添加自己发送数据的函数,send_data中包含数据的长度和数据内容的指针
void Txd2_Send_data(Data_Buffer send_data) {
    HAL_UART_Transmit(&huart2, send_data.data, send_data.length, 100);
}

//根据自己的通讯协议添加帧头和帧尾
void Txd2_add_protocol(Data_Buffer send_data, Send_Data_Type SendDataType) {
    if (SendDataType == SendData_normal)
        return;

    Txd2.Head_End_DateBuffer[0] = FRAME_HEAD;
    Txd2.Head_End_DateBuffer[1] = FRAME_DEVICE_ID;
    Txd2.Head_End_DateBuffer[2] = send_data.length + 3;
    //根据不同的数据添加不同的功能字
    switch (SendDataType) {
        case SendData_ServoControl:
            Txd2.Head_End_DateBuffer[3] = ServoControl_Sign;
            break;
    }

    Txd2.Head_End_DateBuffer[4] = Txd2.Head_End_DateBuffer[2] + Txd2.Head_End_DateBuffer[3];
    for (uint16_t i = 0; i < send_data.length; i++) {
        Txd2.Head_End_DateBuffer[4] += *(send_data.data + i);
    }
    Txd2.data_head.length = 4;
    Txd2.data_head.data = Txd2.Head_End_DateBuffer;
    Txd2.data_end.length = 1;
    Txd2.data_end.data = Txd2.Head_End_DateBuffer + Txd2.data_head.length;
}
#endif


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
        case ServoControl_Sign:
            Date_Handle_Temp.event = ServoControl_event;
            break;
        case FanControl_Sign:
            Date_Handle_Temp.event = FanControl_event;
            break;
        case RGBled_Sign:
            Date_Handle_Temp.event = RGBControl_event;
            break;
        case Awaken_Sign:
            Date_Handle_Temp.event = Awaken_event;
            break;
        default:
            break;
    }
    return Date_Handle_Temp;
}

void Communication::Send_protocol_data(Data_Buffer send_data, Send_Data_Type SendDataType) {
#if use_dma == 0
    if (SendDataType != SendData_normal) {
        SendDataInterface->Add_protocol(send_data, SendDataType);
        SendDataInterface->Send_data(SendDataInterface->data_head);
    }
#endif
    SendDataInterface->Send_data(send_data);
#if use_dma == 0
    if (SendDataType != SendData_normal) {
        SendDataInterface->Send_data(SendDataInterface->data_end);
    }
#endif
}