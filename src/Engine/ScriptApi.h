#pragma once

#include <boost/dll/import.hpp>
#include <string_view>

//NOTE(darius) needed to check dll creation date
#include <sys/types.h>
#include <sys/stat.h>
#define stat _stat

class Scene;
class Object;
class Script;
class GameState;

struct ScriptArgument
{
	Scene* scene;
	Object* obj;
	Script* script; 
};

class ScriptRoutine
{
public:
    virtual void start(ScriptArgument& arg) = 0;
    virtual void update(ScriptArgument& arg) = 0;
	virtual void setInstance(GameState* p) = 0;
};

class DllScriptRoutine 
{
public:
    DllScriptRoutine(std::string_view path, GameState* inst);

    DllScriptRoutine(ScriptRoutine* routine) : plugin(routine){}

	DllScriptRoutine() = default;

	void loadDll(std::string_view path);

    void update(ScriptArgument& arg);

	void start(ScriptArgument& arg);

	long checkTimeOfCreation();

	std::string_view getPath();

private:
	GameState* instance = nullptr;
    boost::shared_ptr<ScriptRoutine> plugin;
	int time = 0;
	std::string path;
};
