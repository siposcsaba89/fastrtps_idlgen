#if 0
#include <iostream>
#include <fastrtps_utils/subscriber.hpp>
#include <hello_worldPubSubTypes.h>

typedef Subscriber<HelloWorldPubSubType> HelloWorldSubscriber;

void alma(const HelloWorld& data)
{
    // Print your structure data here.
    std::cout << "Message " << data.msg() << " " << data.msg2() << " RECEIVED" << std::endl;
}

int main()
{
    HelloWorldSubscriber mysub;
    if (mysub.init("subscriber", "HelloWorldTopic", &alma))
    {
        std::cin.ignore();
        //mysub.run();
    }
    return 0;
}

#else

#include <iostream>
#include <fastrtps_utils/subscriber_dds.hpp>
#include <hello_worldPubSubTypes.h>

typedef SubscriberDDS<HelloWorldPubSubType> HelloWorldSubscriber;

void alma(const HelloWorld& data)
{
    // Print your structure data here.
    std::cout << "Message " << data.msg() << " " << data.msg2() << " RECEIVED" << std::endl;
    std::cout << data.alma().size() << std::endl;
}

int main()
{
    HelloWorldSubscriber mysub;
    if (mysub.init("subscriber", "HelloWorldTopic", &alma))
    {
        std::cin.ignore();
        //mysub.run();
    }
    return 0;
}
#endif