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

#include "listener.h"

#include "rornet.h"
#include "messaging.h"
#include "sequencer.h"
#include "SocketW.h"
#include "config.h"
#include "UnicodeStrings.h"
#include "utils.h"

#include <stdexcept>
#include <sstream>
#include <stdio.h>

#include "Poco/Util/Application.h"

#ifdef __GNUC__

#include <stdlib.h>

#endif

void *s_lsthreadstart(void *arg) {
    Listener *listener = static_cast<Listener *>(arg);
    listener->threadstart();
    return nullptr;
}


Listener::Listener(Sequencer *sequencer, int port) :
        m_listen_port(port),
        m_sequencer(sequencer),
        m_thread_shutdown(false) {
}

Listener::~Listener(void) {
    if (!m_thread_shutdown) {
        this->Shutdown();
    }
}

bool Listener::Initialize() {
    if (!m_ready_cond.Initialize()) {
        return false;
    }
    return (0 == pthread_create(&m_thread, NULL, s_lsthreadstart, this));
}

void Listener::Shutdown() {
    auto &app = Poco::Util::Application::instance();
    app.logger().trace("Stopping listener thread...");
    m_thread_shutdown = true;
    m_listen_socket.disconnect();
    pthread_join(m_thread, nullptr);
    app.logger().trace("Listener thread stopped");
}

bool Listener::WaitUntilReady() {
    auto &app = Poco::Util::Application::instance();
    int result = 0;
    if (!m_ready_cond.Wait(&result)) {
        app.logger().error( "Internal: Error while starting listener thread");
        return false;
    }
    if (result < 0) {
        app.logger().error( "Internal: Listerer thread failed to start");
        return false;
    }
    return true;
}

