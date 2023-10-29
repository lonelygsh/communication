//
// Created by gsh on 23-10-29.
//

#include "myserial.h"

serial::Serial uart3;

/*****************************************************************************接收数据相关************************************************************************/
void serial_rxd_init();
uint8_t serial_rxd_data_correction(Data_Buffer receive_data);

GetData_Interface serial3_rxd_data(&serial_rxd_init, &serial_rxd_data_correction);

void serial_rxd_init() {
}

uint8_t serial_rxd_data_correction(Data_Buffer receive_data) {
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

//结束数据函数，类似于STM32的串口中断函数
void ros2serial::readData() {
    while (rclcpp::ok()) {
        uart3.read(&serial3_rxd_data.data_temp, 1);
        switch (serial3_rxd_data.data_getting_flag) {
            case 0:
                if (serial3_rxd_data.data_temp == FRAME_HEAD) {
                    serial3_rxd_data.data_buffer[0] = FRAME_HEAD;
                    serial3_rxd_data.data_getting_flag = 1;
                }
                break;
            case 1:
                if (serial3_rxd_data.data_temp == FRAME_DEVICE_ID) {
                    serial3_rxd_data.data_buffer[1] = FRAME_DEVICE_ID;
                    serial3_rxd_data.data_getting_flag = 2;
                    serial3_rxd_data.data_index = 2;
                } else {
                    serial3_rxd_data.data_getting_flag = 0;
                    serial3_rxd_data.data_buffer[0] = 0x0;
                }
                break;
            case 2:
                serial3_rxd_data.data_length = serial3_rxd_data.data_temp + 2;
                if (serial3_rxd_data.data_length >= DATA_BUF_LEN) {
                    serial3_rxd_data.data_index = 0;
                    serial3_rxd_data.data_getting_flag = 0;
                    serial3_rxd_data.data_buffer[0] = 0;
                    serial3_rxd_data.data_buffer[1] = 0;
                    serial3_rxd_data.data_length = 0;
                    break;
                }
                serial3_rxd_data.data_buffer[serial3_rxd_data.data_index] = serial3_rxd_data.data_temp;
                serial3_rxd_data.data_index++;
                serial3_rxd_data.data_getting_flag = 3;
                break;
            case 3:
                if (serial3_rxd_data.data_index == 3) {
                    serial3_rxd_data.data_event_id = serial3_rxd_data.data_temp;
                }
                serial3_rxd_data.data_buffer[serial3_rxd_data.data_index] = serial3_rxd_data.data_temp;
                serial3_rxd_data.data_index++;
                if (serial3_rxd_data.data_index >= serial3_rxd_data.data_length) {
                    serial3_rxd_data.data_accept_flag = 1;
                    serial3_rxd_data.data_index = 0;
                    serial3_rxd_data.data_getting_flag = 0;
                }
                break;
            default:
                break;
        }
    }

    Date_Handle MyDataHandle = communication_interface.data_handle();

    if (MyDataHandle.event == no_event)
        return;
    switch (MyDataHandle.event) {
        case ServoControl_event:
            /* code */
            break;

        default:
            break;
    }
}

/*****************************************************************************发送数据相关************************************************************************/
#if use_dma == 1
void Txd2_send_init();
void Txd2_Send_data(Data_Buffer send_data);
SendData_Interface Txd2(&Txd2_send_init, &Txd2_Send_data);
// 发送初始化,dma初始化等
void Txd2_send_init()
{
}
// 添加自己dma发送数据的函数
void Txd2_Send_data(Data_Buffer send_data)
{
}

#elif use_dma == 0
void serial_txd_init();
void serial_txd_data(Data_Buffer send_data);
void serial_txd_add_protocol(Data_Buffer send_data, Send_Data_Type SendDataType);
SendData_Interface serial3_txd_data(&serial_txd_init, &serial_txd_data, &serial_txd_add_protocol);

// 发送初始化
void serial_txd_init() {
    // 加载参数
    std::string port_name = "ttyS3";

    // 控制器与扩展驱动板卡的串口配置与通信
    try {
        uart3.setPort("/dev/" + port_name);                             // 选择要开启的串口号
        uart3.setBaudrate(115200);                                      // 设置波特率
        serial::Timeout timeOut = serial::Timeout::simpleTimeout(2000); // 超时等待
        uart3.setTimeout(timeOut);
        uart3.open(); // 开启串口
    }
    catch (serial::IOException &e) {
        printf("serial can not open serial port,Please check the serial port cable!\r\n"); // 如果开启串口失败，打印错误信息
    }
}

// 添加自己发送数据的函数
void serial_txd_data(Data_Buffer send_data) {
    uart3.write(send_data.data, send_data.length);
}

// 添加自己的通讯协议的帧头和帧尾
void serial_txd_add_protocol(Data_Buffer send_data, Send_Data_Type SendDataType) {
    if (SendDataType == SendData_normal)
        return;

    serial3_txd_data.Head_End_DateBuffer[0] = FRAME_HEAD;
    serial3_txd_data.Head_End_DateBuffer[1] = FRAME_DEVICE_ID;
    serial3_txd_data.Head_End_DateBuffer[2] = send_data.length + 3;
    // 根据不同的数据添加不同的功能字
    switch (SendDataType) {
        case SendData_ServoControl:
            serial3_txd_data.Head_End_DateBuffer[3] = ServoControl_Sign;
            break;
        case SendData_FanControl:
            serial3_txd_data.Head_End_DateBuffer[3] = FanControl_Sign;
            break;
        case SendData_RGBled:
            serial3_txd_data.Head_End_DateBuffer[3] = RGBled_Sign;
            break;
        case SendData_Awaken:
            serial3_txd_data.Head_End_DateBuffer[3] = Awaken_Sign;
            break;
        default:
            break;
    }

    serial3_txd_data.Head_End_DateBuffer[4] =
            serial3_txd_data.Head_End_DateBuffer[2] + serial3_txd_data.Head_End_DateBuffer[3];
    for (uint16_t i = 0; i < send_data.length; i++) {
        serial3_txd_data.Head_End_DateBuffer[4] += *(send_data.data + i);
    }
    serial3_txd_data.data_head.length = 4;
    serial3_txd_data.data_head.data = serial3_txd_data.Head_End_DateBuffer;
    serial3_txd_data.data_end.length = 1;
    serial3_txd_data.data_end.data = serial3_txd_data.Head_End_DateBuffer + serial3_txd_data.data_head.length;
}
#endif

ros2serial::ros2serial(std::string nodeName) : Node(nodeName) {
    communication_interface.Communication_init(&serial3_rxd_data, &serial3_txd_data);
    // 如果串口打开，则开始一个新的读取数据的线程
    if (uart3.isOpen()) {
        RCLCPP_INFO(this->get_logger(), "serial serial port opened"); // 串口开启成功提示

        // 启动一个新线程读取并处理串口数据
        read_data_thread_ = std::shared_ptr<std::thread>(
                new std::thread(std::bind(&ros2serial::readData, this)));
    }

    speech_cmd = this->create_subscription<audio_msg::msg::SmartAudioData>(
            "audio_smart", 10, std::bind(&ros2serial::speech_callback, this, std::placeholders::_1));

}

ros2serial::~ros2serial() {
    uart3.close();
}

//发送数据
void ros2serial::speech_callback(const audio_msg::msg::SmartAudioData::SharedPtr ai_msg) {
    if (!ai_msg || !rclcpp::ok()) {
        return;
    }

    Data_Buffer data_temp;

    if (ai_msg->frame_type.value == 3) {
        if (ai_msg->cmd_word.find("开窗") != std::string::npos) {
            uint8_t temp = 90;
            data_temp.data = &temp;
            data_temp.length = 1;
            communication_interface.Send_protocol_data(data_temp, SendData_ServoControl);
        } else if (ai_msg->cmd_word.find("关窗") != std::string::npos) {
            uint8_t temp = 0;
            data_temp.data = &temp;
            data_temp.length = 1;
            communication_interface.Send_protocol_data(data_temp, SendData_ServoControl);
        }
        //后面的就不放了，根据自己的需要写
    }
}


int main(int argc, char **argv) {
    // 初始化ROS节点
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<ros2serial>("ros2_serial"));

    // 关闭ROS2接口，清除资源
    rclcpp::shutdown();

    return 0;
}
