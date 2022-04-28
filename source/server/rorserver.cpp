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

// RoRserver.cpp : Defines the entry point for the console application.

#include "config.h"
#include "listener.h"
#include "master-server.h"
#include "messaging.h"
#include "rornet.h"
#include "sequencer.h"
#include "utils.h"
#include "config.h"

#include "sha1.h"
#include "sha1_util.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <stdio.h>
#include <string.h>

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/IniFileConfiguration.h"

using namespace Poco::Util;

static Sequencer s_sequencer;
static MasterServer::Client s_master_server;

class HeartBeatTask : public Poco::Task
{
  public:
    HeartBeatTask() : Task("HeartBeatTask")
    {
    }

    void runTask()
    {
        Application &app = Application::instance();
        // start the main program loop
        // if we need to communiate to the master user the notifier routine
        if (app.config_server_mode == "INET")
        {
            // heartbeat
            while (!sleep(app.config_heartbeat_interval_sec))
            {
                Messaging::UpdateMinuteStats();
                s_sequencer.UpdateMinuteStats();
                app.logger().trace("Sending heartbeat...");
                Json::Value user_list(Json::arrayValue);
                s_sequencer.GetHeartbeatUserList(user_list);
                if (!s_master_server.SendHeatbeat(user_list))
                {
                    unsigned int timeout = app.config_heartbeat_retry_seconds;//app.config().getIntHeartbeatRetrySeconds();
                    unsigned int max_retries = app.config_heartbeat_retry_count;
                    app.logger().warning("A heartbeat failed! Retry in %d seconds.", timeout);
                    bool success = false;
                    for (unsigned int i = 0; i < max_retries; ++i)
                    {
                        Utils::SleepSeconds(timeout);
                        success = s_master_server.SendHeatbeat(user_list);

//                        LogLevel log_level = (success ? LOG_INFO : LOG_ERROR);
//                        const char *log_result = (success ? "successful." : "failed.");
//                        Logger::Log(log_level, "Heartbeat retry %d/%d %s", i + 1, max_retries, log_result);
                        if (success)
                        {
                            break;
                        }
                    }
                    if (!success)
                    {
                        app.logger().error( "Unable to send heartbeats, exit");

                        return;
                    }
                }
                else
                {
                    app.logger().trace("Heartbeat sent OK");
                }
            }
        }
        else
        {
            while (!sleep(60000))
            {
                app.logger().trace("AAAAAAAAAAAAAAAAAAAAAAAAA");



                Messaging::UpdateMinuteStats();
                s_sequencer.UpdateMinuteStats();

                // broadcast our "i'm here" signal
                Messaging::broadcastLAN();
            }
        }
    }
};

class RoRServer : public ServerApplication
{
  public:
    RoRServer() : _helpRequested(false)
    {
    }

    ~RoRServer()
    {
    }

  protected:
    void initialize(Application &self)
    {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void uninitialize()
    {
        logger().information("shutting down");
        if (s_master_server.IsRegistered())
        {
            s_master_server.UnRegister();
        }
        ServerApplication::uninitialize();
    }

    void defineOptions(OptionSet &options)
    {
        ServerApplication::defineOptions(options);

        options.addOption(Option("help", "h", "display help information on command line arguments")
                              .required(false)
                              .repeatable(false)
                              .callback(OptionCallback<RoRServer>(this, &RoRServer::handleHelp)));
        options.addOption(Option("config", "c", "Loads the configuration from a file")
                              .required(false)
                              .repeatable(false)
                              .callback(OptionCallback<RoRServer>(this, &RoRServer::loadConfig)));
    }

    void loadConfig(const std::string &name, const std::string &value)
    {
        logger().information( "Loading config from %s", value);
        loadConfiguration(value);
    }

    void handleHelp(const std::string &name, const std::string &value)
    {
        _helpRequested = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void displayHelp()
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader(
            "A sample server application that demonstrates some of the features of the Util::ServerApplication class.");
        helpFormatter.format(std::cout);
    }

    int main(const std::vector<std::string> &args)
    {
        // set default verbose levels
        logger().setLevel("trace");

        // Check configuration
        if (config_server_mode == "INET")
        {
            logger().information( "Starting server in INET mode");
            std::string ip_addr = config().getString("ip", "127.0.0.1");
            if (ip_addr.empty() || (ip_addr == "0.0.0.0"))
            {
                logger().warning("No IP given, detecting...");
                if (!MasterServer::RetrievePublicIp())
                {
                    logger().error( "Failed to auto-detect public IP, exit.");
                    return Application::EXIT_TEMPFAIL;
                }
            }
            logger().information( "IP address: %s", config_ip_addr.c_str());

            unsigned int max_clients = config_max_clients;
            logger().information( "Maximum required upload: %ikbit/s", max_clients * (max_clients - 1) * 64);
            logger().information( "Maximum required download: %ikbit/s", max_clients * 64);

            if (config_server_name.empty())
            {
                logger().error( "Server name not specified, exit.");
                return Application::EXIT_USAGE;
            }
            logger().information( "Server name: %s", config_server_name.c_str());
        }

        if (!sha1check())
        {
            logger().error( "sha1 malfunction!");
            return Application::EXIT_SOFTWARE;
        }

        Listener listener(&s_sequencer, config_listen_port);
        listener.activity().start();
        s_sequencer.Initialize();

        // Listener is ready, let's register ourselves on serverlist (which will contact us back to check).
        if (config_server_mode != "LAN")
        {
            bool registered = s_master_server.Register();
            if (!registered && (config_server_mode == "INET"))
            {
                logger().error( "Failed to register on serverlist. Exit");
                listener.activity().stop();
                return Application::EXIT_UNAVAILABLE;
            }
            else if (!registered) // server_mode == SERVER_AUTO
            {
                logger().warning("Failed to register on serverlist, continuing in LAN mode");
                config().setString("mode", "LAN");
            }
            else
            {
                logger().information( "Registration successful");
            }
        }

        Poco::TaskManager tm;
        tm.start(new HeartBeatTask);

        waitForTerminationRequest();

        tm.cancelAll();
        tm.joinAll();

        s_sequencer.Close();

        return Application::EXIT_OK;
    }

  private:
    bool _helpRequested;
};

POCO_SERVER_MAIN(RoRServer)