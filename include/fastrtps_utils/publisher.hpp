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

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>


template <typename T>
class Publisher
{
public:
    Publisher()
        : mp_participant(nullptr)
        , mp_publisher(nullptr)
    {
    }

    virtual ~Publisher()
    {
        eprosima::fastrtps::Domain::removeParticipant(mp_participant);
    }
    //!Initialize
    bool init(const std::string& participant_name, const std::string & topic_name)
    {
        eprosima::fastrtps::ParticipantAttributes PParam;
        PParam.domainId = 1;
        PParam.rtps.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol_t::SIMPLE;
        PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
        PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
        PParam.rtps.builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        PParam.rtps.setName(participant_name.c_str());
        mp_participant = eprosima::fastrtps::Domain::createParticipant(PParam);

        if (mp_participant == nullptr)
        {
            return false;
        }
        //REGISTER THE TYPE

        eprosima::fastrtps::Domain::registerType(mp_participant, &m_type);

        //CREATE THE PUBLISHER
        eprosima::fastrtps::PublisherAttributes Wparam;
        Wparam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
        Wparam.topic.topicDataType = m_type.getName();
        Wparam.topic.topicName = topic_name;
        Wparam.topic.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
        Wparam.topic.historyQos.depth = 30;
        Wparam.topic.resourceLimitsQos.max_samples = 50;
        Wparam.topic.resourceLimitsQos.allocated_samples = 20;
        Wparam.times.heartbeatPeriod.seconds = 2;
        Wparam.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
        Wparam.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        mp_publisher = eprosima::fastrtps::Domain::createPublisher(mp_participant, Wparam, (eprosima::fastrtps::PublisherListener*)&m_listener);
        if (mp_publisher == nullptr)
        {
            return false;
        }

        return true;

    }    //!Publish a sample
    bool publish(const typename T::type & data, bool waitForListener = true)
    {
        if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
        {
            mp_publisher->write((void*)&data);
            return true;
        }
        return false;
    }

private:
    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_publisher;

    class PubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        PubListener() :n_matched(0), firstConnected(false) {};
        ~PubListener() {};
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info)
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                n_matched++;
                firstConnected = true;
            }
            else
            {
                n_matched--;
            }
        }
    public:
        int n_matched;
        bool firstConnected;

    } m_listener;

    T m_type;
};


#endif /* HELLOWORLDPUBLISHER_H_ */