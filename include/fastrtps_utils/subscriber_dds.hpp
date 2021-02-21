#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <memory>

template <typename T>
class SubscriberDDS
{
public:
    SubscriberDDS()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new T())
    {

    }
    virtual ~SubscriberDDS()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
    //!Initialize the subscriber
    bool init(const std::string & participant_name,
        const std::string & topic_name,
        std::function<void(const typename T::type & data)> f)
    {
        //CREATE THE PARTICIPANT
        eprosima::fastdds::dds::DomainParticipantQos pqos;
        pqos.name(participant_name);
        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        pqos.transport().send_socket_buffer_size = 1048576;
        pqos.transport().listen_socket_buffer_size = 4194304;
        std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> descriptor =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);
        //descriptor->maxMessageSize = 1;
        pqos.transport().user_transports.push_back(descriptor);

        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(1, pqos);
        if (participant_ == nullptr)
        {
            return false;
        }

        //REGISTER THE TYPE
        type_.register_type(participant_);

        //CREATE THE SUBSCRIBER
        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr)
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

        //CREATE THE READER
        eprosima::fastdds::dds::DataReaderQos rqos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        rqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        rqos.history().depth = 30;
        rqos.resource_limits().max_samples = 50;
        rqos.resource_limits().allocated_samples = 20;
        //rqos.reliability().kind = reliabilityKind;
        rqos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        rqos.endpoint().history_memory_policy =
            eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

        reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
        if (reader_ == nullptr)
        {
            return false;
        }

        listener_.m_func = f;
        return true;
    }
private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;
public:
    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        SubListener() = default;

        ~SubListener() override = default;

        void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override
        {
            // Take data
            eprosima::fastdds::dds::SampleInfo info;

            if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    m_func(st);
                    // Print your structure data here.
                    ++samples;
                }
            }
        }

        void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                matched = info.total_count;
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched = info.total_count;
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                    << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        int matched = 0;
        uint32_t samples = 0;
        std::function<void(const typename T::type& data)> m_func;
        typename T::type st;
    }
    listener_;
};
