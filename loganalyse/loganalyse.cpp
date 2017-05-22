// loganalyse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>     // std::cout
#include <fstream> 
#include <string.h>
#include <sstream>

using namespace std;

class Time
{
private:
	unsigned int hh, mm, ss, ooo;

public:
	unsigned int mss;
	void convertIntomillianSeconds(void);
	string convertTohhmmss();
	Time(string t);
};

void Time::convertIntomillianSeconds(void)
{
	mss = (hh * 3600 + mm * 60 + ss) * 1000 + ooo;
}

string Time::convertTohhmmss()
{
	stringstream stream;

	stream << hh << ':' << mm << ':' << 'ss' << '.' << ooo << std::endl;

	return stream.str();
}

Time::Time(string t)
{
	string s;
	s = t.substr(0, 2);
	hh = atoi(s.c_str());

	s = t.substr(3, 2);
	mm = atoi(s.c_str());

	s = t.substr(6, 2);
	ss = atoi(s.c_str());

	s = t.substr(9, 3);
	ooo = atoi(s.c_str());

	convertIntomillianSeconds();
}

// t2 - t1
// format: hh:mm:ss.000
string calculat_time(string t1, string t2)
{
	stringstream stream;

	Time ta(t1);
	Time tb(t2);

	int mss = ta.mss - tb.mss;
	int hh = mss / 3600000;
	int mm = (mss - hh * 3600000) / 60000;
	int ss = (mss - hh * 3600000 - mm * 60000) / 1000;
	int ms = mss - hh * 3600000 - mm * 60000 - ss * 1000;

	stream << hh << ':' << mm << ':' << ss << '.' << ms;

	return stream.str();
}


int main()
{
	std::string line;
	std::ifstream file;
	char filename[100];
	string stime("Time");
	string sBail("Bail number");
	string sInput00X11("Inputs:00X11");
	string sInput00X00("Inputs:00X00");
	string sSensorsTime("Sensors time(ms)");
	string sBailassignTime("Bail processing time(ms)");
	string bail_num("");

	bool restart = false;  // after reallocation, restart sets to true, the bail_num will be re-counted

	string time11, time00, time;

	string last_time("00:00:00.000");

	std::ofstream output_file;

	std::cout << "file name:  ";

	scanf_s("%s", filename, 100);

	// open file
	file.open(filename);

	output_file.open("analyse.txt");

	output_file << "analyse result by " << filename << std::endl;
	output_file << '\n' << std::endl;
	output_file << stime << '\t' << '\t' << sBail << '\t' << '\t' <<
		sInput00X11 << '\t' << '\t' << sInput00X00 << '\t' << '\t' << sSensorsTime << '\t' << '\t' << sBailassignTime << std::endl;

	// read line
	while (std::getline(file, line))
	{
		std::istringstream iss(line);

		string a;
		int i = 0;

		std::size_t found = line.find("Saber.Shed.Common.Udp.UdpLogListener");

		if (found == std::string::npos)
		{
			continue;
		}

		while(iss >> a)
		{
			i++;
			if (i == 1)
			{
				time = a;
			}
			else if (i == 5)
			{
				found = a.find("Inputs:");
				if (found != std::string::npos)
				{
					string b;
					iss >> b;
					if (b.at(0) == '1')
					{
						output_file << time << '\t' << "Full prox" << std::endl;
					}
					else if (b.at(1) == '1')
					{
						output_file << time << '\t' << "Half prox" << std::endl;
					}

					if (b.at(3) == '1' && b.at(4) == '1')    // all block
					{
						time11 = a.substr(0, 12);
					}
					else if (b.at(3) == '0' && b.at(4) == '0')          // all clear
					{
						time00 = a.substr(0, 12);
					}
					break;
				}

				if (restart == false)
				{
					found = a.find("assigned");
					if (found != std::string::npos)
					{
						string f;
						iss >> f;
						found = f.find(bail_num);
						if (found != std::string::npos)
						{
							output_file << time << '\t' << bail_num << '\t' << '\t' << '\t' << time11 << '\t' << '\t' << time00 << '\t' << '\t' << '\t' << calculat_time(time00, time11)
								<< '\t' << '\t' << calculat_time(time, last_time) << std::endl;
							last_time = time;
						}
						break;
					}
				}

				found = a.find("Reverse");
				if (found != std::string::npos)
				{
					output_file << time << '\t' << "platform reversed" << std::endl;
					break;
				}

			}
			else if (i == 6)
			{
				found = a.find("has");
				if (found != std::string::npos)
				{
					iss >> a;
					found = a.find("stopped");
					if (found != std::string::npos)
					{
						output_file << time << '\t' << "platform stopped" << std::endl;
						break;
					}
				}
			}
			else if (i == 7)
			{
				if (strcmp(a.c_str(), "Movement") == 0)
				{
					string h, i, j, k, l;
					iss >> h >> i >> j >> k >> l;
					if (strcmp(h.c_str(), "to") != 0)
					{
						break;
					}
					bail_num = k.substr(5, 2);
					restart = false;
					//output_file << time << '\t' << bail_num << std::endl;
					break;
				}

				found = a.find("Reallocating");
				if (found != std::string::npos)
				{
					output_file << time << '\t' << "Reallocation" << std::endl;
					restart = true;
					break;
				}
			}
		}
	}

    return 0;
}

