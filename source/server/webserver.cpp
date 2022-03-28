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
along with "Rigs of Rods Server".
If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef WITH_WEBSERVER

#include "webserver.h"

#include "config.h"
#include "logger.h"
#include "master-server.h"
#include "messaging.h"
#include "sequencer.h"
#include "userauth.h"

#include "utils.h"

// important webserver context
struct mg_context *ctx;

#ifdef _WIN32
#include <winsock.h>
#define snprintf _snprintf

#ifndef _WIN32_WCE
#ifdef _MSC_VER /* pragmas not valid on MinGW */
#endif          /* _MSC_VER */
#else           /* _WIN32_WCE */
/* Windows CE-specific definitions */
#pragma comment(lib, "ws2")
//#include "compat_wince.h"
#endif /* _WIN32_WCE */

#else

#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>

#endif /* _WIN32 */

#ifndef _WIN32_WCE /* Some ANSI #includes are not available on Windows CE */

#include <errno.h>
#include <signal.h>
#include <time.h>

#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctemplate/template.h>

#include <json/json.h>

#include "mongoose.h"

static bool s_is_advertised;
static int s_trust_level;
static Sequencer *s_sequencer;

static ctemplate::TemplateDictionary *getTemplateDict(std::string title) {
  ctemplate::TemplateDictionary *dict =
      new ctemplate::TemplateDictionary("website");

  // if you remove the following line, it will enforce reading from the disc ->
  // nice to create the templates
#ifdef _DEBUG
  ctemplate::Template::ClearCache();
#endif

  ctemplate::TemplateDictionary::SetGlobalValue(
      "TITLE", "Rigs of Rods Server - " + title);

  ctemplate::TemplateDictionary *dict_header =
      dict->AddIncludeDictionary("HEADER");
  dict_header->SetFilename(Config::getResourceDir() +
                           "webserver/templates/header.html");
  ctemplate::TemplateDictionary *dict_footer =
      dict->AddIncludeDictionary("FOOTER");
  dict_footer->SetFilename(Config::getResourceDir() +
                           "webserver/templates/footer.html");
  return dict;
}

static void renderTemplate(ctemplate::TemplateDictionary *dict,
                           struct mg_connection *conn,
                           std::string templatefile) {
  std::string output;
  ctemplate::ExpandTemplate(templatefile, ctemplate::DO_NOT_STRIP, dict,
                            &output);

  mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");
  mg_printf(conn, "%s", "Content-Type: text/html\r\n\r\n");
  mg_write(conn, output.c_str(), output.size());

  delete (dict);
  dict = 0;
}

static void show_index(struct mg_connection *conn,
                       const struct mg_request_info *request_info, void *data) {
  ctemplate::TemplateDictionary *dict = getTemplateDict("Overview");

  dict->SetIntValue("MAX_CLIENTS", Config::getMaxClients());
  dict->SetValue("SERVER_NAME", Config::getServerName());
  dict->SetValue("TERRAIN_NAME", Config::getTerrainName());
  dict->SetValue("PASSWORD_PROTECTED",
                 Config::getPublicPassword().empty() ? "No" : "Yes");
  dict->SetValue("IP_ADDRESS", Config::getIPAddr() == "0.0.0.0"
                                   ? "0.0.0.0 (Any)"
                                   : Config::getIPAddr());
  dict->SetIntValue("LISTENING_PORT", Config::getListenPort());
  dict->SetValue("PROTOCOL_VERSION", std::string(RORNET_VERSION));

  if (Config::getServerMode() == SERVER_LAN) {
    dict->SetValue("SERVER_MODE", "LAN");
  } else if (Config::getServerMode() == SERVER_INET) {
    if (s_is_advertised) {
      dict->SetValue("SERVER_MODE", "Public, Internet");
      dict->SetIntValue("SERVER_TRUST_LEVEL", s_trust_level);
    } else {
      dict->SetValue("SERVER_MODE", "Private, Internet");
    }
  }

  dict->SetValue("ADVERTISED_ON_MASTER_SERVER", s_is_advertised ? "YES" : "NO");
  dict->SetValue("PRINT_CONSOLE_STATS", Config::getPrintStats() ? "YES" : "NO");

#ifdef WITH_ANGELSCRIPT
  dict->SetValue("SCRIPTING_SUPPORT", "Yes");
  dict->SetValue("SCRIPTING_ENABLED",
                 Config::getEnableScripting() ? "YES" : "NO");
  if (Config::getEnableScripting())
    dict->SetValue("SCRIPTNAME_IN_USE", Config::getScriptName());
#else  // WITH_ANGELSCRIPT
    dict->SetValue("SCRIPTING_SUPPORT", "NO"));
