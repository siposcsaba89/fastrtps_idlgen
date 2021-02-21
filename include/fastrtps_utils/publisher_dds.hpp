// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <memory>
#include <iostream>

template <typename T>
class PublisherDDS
{
public:
    PublisherDDS()
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new T())
    {
    }

    virtual ~PublisherDDS()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
    //!Initialize
    bool init(const std::string& participant_name, const std::string & topic_name)
    {
        /* Initialize data_ here */

           //CREATE THE PARTICIPANT
        eprosima::fastdds::dds::DomainParticipantQos pqos;
        pqos.transport().send_socket_buffer_size = 1048576;
        pqos.transport().listen_socket_buffer_size = 4194304;

        pqos.name(participant_name);
        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;

        std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> descriptor = 
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);

        pqos.transport().user_transports.push_back(descriptor);

        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(1, pqos);
        if (participant_ == nullptr)
        {
            return false;
        }

        //REGISTER THE TYPE
        type_.register_type(participant_);

        //CREATE THE PUBLISHER
        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher_ == nullptr)
        {
            return false;
        }

        //CREATE THE TOPIC
        topic_ = participant_->create_topic(
            topic_name,
            type_.get_type_name(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            return false;
        }

        //CREATE THE DATAWRITER
        eprosima::fastdds::dds::DataWriterQos wqos;
        wqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        wqos.history().depth = 30;
        wqos.resource_limits().max_samples = 50;
        wqos.resource_limits().allocated_samples = 20;
        wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
        wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
        //wqos.reliability().kind = reliabilityKind;
        wqos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
        wqos.endpoint().history_memory_policy =
            eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        // CREATE THE WRITER
        writer_ = publisher_->create_datawriter(topic_, wqos, &listener_);
        if (writer_ == nullptr)
        {
            return false;
        }
        return true;
    }    //!Publish a sample
    bool publish(const typename T::type & data, bool waitForListener = true)
    {
        if (listener_.matched == 0)
            return false;

        if (writer_->write((void*)&data))
        {
            return true;
        }
        return false;
    }

private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;


    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener() = default;

        ~PubListener() override = default;

        void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                matched = info.total_count;
                std::cout << "DataWriter matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched = info.total_count;
                std::cout << "DataWriter unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                    << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
            }

        }

        int matched = 0;
    }
    listener_;
};


#endif /* HELLOWORLDPUBLISHER_H_ */