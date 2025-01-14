#pragma once
#include <string>
#include "Engine/Camera.h"
#include "MouseState.h"
#include "KeyboardState.h"
#include <Core/Timer.h>

//TODO(darius) fix this bullshit
class GameState
{
public:
	void msg(std::string&& str);

	void clear_msg();

	static void set_instance(GameState* ptr);

	static GameState* get_instance();

	static void setEditorCameraMode();

	static void saveActiveCameraStateIntoEditorCamera();

	static void setCameraMotionPoints(const CameraPoints& points);
	
public:
	//TODO(darius) make it not static? See why below
	//TODO(darius) currently NO synchronization
	Camera cam;// active camera
	static Camera* editorCamera;
	static bool editorCameraMode;
	static CameraPoints cameraMotionPoints;
	static Timer currTime;

	static float gammaBrightness;
	static float gammaFactor;

	static MouseState ms;
	KeyboardState ks;
	static GameState* instance;
    static int editor_mode;
	static std::string engine_path;
	static bool cullEnabled;
	static bool shadowEnabled;

	//TODO(darius) make it a class
	std::string debug_msg;
	size_t debug_len = 0;
};

