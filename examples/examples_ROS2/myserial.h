//
// Created by gsh on 23-10-29.
//
#include <rclcpp/rclcpp.hpp>
#include "serial/serial.h"
#include "std_msgs/msg/string.hpp"
#include "communication.h"
#include "audio_msg/msg/smart_audio_data.hpp"

#define ledEvent_brightless     0x01
#define ledEvent_color_static   0x02
#define ledEvent_rainbow        0x03
#define ledEvent_fading         0x04
#define ledEvent_flow           0x05
#define ledColor_RED            0x01
#define ledColor_GREEN          0x02
#define ledColor_BLUE           0x03
#define ledColor_YELLOW         0x04
#define ledColor_GRAY           0x05
#define ledColor_PURPLE         0x06
#define brightness_low          10
#define brightness_normal       20
#define brightness_high         50

class ros2serial : public rclcpp::Node
{
public:
    ros2serial(std::string nodeName);
    ~ros2serial();

private:
    Communication communication_interface;
    void speech_callback(const audio_msg::msg::SmartAudioData::SharedPtr ai_msg);
    rclcpp::Subscription<audio_msg::msg::SmartAudioData>::SharedPtr speech_cmd;

    void readData();
    void publishData_test();
    std::shared_ptr<std::thread> read_data_thread_;
};
