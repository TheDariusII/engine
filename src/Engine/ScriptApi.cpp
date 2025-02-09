#include <Engine/ScriptApi.h>
#include <stdlib.h>
#include <iostream>

//TODO(darius) throws exception
// DLL info - https://learn.microsoft.com/en-us/troubleshoot/windows-client/deployment/dynamic-link-library
DllScriptRoutine::DllScriptRoutine(std::string_view path_in, GameState* inst) : instance(inst), path(path_in)
{
	 //"E:/own/programming/engine/logicScripts/EngineLogic/x64/Debug"
	loadDll(path);
}

//NOTE(darius) kinda cool, kinda cringe. check this - https://serverfault.com/questions/503721/replacing-dll-files-while-the-application-is-running
									//   also check this - https://www.youtube.com/watch?v=WMSBRk5WG58&ab_channel=MollyRocket
									//   TODO(darius) generalize this for any .dll?(xinput, etc)
void DllScriptRoutine::loadDll(std::string_view path)
{
	boost::dll::fs::path lib_path(path);


	plugin = boost::dll::import_symbol<ScriptRoutine>(
		lib_path / "EmptyLogic",
		"script",
		boost::dll::load_mode::append_decorations
	);

	auto pluginCopy = plugin;

	do {
		try {
			plugin = boost::dll::import_symbol<ScriptRoutine>(
				lib_path / "EngineLogic",
				"script",
				boost::dll::load_mode::append_decorations
			);
		}
		catch (...)
		{

		}
	} while (plugin == pluginCopy);


	if (instance)
		plugin->setInstance(instance);

	time = checkTimeOfCreation();
}

void DllScriptRoutine::update(ScriptArgument& arg) {
	if (checkTimeOfCreation() != time) 
	{
		loadDll(path);
	
		return;
	}
	plugin->update(arg);
}

void DllScriptRoutine::start(ScriptArgument& arg) {
	plugin->start(arg);
}

//TODO(darius) make own wrapper?
long DllScriptRoutine::checkTimeOfCreation() 
{
	 struct stat result;
	 if (stat("E:/own/programming/engine/logicScripts/EngineLogic/x64/Debug/EngineLogic.dll", &result) == 0)
		 return (long)(result.st_mtime);
	 return 0;
}

std::string_view DllScriptRoutine::getPath()
{
	return path;
}

