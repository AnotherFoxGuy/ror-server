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

#pragma once

#include "SocketW.h"
#include "prerequisites.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

#include <Poco/Activity.h>

class Listener
{
  private:
    SWInetSocket m_listen_socket;
    int m_listen_port;
    Sequencer *m_sequencer;
    Poco::Activity<Listener> _activity;
    void runActivity();

  public:
    Listener(Sequencer *sequencer, int port);

    Poco::Activity<Listener> &activity()
    {
        return _activity;
    }

};
