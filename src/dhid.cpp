#include <dseed.h>

dseed::mouse_state::mouse_state ()
	: position (int32_min, int32_min), wheel (0, 0)
	, buttons (mousebuttons_no_button)
	, is_relative (false)
{ }