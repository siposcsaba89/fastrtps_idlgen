# fastrtps_idlgen

CMake support to compile idl for eProsima Fast DDS middleware.

## Dependencies

- java - for the idl parser and compiler
- eProsima Fast-DDS-Gen: ```https://github.com/eProsima/Fast-DDS-Gen```
- fstcdr
- fstrtps
- cmake VERSION >= 3.10

## Usage

This library defines the fastrtps_idlgen cmake function:

```
fastrtps_idlgen(
    NAME <lib name>
    IDLS
        <list of idl files>
    INSTALL # generate install targets end exports
    INCLUDES
        <list of idl include directories>
    DEPENDS
        <list of idl library dependency targets>
)
```

After installing you can use `find_package(fastrtps_idlgen REQUIRED)` to include the necessary cmake modules.

The function generates `<lib name>::<lib name>` static libray target which can be used in `target_link_libraries` calls.

## Example

```
cmake_minimum_required(VERSION 3.10)
project(fastrtps_utils_examples)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT TARGET fstcdr)
    find_package(fastcdr CONFIG REQUIRED)
endif()
if (NOT TARGET fastrtps)
    find_package(fastrtps CONFIG REQUIRED)
endif()

if (NOT COMMAND fastrtps_idlgen)
    find_package(fastrtps_idlgen REQUIRED)
endif()

fastrtps_idlgen(
    NAME hello_world_idl_library
    IDLS
        idls/hello_world.idl
        idls/hello_world_include.idl 
    #INSTALL
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/idls
    DEPENDS
)

```

## fastrtps_utils library

Defines simple helper templates to create publishers/sibscribers for the generated idl types.

### Simple subscriber example for the HelloWorld idl template:

```
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
```

### Simple publisher example for the HelloWorld idl template:

```
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
```

## Build and install

```
git clone https://github.com/siposcsaba89/fastrtps_idlgen.git
cd fastrtps_idlgen
mkdir b
cd b
cmake .. -DCMAKE_INSTALL_PREFIX=<install location> -DBUILD_EXAMPLES=<ON,OFF> -DCMAKE_BUILD_TYPE=Release
```



