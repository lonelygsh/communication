例子的通讯协议为：

数据包头为:`0xFF,0xFC`

最后一位为校验位，没有结束位，结束条件为长度的值。

校验位的计算方式：从长度的值（下标为2）累加到校验位前的值，再对256取余。

长度的计算方式：除了包头1和包头2的所有字节之和，快捷计算方式：校验位的下标减去1。

| 包头1 | 包头2 | 长度 | 功能字 | 数据 | 校验位 |
| :---: | :---: | :--: | :----: | :--: | :----: |
| 0xFF  | 0xFC  |  xx  |   xx   |  xx  |   xx   |



本ROS2实例中需要提前准备好ROS2设备的串口库

```
git clone https://github.com/ZhaoXiangBox/serial.git
cd serial
mkdir build
cd build
cmake ..
make
```

