#include "Hooks.h"
#include "Settings.h"

namespace RE
{
	std::uint32_t GetNumTimesUnityEntered()
	{
		using func_t = decltype(&GetNumTimesUnityEntered);
		REL::Relocation<func_t> func{ REL::ID(153500) };
		return func();
	}
}

namespace PermaDeath
{
	bool CanTriggerPermaDeath(const std::uint32_t a_numTimesUnityEntered)
	{
		const auto settings = Settings::GetSingleton();
		if (settings->ngZero && settings->ngPlus) {
			return true;
		}
	    if (settings->ngZero && a_numTimesUnityEntered == 0) {
			return true;
		}
		if (settings->ngPlus && a_numTimesUnityEntered >= 1) {
			return true;
		}
		return false;
	}

    std::uint64_t GetCurrentPlayerID()
	{
		if (const auto saveLoadMgr = RE::BGSSaveLoadManager::GetSingleton()) {
			return saveLoadMgr->currentPlayerID;
		}
		return 0;
	}

	// https://stackoverflow.com/questions/70257751/move-a-file-or-folder-to-the-recyclebin-trash-c17
	bool RecycleSaves(const std::wstring& path)
	{
		const std::wstring widestr = path + L'\0';

		SHFILEOPSTRUCT fileOp;
		fileOp.hwnd = nullptr;
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = widestr.c_str();
		fileOp.pTo = nullptr;
		fileOp.fFlags = FOF_ALLOWUNDO | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;

	    return SHFileOperation(&fileOp) == 0;
	}

	void DeleteSaves(const std::uint64_t a_playerID, std::uint32_t a_numTimesUnityEntered)
	{
		auto save_directory = []() -> std::optional<std::filesystem::path> {
			wchar_t*                                                                 buffer{};
			const auto                                                               result = SFSE::WinAPI::SHGetKnownFolderPath(SFSE::WinAPI::FOLDERID_DOCUMENTS, SFSE::WinAPI::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
			const std::unique_ptr<wchar_t[], decltype(&SFSE::WinAPI::CoTaskMemFree)> knownPath(buffer, SFSE::WinAPI::CoTaskMemFree);
			if (!knownPath || result != 0) {
				logger::error("failed to get known folder path"sv);
				return std::nullopt;
			}

			std::filesystem::path path = knownPath.get();
			path /= R"(My Games\Starfield\)";
			path /= RE::INISettingCollection::GetSingleton()->GetSetting("sLocalSavePath:General")->GetString();

			logger::info("Save directory : {}", path.string());

		    return path;
		};

		static const auto saveDir = save_directory();

		logger::info("PlayerID : {:X} [NG{}+]", a_playerID, a_numTimesUnityEntered);

		if (saveDir) {
		    std::vector<std::filesystem::path> savesToDelete{};
			for (const auto& dirEntry : std::filesystem::directory_iterator(*saveDir)) {
				if (dirEntry.exists() && dirEntry.path().extension() == ".sfs") {
				    if (const auto save = string::split(dirEntry.path().string(), "_"); save.size() == 8) { // vanilla saves have 8 underscores
						if (string::to_num<std::uint64_t>(save[1], true) == a_playerID) {
							savesToDelete.push_back(dirEntry.path());
						}
					}
				}
			}
			const auto recycleSaves = Settings::GetSingleton()->sendSavesToRecycleBin;
		    for (auto& save : savesToDelete) {
				logger::info("\tDeleting {}", save.filename().string());
				if (recycleSaves) {
					RecycleSaves(save.wstring());
				} else {
					std::filesystem::remove(save);
				}
			}
		}
	}

	RE::BSEventNotifyControl EventHandler::ProcessEvent(const RE::TESDeathEvent& a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
	    if (a_event.actorDying && a_event.actorDying->IsPlayerRef() && !a_event.dead) {
			if (const auto numTimesUnityEntered = RE::GetNumTimesUnityEntered(); CanTriggerPermaDeath(numTimesUnityEntered)) {
				if (const auto currentPlayerID = GetCurrentPlayerID(); currentPlayerID != 0) {
					DeleteSaves(currentPlayerID, numTimesUnityEntered);
				}
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	void InstallOnPostLoad()
	{
		const auto settings = Settings::GetSingleton();
	    settings->Load();

	    if (!settings->enablePermaDeath || (!settings->ngZero && !settings->ngPlus)) {
			logger::info("PermaDeath disabled...");
		} else {
			RE::TESDeathEvent::GetEventSource()->RegisterSink(EventHandler::GetSingleton());
			logger::info("Registered TESDeathEventHandler");
		}
	}
}
