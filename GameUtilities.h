#pragma once

/*
	Game specific utilities that maybe useful to mode/entities etc.
*/

/*
	Generic Input flag container
*/
struct InputFlags16
{
	InputFlags16()
		:action1(0), action2(0), action3(0), action4(0),
		action5(0), action6(0), action7(0), action8(0),
		action9(0), action10(0), action11(0), action12(0),
		up(0), down(0), left(0), right(0)
	{

	}

	//Generic actions (context defined)
	bool action1  : 1;
	bool action2  : 1;
	bool action3  : 1;
	bool action4  : 1;
	bool action5  : 1;
	bool action6  : 1;
	bool action7  : 1;
	bool action8  : 1;
	bool action9  : 1;
	bool action10 : 1;
	bool action11 : 1;
	bool action12 : 1;

	//Direction Reserved
	bool up       : 1;
	bool down     : 1;
	bool left     : 1;
	bool right    : 1;
};