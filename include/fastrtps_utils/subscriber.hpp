#pragma once

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/Domain.h>

template <typename T>
class Subscriber
{
public:
    Subscriber()
        : mp_participant(nullptr)
        , mp_subscriber(nullptr)
    {

    }
    virtual ~Subscriber()
    {
        eprosima::fastrtps::Domain::removeSubscriber(mp_subscriber);
        eprosima::fastrtps::Domain::removeParticipant(mp_participant);
    }
    //!Initialize the subscriber
    bool init(const std::string & participant_name,
        const std::string & topic_name,
        std::function<void(const typename T::type & data)> f)
    {
        eprosima::fastrtps::ParticipantAttributes PParam;
        PParam.rtps.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol_t::SIMPLE;
        PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
        PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
        PParam.rtps.builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        PParam.domainId = 1;
        PParam.rtps.setName(participant_name.c_str());

        mp_participant = eprosima::fastrtps::Domain::createParticipant(PParam);
        if (mp_participant == nullptr)
        {
            return false;
        }

        //REGISTER THE TYPE
        eprosima::fastrtps::Domain::registerType(mp_participant, &m_type);
        
        //CREATE THE SUBSCRIBER
        eprosima::fastrtps::SubscriberAttributes Rparam;
        Rparam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
        Rparam.topic.topicDataType = m_type.getName();
        //Rparam.topic.topicDataType = 
        Rparam.topic.topicName = topic_name;
        Rparam.topic.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
        Rparam.topic.historyQos.depth = 30;
        Rparam.topic.resourceLimitsQos.max_samples = 50;
        Rparam.topic.resourceLimitsQos.allocated_samples = 20;
        Rparam.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        Rparam.qos.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_subscriber = eprosima::fastrtps::Domain::createSubscriber(mp_participant, Rparam, (eprosima::fastrtps::SubscriberListener*)&m_listener);

        if (mp_subscriber == nullptr)
        {
            return false;
        }

        m_listener.m_func = f;

        return true;
    }
private:
    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Subscriber* mp_subscriber;
public:
    class SubListener :public eprosima::fastrtps::SubscriberListener
    {
    public:
        SubListener() :n_matched(0), n_samples(0) {}
        ~SubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, 
            eprosima::fastrtps::rtps::MatchingInfo& info)
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                n_matched++;
                //std::cout << "Subscriber matched" << std::endl;
            }
            else
            {
                n_matched--;
                //std::cout << "Subscriber unmatched" << std::endl;
            }
        }
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
        {
            if (sub->takeNextData((void*)&m_Hello, &m_info))
            {
                if (m_info.sampleKind == eprosima::fastrtps::rtps::ALIVE)
                {
                    m_func(m_Hello);
                    this->n_samples++;
                }
            }
        }
        typename T::type m_Hello;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
        std::function<void(const typename T::type& data)> m_func;
    } m_listener;
private:
    T m_type;
};
