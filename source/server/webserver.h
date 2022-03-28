#pragma once

#ifdef WITH_WEBSERVER
#include "cmrc/cmrc.hpp"
class Sequencer; // Forward decl...

int StartWebserver(int port, Sequencer *sequencer, bool is_advertised, int trust_level);

void UpdateWebserver();

int StopWebserver();

#endif
