#if 0
#include <iostream>
#include <fastrtps_utils/publisher.hpp>

#include "hello_world.h"
#include "hello_worldPubSubTypes.h"

typedef Publisher<HelloWorldPubSubType> HelloWorldPublisher;

int main()
{
    HelloWorld my_msg;
    HelloWorldPublisher mypub;
    if (mypub.init("publisher", "HelloWorldTopic"))
    {
        for (int i = 0; i < 10000; i += 10)
        {
            my_msg.msg() = std::to_string(i) + " Hellp !!!!";
            my_msg.msg2() = std::to_string(i * i * i) + " Bye bye !!!!";
            std::thread a([&mypub, &my_msg]() {mypub.publish(my_msg, true); });
            a.join();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    return 0;
}
#else

#include <iostream>
#include <fastrtps_utils/publisher_dds.hpp>
#include "hello_world.h"
#include "hello_worldPubSubTypes.h"

typedef PublisherDDS<HelloWorldPubSubType> HelloWorldPublisher;

int main()
{
    HelloWorldPublisher mypub;
    std::string msg_str;
    if (mypub.init("publisher", "HelloWorldTopic"))
    {
        for (int i = 0; i < 10000; i += 10)
        {
            HelloWorld my_msg;

            msg_str  = std::to_string(i) + " Hellp !!!! \n";
            my_msg.msg2()  = msg_str;
            my_msg.msg()  = msg_str;

            if (!mypub.publish(my_msg, true))
            {
                std::cout << "Failed to publish" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    return 0;
}
#endif