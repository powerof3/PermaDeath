#pragma once

#include "RE/T/TESDeathEvent.h"

namespace RE
{
	class BGSSaveLoadManager
	{
	public:
        static BGSSaveLoadManager* GetSingleton()
		{
			REL::Relocation<BGSSaveLoadManager**> singleton{ REL::ID(880997) };
			return *singleton;
		}

		// members
		std::uint8_t  unk00[0x110];     // 000
		std::uint64_t currentPlayerID;  // 110
	};
	static_assert(offsetof(BGSSaveLoadManager, currentPlayerID) == 0x110);

	std::uint32_t GetNumTimesUnityEntered();
}

namespace PermaDeath
{
	class EventHandler :
		public ISingleton<EventHandler>,
		public RE::BSTEventSink<RE::TESDeathEvent>
	{
		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent& a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) override;
	};

	void InstallOnPostLoad();
}
