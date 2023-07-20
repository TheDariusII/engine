#pragma once
#include <atomic>

class MouseState
{
public:
	void init(int x_pos = 430, int y_pos = 300, int x_off = 0, int y_off = 0, bool left_press = 0, bool right_press = 0);
	
	//TODO(darius) make macro for this shit
	void set_x(int x);

	void set_y(int y);
	
	void set_offset_x(int x);

	void set_offset_y(int y);

	void set_left(bool state);

	void set_right(bool state);

	int get_x();

	int get_y();

	bool get_left();

	bool get_right();

	int get_offset_x();

	int get_offset_y();

	int prev_x = 0;
	int prev_y = 0;

    int click_x = 0;
    int click_y = 0;

	int cursor_x = 0;
	int cursor_y = 0;

private:
	//don need atomics at all?..
	std::atomic_int curr_x;
	std::atomic_int curr_y;
	std::atomic_int offset_x;
	std::atomic_int offset_y;
	std::atomic_bool left_pressed;
	std::atomic_bool right_pressed;
};

