#include <SFML/Graphics.hpp>

using namespace sf;

View view; // объ€вили объект, который будет следить за игроком

View get_coord_for_view(float x, float y)
{
	float tmp_x = x, tmp_y = y;

	if (tmp_x < 600)
		tmp_x = 600;
	if (tmp_y < 300)
		tmp_y = 300;
	if (tmp_x > 1000)
		tmp_x = 1000;
	if (tmp_y > 500)
		tmp_y = 500;

	view.setCenter(tmp_x, tmp_y);
	return view;
}