void Listener::threadstart() {
    auto &app = Poco::Util::Application::instance();

    app.logger().trace( "Listerer thread starting");
    //here we start
    SWBaseSocket::SWBaseError error;

    //manage the listening socket
    m_listen_socket.bind(m_listen_port, &error);
    if (error != SWBaseSocket::ok) {
        //this is an error!
        app.logger().error( "FATAL Listerer: %s", error.get_error().c_str());
        //there is nothing we can do here
        return;
        // exit(1);
    }
    m_listen_socket.listen();

    app.logger().trace("Listener ready");
    m_ready_cond.Signal(1);

    //await connections
    while (!m_thread_shutdown) {
        app.logger().trace("Listener awaiting connections");
        SWInetSocket *ts = (SWInetSocket *) m_listen_socket.accept(&error);

        if (error != SWBaseSocket::ok) {
            if (m_thread_shutdown) {
                app.logger().error( "INFO Listener shutting down");
            } else {
                app.logger().error( "ERROR Listener: %s", error.get_error().c_str());
            }
        }

        app.logger().trace("Listener got a new connection");

        ts->set_timeout(5, 0);

        //receive a magic
        int type;
        int source;
        unsigned int len;
        unsigned int streamid;
        char buffer[RORNET_MAX_MESSAGE_LENGTH];

        try {
            // this is the start of it all, it all starts with a simple hello
            if (Messaging::ReceiveMessage(ts, &type, &source, &streamid, &len, buffer, RORNET_MAX_MESSAGE_LENGTH))
                throw std::runtime_error("ERROR Listener: receiving first message");

            // make sure our first message is a hello message
            if (type != RoRnet::MSG2_HELLO) {
                Messaging::SendMessage(ts, RoRnet::MSG2_WRONG_VER, 0, 0, 0, 0);
                throw std::runtime_error("ERROR Listener: protocol error");
            }

            // check client version
            if (source == 5000 && (std::string(buffer) == "MasterServer")) {
                app.logger().trace("Master Server knocked ...");
                // send back some information, then close socket
                char tmp[2048] = "";
                sprintf(tmp, "protocol:%s\nrev:%s\nbuild_on:%s_%s\n", RORNET_VERSION, VERSION, __DATE__, __TIME__);
                if (Messaging::SendMessage(ts, RoRnet::MSG2_MASTERINFO, 0, 0, (unsigned int) strlen(tmp), tmp)) {
                    throw std::runtime_error("ERROR Listener: sending master info");
                }
                // close socket
                ts->disconnect(&error);
                delete ts;
                continue;
            }

            // compare the versions if they are compatible
            if (strncmp(buffer, RORNET_VERSION, strlen(RORNET_VERSION))) {
                // not compatible
                Messaging::SendMessage(ts, RoRnet::MSG2_WRONG_VER, 0, 0, 0, 0);
                throw std::runtime_error("ERROR Listener: bad version: " + std::string(buffer) + ". rejecting ...");
            }

            // compatible version, continue to send server settings
            std::string motd_str;
            {
                std::vector<std::string> lines;
                if (!Utils::ReadLinesFromFile(app.config_motdfile, lines))
                {
                    for (const auto& line : lines)
                        motd_str += line + "\n";
                }
            }

            app.logger().trace( "Listener sending server settings");
            RoRnet::ServerInfo settings;
            memset(&settings, 0, sizeof(RoRnet::ServerInfo));
            settings.has_password = !app.config_public_password.empty();
            strncpy(settings.info, motd_str.c_str(), motd_str.size());
            strncpy(settings.protocolversion, RORNET_VERSION, strlen(RORNET_VERSION));
            strncpy(settings.servername, app.config_server_name.c_str(), app.config_server_name.size());
            strncpy(settings.terrain, app.config_terrain_name.c_str(), app.config_terrain_name.size());

            if (Messaging::SendMessage(ts, RoRnet::MSG2_HELLO, 0, 0, (unsigned int) sizeof(RoRnet::ServerInfo),
                                       (char *) &settings))
                throw std::runtime_error("ERROR Listener: sending version");

            //receive user infos
            if (Messaging::ReceiveMessage(ts, &type, &source, &streamid, &len, buffer, RORNET_MAX_MESSAGE_LENGTH)) {
                std::stringstream error_msg;
                error_msg << "ERROR Listener: receiving user infos\n"
                          << "ERROR Listener: got that: "
                          << type;
                throw std::runtime_error(error_msg.str());
            }

            if (type != RoRnet::MSG2_USER_INFO)
                throw std::runtime_error("Warning Listener: no user name");

            if (len > sizeof(RoRnet::UserInfo))
                throw std::runtime_error("Error: did not receive proper user credentials");
            app.logger().information( "Listener creating a new client...");

            RoRnet::UserInfo *user = (RoRnet::UserInfo *) buffer;
            user->authstatus = RoRnet::AUTH_NONE;

            // authenticate
            user->username[RORNET_MAX_USERNAME_LEN - 1] = 0;
            std::string nickname = Str::SanitizeUtf8(user->username);
            user->authstatus = m_sequencer->AuthorizeNick(std::string(user->usertoken, 40), nickname);
            strncpy(user->username, nickname.c_str(), RORNET_MAX_USERNAME_LEN - 1);

            if (app.config_server_mode == "INET") {
                app.logger().trace( "password login: %s == %s?",
                            app.config_public_password.c_str(),
                            std::string(user->serverpassword, 40).c_str());
                if (strncmp(app.config_public_password.c_str(), user->serverpassword, 40)) {
                    Messaging::SendMessage(ts, RoRnet::MSG2_WRONG_PW, 0, 0, 0, 0);
                    throw std::runtime_error("ERROR Listener: wrong password");
                }

                app.logger().trace( "user used the correct password, "
                        "creating client!");
            } else {
                app.logger().trace( "no password protection, creating client");
            }

            //create a new client
            m_sequencer->createClient(ts, *user); // copy the user info, since the buffer will be cleared soon
            app.logger().trace( "listener returned!");
        }
        catch (std::runtime_error &e) {
            app.logger().error( e.what());
            ts->disconnect(&error);
            delete ts;
        }
    }
}
