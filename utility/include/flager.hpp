
#pragma once

class oncer_flag
{
public:
	bool if_first_run() const
	{
		if (first_run)
		{
			first_run = false;
			return true;
		}
		return first_run;
	}
private:
	mutable bool first_run = true;
};
