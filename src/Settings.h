#pragma once

class Settings : public ISingleton<Settings>
{
public:
	void Load()
	{
		constexpr auto path = L"Data/SFSE/Plugins/po3_PermaDeath.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		ini::get_value(ini, enablePermaDeath, "Settings", "bEnablePermaDeath", nullptr);
		ini::get_value(ini, ngZero, "Settings", "bNG0", nullptr);
		ini::get_value(ini, ngPlus, "Settings", "bNG+", nullptr);
		ini::get_value(ini, sendSavesToRecycleBin, "Settings", "bSendSavesToRecycleBin", nullptr);

		(void)ini.SaveFile(path);
	}

	bool enablePermaDeath{ false };
	bool sendSavesToRecycleBin{ false };
	bool ngPlus{ false };
	bool ngZero{ false };
};
