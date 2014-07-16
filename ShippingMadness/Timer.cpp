

#include "Timer.h"

timer::timer()
{
	resetted=true;
	running=false;
	begin=0;
	end=0;

}

void timer::start()
{
	if(! running)
	{
		if(resetted)
		{
			begin=(unsigned long) clock();
		}
		else
		{
			begin -= (end - (unsigned long) clock() );
		}
		running=true;
		resetted=false;
	}
}

void timer::stop()
{
	if(running)
	{
		end= (unsigned long) clock();
		running=false;
	}
}

void timer::reset()
{
	bool wereRunning=running;
	if(wereRunning)
	{
		stop();
	}
	resetted=true;
	begin=0;
	end=0;
	if(wereRunning)
	{
		start();
	}
}

bool timer::isRunning()
{
	return running;
}

unsigned long timer::getTime()
{
	if(running)
	{
		return ( (unsigned long) clock() - begin) / CLOCKS_PER_SEC;
	}
	else
	{
		return end - begin;
	}
}

bool timer::isOver(unsigned long seconds)
{
	return seconds >= getTime();
}