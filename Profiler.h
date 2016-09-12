/*

#ifdef linux
#include <time.h>
void Sleep(int milliseconds)
{
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}
#endif

int main()
{	Profiler prof;

	for(;;)
	{	
		Sleep(50); // code, which does not need to measure performance

		prof(NULL);

		Sleep(100); // some code

		prof("code1");

		Sleep(200); // some code

		prof("code2");

		prof.periodic_dump(5);
			// every 5 seconds will print table
	}
	return 0;
}
*/

#include <stdint.h>
#include <stdio.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::map;

#include <algorithm>
using std::min;
using std::max;

#include <utility>
using std::make_pair;

#include <iostream>
using std::cout;

#ifdef linux

#include <sys/time.h>

static uint64_t microseconds()
{	timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

#else

#include <Windows.h>

static uint64_t microseconds()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return now.QuadPart * 1000 / (freq.QuadPart / 1000);
		// overflow occurs much later
}

#endif

class Profiler
{	
	class Event
	{
	public:
		uint64_t time;
		uint64_t count;
		uint64_t min_time;
		uint64_t max_time;

		void reset()
		{	time = 0;
			count = 0;
			min_time = (uint64_t)-1;
			max_time = 0;
		}
	};

	map<const char*, Event> events;

	uint64_t t0;
	uint64_t last_dump;

	map<const char*, Event>::iterator i;

public:
	vector<string> out;

	Profiler()
	{	last_dump = t0 = microseconds();
	}

	void operator()(const char* what)
	{	
		if (what == NULL)
		{
			t0 = microseconds();
			return;
		}
		
		uint64_t t = microseconds() - t0;

		i = events.find(what);
		
		if(i == events.end())
		{	
			Event e;
			e.reset();
			i = events.insert(make_pair(what, e)).first;
		}

		Event& e = (*i).second;

		e.time += t;
		e.min_time = min(e.min_time, t);
		e.max_time = max(e.max_time, t);
		++e.count;

		t0 = microseconds();
	}

	void dump()
	{
		out.clear();

		const float us_to_ms = 0.001f;
		
		float summ = 0;
		for (i = events.begin(); i != events.end(); ++i)
		{	
			Event& e = (*i).second;

			summ += (float)e.time;
		}

		if (summ == 0) return;

		summ *= us_to_ms;

		out.push_back("           name count   total(%)        min   avg   max\n");

		for(i = events.begin(); i != events.end(); ++i)
		{
			Event& e = (*i).second;

			if(e.count == 0) e.min_time = 0;

			float time = e.time * us_to_ms;
			float min_time = e.min_time * us_to_ms;
			float max_time = e.max_time * us_to_ms;
			float average = e.count == 0 ? 0 : time / (float)e.count;

			char tmp[0x100];

			snprintf(tmp, sizeof(tmp), "%15s %5llu %7.1f(%5.1f%%) %5.1f %5.1f %5.1f\n",
				i->first, (long long unsigned int)e.count,
				time, time / summ * 100, min_time, average, max_time);
			out.push_back(tmp);
			
			e.reset();
		}

		for (int i = 0; i < out.size(); ++i) cout << out[i];
	}

	void periodic_dump(unsigned int period)
	{	if(microseconds() < last_dump + period * 1000000) return;
		dump();
		last_dump = microseconds();
	}
};
