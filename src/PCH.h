#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;

namespace logger = SKSE::log;

// Common RE headers
#include <RE/M/MenuOpenCloseEvent.h>
#include <RE/D/DialogueMenu.h>
#include <RE/M/MenuTopicManager.h>
#include <RE/U/UI.h>
#include <RE/A/Actor.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESFaction.h>
#include <RE/T/TESBoundObject.h>
