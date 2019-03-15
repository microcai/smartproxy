
#pragma once

class doubleenteravoider
{
public:
	doubleenteravoider(bool& flager)
		: flager(flager)
		{
			if (flager)
			{
				can_enter = false;
			}
			else
			{
				flager = can_enter = true;
			}
		}

	~doubleenteravoider()
	{
		if (can_enter)
			flager = false;
	}

	operator bool () const{
		return can_enter;
	}

	bool& flager;
	bool can_enter;
};
