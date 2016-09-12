/*
Profiler prof;

for (;;)
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
*/

#include <stdint.h>
#include <stdio.h>

#include <string>
using std::string;

#include <set>
using std::set;

#include <algorithm>
using std::min;
using std::max;

#ifdef WIN32
#include <Windows.h>

class Microseconds
{
public:
	uint64_t operator()()
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		return now.QuadPart * 1000 / (freq.QuadPart / 1000);
			// overflow occurs much later
	}
};

#else
#include <sys/time.h>
//#include "android_workarround.h"
class Microseconds
{
public:
	uint64_t operator()()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
	}
};
#endif

class Profiler
{
	Microseconds microseconds;

	class Event
	{
		public:
		const char* name;
		uint64_t time;
		uint64_t count;
		uint64_t min_time;
		uint64_t max_time;

		void reset()
		{
			time = 0;
			count = 0;
			min_time = (uint64_t)-1;
			max_time = 0;
		}
	};

	class Comparator
	{
		public:
		bool operator()(const Event& a, const Event& b) const
		{	//return strcmp(a.name, b.name) < 0;
			return (void*)a.name < (void*)b.name;
		}
	};

	set<Event, Comparator> events;

	uint64_t t0;
	uint64_t last_dump;

	Event c;
	set<Event>::iterator i;

public:
	Profiler()
	{
		last_dump = t0 = microseconds();
	}

	void operator()(const char* what)
	{
		if (what == NULL)
		{
			t0 = microseconds();
			return;
		}

		uint64_t t = microseconds() - t0;

		c.name = what;
		i = events.find(c);

		if (i == events.end())
		{
			c.reset();
			i = events.insert(c).first;
		}

		Event& e = const_cast<Event&>(*i);

		e.time += t;
		e.min_time = min(e.min_time, t);
		e.max_time = max(e.max_time, t);
		++e.count;

		t0 = microseconds();
	}

	void dump()
	{
		const float MS = 0.001f;

		float f_summ = 0;
		for (i = events.begin(); i != events.end(); ++i)
			f_summ += (float)i->time;

		if (f_summ == 0) return;

		f_summ *= MS;
		f_summ *= .01f; // %

		printf("           name count   total(%%)        min   avg   max\n");

		for (i = events.begin(); i != events.end(); ++i)
		{
			Event& e = const_cast<Event&>(*i);

			if (e.count == 0) e.min_time = 0;

			float f_time = e.time * MS;
			float f_min = e.min_time * MS;
			float f_max = e.max_time * MS;
			float f_average = e.count == 0 ? 0 : f_time / (float)e.count;

			printf("%15s %5llu %7.1f(%5.1f%%) %5.1f %5.1f %5.1f\n",
				e.name, (long long unsigned int)e.count,
				f_time, f_time / f_summ, f_min, f_average, f_max);

			e.reset();
		}
	}

	void periodic_dump(unsigned int period)
	{
		if (microseconds() < last_dump + period * 1000000) return;
		dump();
		last_dump = microseconds();
	}
};
