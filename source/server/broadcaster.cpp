/*
This file is part of "Rigs of Rods Server" (Relay mode)

Copyright 2007   Pierre-Michel Ricordel
Copyright 2014+  Rigs of Rods Community

"Rigs of Rods Server" is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

"Rigs of Rods Server" is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar. If not, see <http://www.gnu.org/licenses/>.
*/

#include "broadcaster.h"

#include "SocketW.h"
#include "messaging.h"
#include "sequencer.h"

#include <algorithm>
#include <map>

#include "Poco/Util/Application.h"

Broadcaster::Broadcaster(Sequencer *sequencer)
    : m_sequencer(sequencer), m_client(nullptr), m_is_dropping_packets(false), m_packet_drop_counter(0),
      m_packet_good_counter(0), _activity(this, &Broadcaster::runActivity)
{
}

void Broadcaster::Start(Client *client)
{
    m_client = client;
    m_is_dropping_packets = false;
    m_packet_drop_counter = 0;
    m_packet_good_counter = 0;
    m_msg_queue.clear();
}

void Broadcaster::runActivity()
{
    auto &app = Poco::Util::Application::instance();
    bool socket_error = false;
    while (!_activity.isStopped())
    {
        queue_entry_t msg;
        // define a new scope and use a scope lock
        {
            std::unique_lock<std::mutex> scoped_lock(m_queue_mutex);
            while (m_msg_queue.empty())
            {
                m_queue_cond.wait(scoped_lock);
            }

            if (!m_msg_queue.empty())
            { // This shouldn't be needed, but rorserver is haunted:
              // https://github.com/RigsOfRods/ror-server/pull/90#issuecomment-500597467 ~ only_a_ptr, 06/2019
                msg = m_msg_queue.front();
                m_msg_queue.pop_front();
            }
        }

        if (msg.type == RoRnet::MSG2_STREAM_DATA_DISCARDABLE)
        {
            msg.type = RoRnet::MSG2_STREAM_DATA;
        }

        // TODO WARNING THE SOCKET IS NOT PROTECTED!!!
        if (Messaging::SWSendMessage(m_client->GetSocket(), msg.type, msg.uid, msg.streamid, msg.datalen, msg.data) !=
            0)
        {
            m_sequencer->QueueClientForDisconnect(m_client->GetUserId(), "Broadcaster: Send error", true, true);
            socket_error = true;
            break;
        }
    }

    if (!socket_error)
    {
        const std::lock_guard<std::mutex> scoped_lock(m_queue_mutex);
        for (const auto &msg : m_msg_queue)
        {
            if (msg.type != RoRnet::MSG2_STREAM_DATA && msg.type != RoRnet::MSG2_STREAM_DATA_DISCARDABLE)
            {
                Messaging::SWSendMessage(m_client->GetSocket(), msg.type, msg.uid, msg.streamid, msg.datalen, msg.data);
            }
        }
    }

    app.logger().trace("Broadcaster thread  (client_id %d) exits", m_client->GetUserId());
}

// this is called all the way from the receiver threads, we should process this swiftly
// and keep in mind that it is called crazily and concurently from lots of threads
// we MUST copy the data too
// also, this function can be called by threads owning clients_mutex !!!
void Broadcaster::QueueMessage(int type, int uid, unsigned int streamid, unsigned int len, const char *data)
{
    if (_activity.isStopped())
    {
        return;
    }
    queue_entry_t msg = {type, uid, streamid, len, ""};
    memcpy(msg.data, data, len);

    {
        const std::lock_guard<std::mutex> scoped_lock(m_queue_mutex);
        if (m_msg_queue.empty())
        {
            m_packet_drop_counter = 0;
            m_is_dropping_packets = (++m_packet_good_counter > 3) ? false : m_is_dropping_packets;
        }
        else if (type == RoRnet::MSG2_STREAM_DATA_DISCARDABLE)
        {
            auto search = std::find_if(m_msg_queue.begin(), m_msg_queue.end(), [&](const queue_entry_t &m) {
                return m.type == RoRnet::MSG2_STREAM_DATA_DISCARDABLE && m.uid == uid && m.streamid == streamid;
            });
            if (search != m_msg_queue.end())
            {
                // Found outdated discardable streamdata -> replace it
                (*search) = msg;
                m_packet_good_counter = 0;
                m_is_dropping_packets = (++m_packet_drop_counter > 3) ? true : m_is_dropping_packets;
                Messaging::StatsAddOutgoingDrop(sizeof(RoRnet::Header) + msg.datalen); // Statistics
                return;
            }
        }
        m_msg_queue.push_back(msg);
    }

    m_queue_cond.notify_all();
}
