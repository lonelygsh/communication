### 项目简介
​	是一个用C++编写的嵌入式设备通信协议框架，旨在提供一个灵活且易于扩展的通信解决方案。它支持使用已实现的通信协议，同时也允许添加其他自定义的通信协议。



### 主要特性

- 灵活的通信协议框架
- 支持已实现的通信协议
- 可扩展性，可以添加其他自定义的通信协议
- 经过STM32与ROS2设备间通信测试，可用于其他设备的通信
- 可使用这个类在一个项目中定义不同的通讯接口



### 安装与使用

​	具体使用请参考examples。examples里面是一个STM32与ROS2设备进行串口通讯的例子。通信协议的具体内容可参考`/examples/readme.md`。

#### 添加自己的通讯协议并使用

1、添加自己发布数据的结构体与接受数据的结构体，确保自己有接收数据并存放buff中的函数。

```c++
/************接受数据相关***************/
//定义一个自己接受数据时需要进行的初始化，例如初始化串口等等
void Rxd_receive_init(void){}
//根据自己的通讯协议，定义一个数据校正的函数
uint8_t Rxd_data_correction(Data_Buffer receive_data){}
//实例化结构体时用自己定义的两个函数对GetData_Interface中的两个函数指针初始化，如下
GetData_Interface Rxd(&Rxd_receive_init, &Rxd_data_correction);

//确保自己有接收数据的函数，以STM32的串口中断为例
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        //接受一个字符放入上面定义的数据缓存区中
        Upper_Uart2_Receive(Rxd.data_temp);
        HAL_UART_Receive_IT(&huart2, &Rxd.data_temp, 1);
    }
}

/************发送数据相关***************/
//定义一个自己发送数据时需要进行的初始化，例如初始化串口等等
void Txd_send_init(){}
//定义发送数据的函数的函数，send_data的两个成员分别为发送数据的长度和数据的指针
void Txd_Send_data(Data_Buffer send_data){}
//定义自己发送数据时添加通讯协议的函数
void Txd_add_protocol(Data_Buffer send_data, Send_Data_Type SendDataType){}
SendData_Interface Txd(&Txd_send_init, &Txd_Send_data, &Txd_add_protocol);
```

2、根据自己的通讯协议添加和与发送不同数据的种类，例如

```c++
//接受到不同种类的数据
typedef enum _Event_Flag {
    no_event,       //没有接收到数据
    ServoControl_Sign,
    //....
} Event_Flag;

//发送什么类型的数据，这个可以在自己定义的通讯协议Txd_add_protocol中用到，也可以不用
typedef enum _Send_Data_Type {
	//....
} Send_Data_Type;
```

3、根据自己的通讯协议在对应的接受函数中添加内容

```c++
// 处理得到的数据，并返回标志位
Date_Handle Communication::data_handle() {
    //....
    switch (Get_DataEvent_Id()) {
        /********这里根据自己的通信协议写上面Event_Flag定义的不同的返回值***********/
        case ServoControl_Sign:
            Date_Handle_Temp.event = ServoControl_event;
            break;
            //.....
    }
    return Date_Handle_Temp;
}
```

4、在`main`函数中初始化后即可使用

```
Communication MyCommunication;
MyCommunication.Communication_init(&my_Rxd2, &my_Txd2);

Data_Buffer data_temp;
//给data_temp赋值，第二个参数根据自己定义的Send_Data_Type填写
MyCommunication.Send_protocol_data(data_temp, SendData_ServoControl);
```



### 注意

- 发送数据时，如果使用了dma发送，可以将`#define use_dma 0`的值改为1，此时不支持添加通讯协议， 发布数据时直接把需要发送数据数组的地址提交即可。
- 如果只需要用到接受发送功能，把实例`GetData_Interface`或者`SendData_Interface`的函数定义为空函数即可。



### 使用报告

​	如果你在用过程中发现任何问题或有改进建议，请将bug或者改进提交至490991455@qq.com。



