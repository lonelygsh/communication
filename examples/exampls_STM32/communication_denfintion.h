#include "cstdint"

#define use_dma                     0

#define DATA_BUF_LEN                (20)
#define DATA_BUF_HEADEND_LEN        (5)

#define FRAME_HEAD                  (0xFF)
#define FRAME_DEVICE_ID             (0xFC)

#define ServoControl_Sign           0x01
#define FanControl_Sign             0x02
#define RGBled_Sign                 0x03
#define Awaken_Sign                 0x04


//接收到了怎么样的数据
typedef enum _Event_Flag {
    no_event,       //没有接收到数据
    ServoControl_event,
    FanControl_event,
    RGBControl_event,
    Awaken_event
} Event_Flag;

//发送什么类型的数据
typedef enum _Send_Data_Type {
    SendData_normal,
    SendData_ServoControl,
} Send_Data_Type;

typedef struct {
    Event_Flag event;
    uint8_t *event_data;
} Date_Handle;

typedef struct {
    uint16_t length;
    uint8_t *data;
} Data_Buffer;

struct GetData_Interface {
    void (*Get_data_init)();                                    //接受数据初始化函数
    uint8_t (*Data_correction)(Data_Buffer receive_data);       //数据校正函数
    //构造函数
    GetData_Interface(void (*func1)(), uint8_t (*func2)(Data_Buffer receive_data)) : Get_data_init(func1),
                                                                                     Data_correction(*func2) {}
    uint8_t data_temp;                      //接收缓存
    uint8_t data_getting_flag;              //接收数据时的状态
    uint8_t data_buffer[DATA_BUF_LEN];      //接收数据的缓存区
    uint8_t data_index;                     //接收数据的下标
    uint8_t data_length;                    //接收数据的长度
    uint8_t data_accept_flag;               //是否接收到数据的标志位
    uint8_t data_event_id;                  //功能字
};

#if use_dma == 0

struct SendData_Interface {
    void (*Send_data_init)();                                                       //发送数据初始化函数
    void (*Send_data)(Data_Buffer send_data);                                       //发送数据函数
    void (*Add_protocol)(Data_Buffer send_data, Send_Data_Type SendDataType);       //加入自己的通讯协议函数
    //数据协议存放的帧头和帧尾
    Data_Buffer data_head;
    Data_Buffer data_end;
    uint8_t Head_End_DateBuffer[DATA_BUF_HEADEND_LEN];
    SendData_Interface(void (*func1)(), void (*func2)(Data_Buffer send_data),
                       void (*func3)(Data_Buffer send_data, Send_Data_Type SendDataType))
            : Send_data_init(func1), Send_data(func2), Add_protocol(func3) {}
};

#elif use_dma == 1
struct SendData_Interface {
    void (*Send_data_init)();                               //初始化函数
    void (*Send_data)(Data_Buffer send_data);                         //发送数据
    SendData_Interface(void (*func1)(), void (*func2)(Data_Buffer send_data))
            : Send_data_init(func1), Send_data(func2) {}
};
#endif