#endif // WITH_ANGELSCRIPT

  dict->SetIntValue("WEBSERVER_PORT", Config::getWebserverPort());

  renderTemplate(dict, conn,
                 Config::getResourceDir() + "webserver/templates/index.html");
}

static void show_playerlist(struct mg_connection *conn,
                            const struct mg_request_info *request_info,
                            void *data) {
  ctemplate::TemplateDictionary *dict = getTemplateDict("Playerlist");

  std::vector<WebserverClientInfo> clients = s_sequencer->GetClientListCopy();

  for (auto it = clients.begin(); it != clients.end(); it++) {
    ctemplate::TemplateDictionary *sub_dict =
        dict->AddSectionDictionary("PLAYERS");

    if (it->GetStatus() == Client::STATUS_FREE) {
      sub_dict->SetValue("STATUS", "FREE");
    } else if (it->GetStatus() == Client::STATUS_BUSY) {
      sub_dict->SetValue("STATUS", "BUSY");
    } else if (it->GetStatus() == Client::STATUS_USED) {
      // some auth identifiers
      std::string authst = "none";
      if (it->user.authstatus & RoRnet::AUTH_BANNED) authst = "banned";
      if (it->user.authstatus & RoRnet::AUTH_BOT) authst = "bot";
      if (it->user.authstatus & RoRnet::AUTH_RANKED) authst = "ranked";
      if (it->user.authstatus & RoRnet::AUTH_MOD) authst = "moderator";
      if (it->user.authstatus & RoRnet::AUTH_ADMIN) authst = "admin";

      sub_dict->SetValue("STATUS", "USED");
      sub_dict->SetIntValue("UID", it->user.uniqueid);
      sub_dict->SetValue("IP", it->GetIpAddress());
      sub_dict->SetValue("NAME", it->user.username);
      sub_dict->SetValue("AUTH", authst);
    }
  }

  renderTemplate(dict, conn,
                 Config::getResourceDir() +
                     "webserver/templates/playerlist.html");
}

static void show_404(struct mg_connection *conn,
                     const struct mg_request_info *request_info,
                     void *user_data) {
  mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");
  mg_printf(conn, "%s", "Content-Type: text/plain\r\n\r\n");
  mg_printf(conn, "%s", "Oops. File not found! ");
}

static void signal_handler(int sig_num) {
  switch (sig_num) {
#ifndef _WIN32
  case SIGCHLD:
    while (waitpid(-1, &sig_num, WNOHANG) > 0)
      ;
    break;
#endif /* !_WIN32 */
  case 0:
  default:
    break;
  }
}

int StartWebserver(int port, Sequencer *sequencer, bool is_advertised,
                   int trust_level) {
  s_is_advertised = is_advertised;
  s_trust_level = trust_level;
  s_sequencer = sequencer;

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, &signal_handler);
#endif /* !_WIN32 */

  ctx = mg_start();

  char portStr[30] = "";
  sprintf(portStr, "%d", port);
  mg_set_option(ctx, "ports", portStr);
  mg_set_option(ctx, "dir_list", "no");

  mg_set_uri_callback(ctx, "/", &show_index, NULL);
  mg_set_uri_callback(ctx, "/playerlist", &show_playerlist, NULL);

  mg_set_error_callback(ctx, 404, show_404, NULL);
  return 0;
}

int StopWebserver() {
  mg_stop(ctx);
  return 0;
}

#endif // WITH_WEBSERVER
