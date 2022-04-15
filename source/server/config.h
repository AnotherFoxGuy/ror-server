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

#ifdef _WIN32
#define config_RESOURCE_DIR ""
#else // _WIN32 ~ trailing slash important
#define config_RESOURCE_DIR "/usr/share/rorserver/"
#endif // _WIN32

// INET, LAN, AUTO

#define config_server_mode config().getString("mode", "LAN")

#define config_server_name config().getString("name")
#define config_terrain_name config().getString("terrain", "any")
#define config_public_password config().getString("password")
#define config_ip_addr config().getString("ip", "0.0.0.0")
#define config_scriptname config().getString("scriptfile")
#define config_authfile config().getString("authfile", "server.auth")
#define config_motdfile config().getString("motdfile", "server.motd")
#define config_rulesfile config().getString("rulesfile", "server.rules")
#define config_blacklistfile config().getString("blacklistfile", "server.blacklist")
#define config_owner config().getString("owner")
#define config_website config().getString("website")
#define config_irc config().getString("irc")
#define config_voip config().getString("voip")
#define config_serverlist_host config().getString("serverlist-host", "api.rigsofrods.org")
#define config_serverlist_path config().getString("serverlist-path", "")
#define config_resourcedir config().getString("resourcedir", RESOURCE_DIR)

#define config_listen_port config().getInt("port", 14000)
#define config_max_clients config().getInt("slots", 16)
#define config_heartbeat_retry_count config().getInt("heartbeat-retry-count", 5)
#define config_heartbeat_retry_seconds config().getInt("heartbeat-retry-seconds", 15)
#define config_heartbeat_interval_sec config().getInt("heartbeat-interval", 60)

#define config_print_stats config().getBool("printstats")
#define config_foreground config().getBool("foreground")

// Vehicle spawn limits
#define config_max_vehicles config().getInt("vehiclelimit", 0)
#define config_spawn_interval_sec config().getInt("vehicle-spawn-interval", 0)
#define config_max_spawn_rate config().getInt("vehicle-max-spawn-rate", 0)

// 0 disables spamfilter
#define config_spamfilter_msg_interval_sec config().getInt("spamfilter-msg-interval", 0)
#define config_spamfilter_msg_count config().getInt("spamfilter-msg-count", 0)
#define config_spamfilter_gag_duration_sec config().getInt("spamfilter-gag-duration", 0)
