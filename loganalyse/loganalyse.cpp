// loganalyse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>     // std::cout
#include <fstream> 
#include <string.h>
#include <sstream>
#include <Windows.h>

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
	string feedanimal = "cow=EMPTY ";

	bool restart = false;  // after reallocation, restart sets to true, the bail_num will be re-counted

	string time11, time00, time, FeedRelayOn[5], FeedRelayOff[5], ExpectedRelayT[5];

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
		sInput00X11 << '\t' << '\t' << sInput00X00 << '\t' << '\t' << sSensorsTime << '\t' << '\t' << sBailassignTime << '\t' << "FeedorNot" << '\t' << "Head1 Relay On" << '\t' << "Head2 Relay On" << std::endl;

	// read line
	while (std::getline(file, line))
	{
		std::istringstream iss(line);

		string a;
		int i = 0;

		std::size_t found = line.find("Saber.Shed.Id.Rotary.Mediator.Handlers.UdpLogMessageReceivedHandler");
		std::size_t foundMCT = line.find("Inputs: MCT");
		std::size_t foundReallocation = line.find("Reallocating Bails: occurrences");
		
		if (foundReallocation != std::string::npos)
		{
			output_file << "Reallocation happened" << std::endl;			
		}
		
		if (found == std::string::npos)
		{
			continue;
		}

		if (foundMCT != std::string::npos)
		{
			output_file << "MCT happened" << std::endl;
			continue;
		}
				
		while(iss >> a)
		{
			i++;
			if (i == 5)
			{
				time = a;
			}
			else if (i == 6)
			{
#if 1
				found = a.find("Inputs:");
				if (found != std::string::npos)
				{
					string b;
					iss >> b;
					if (b.at(0) == '1')
					{
						//output_file << time << '\t' << "Full prox" << std::endl;
					}
					else if (b.at(1) == '1')
					{
						//output_file << time << '\t' << "Half prox" << std::endl;
					}

					if (b.at(3) == '1' && b.at(4) == '1')    // all block
					{
						//time11 = a.substr(0, 12);
						time11 = time;
					}
					else if (b.at(3) == '0' && b.at(4) == '0')          // all clear
					{
						//time00 = a.substr(0, 12);
						time00 = time;
					}
					break;
				}

				if (restart == false)
				{
					found = a.find("assigned");
					if (found != std::string::npos)
					{
						string f, g, h;
						iss >> f >> g >> h;
						found = f.find(bail_num);
						if (found != std::string::npos && (strcmp(feedanimal.c_str(), "cow=EMPTY ") == 0 || strcmp(feedanimal.c_str(), "repeat") == 0 || strcmp(feedanimal.c_str(), "") == 0))
						{
							output_file << time << '\t' << bail_num << '\t' << '\t' << '\t' << time11 << '\t' << '\t' << time00 << '\t' << '\t' << '\t' << calculat_time(time00, time11)
								<< '\t' << '\t' << calculat_time(time, last_time) << '\t' << g << ' ' << h << '\t' << feedanimal << std::endl;
							last_time = time;

							feedanimal = "";
						}
						else if (found != std::string::npos)
						{
							output_file << time << '\t' << bail_num << '\t' << '\t' << '\t' << time11 << '\t' << '\t' << time00 << '\t' << '\t' << '\t' << calculat_time(time00, time11)
								<< '\t' << '\t' << calculat_time(time, last_time) << '\t' << g << ' ' << h << '\t' << feedanimal << '\t';
							last_time = time;

							feedanimal = "";
						}
						break;
					}
				}

				found = a.find("Reverse");
				if (found != std::string::npos)
				{
					if (strcmp(FeedRelayOn[0].c_str(), "") != 0)
					{
						output_file << FeedRelayOn[0] << '\t' << FeedRelayOff[0] << '\t' << atoi(FeedRelayOff[0].c_str()) - atoi(FeedRelayOn[0].c_str()) <<  '\t'
							<< FeedRelayOn[1] << '\t' << FeedRelayOff[1] << '\t' << atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << std::endl;
						FeedRelayOn[0] = "";
						FeedRelayOff[0] = "";
						FeedRelayOn[1] = "";
						FeedRelayOff[1] = "";
					}
					output_file << time << '\t' << "platform reversed" << std::endl;
					break;
				}

				found = a.find("Not");
				if (found != std::string::npos)
				{
					string h, i, j;
					iss >> h >> i >> j;
					if (strcmp(h.c_str(), "feed") == 0 && strcmp(i.c_str(), "empty") == 0)
					{
						feedanimal = "cow=EMPTY ";
					}
					break;
				}

				found = a.find("Feed:");
				if (found != std::string::npos)
				{
					string h, i, j;
					iss >> h >> i >> j;
					if (strcmp(h.c_str(), "Not") == 0 && strcmp(i.c_str(), "feed") == 0 && strcmp(j.c_str(), "repeat") == 0)
					{
						feedanimal = j;
					}
					break;
				}
#endif
				found = a.find("Not");
				if (found != std::string::npos)
				{
					string h, i, j;
					iss >> h >> i >> j;
					if (strcmp(h.c_str(), "feed") == 0 && strcmp(i.c_str(), "empty") == 0)
					{
						feedanimal = "cow=EMPTY ";
						output_file << feedanimal << '\t'
							<< "Empty" << std::endl;
					}
					break;
				}

				found = a.find("Feed:");
				if (found != std::string::npos)
				{
					string h, i, j;
					iss >> h >> i >> j;
					if (strcmp(h.c_str(), "Not") == 0 && strcmp(i.c_str(), "feed") == 0 && strcmp(j.c_str(), "repeat") == 0)
					{
						feedanimal = j;
						output_file << feedanimal << '\t'
							<< "Repeat" << std::endl;
					}
					break;
				}

				found = a.find("Feed");
				if (found != std::string::npos)
				{
					string h, i, j, k;
					iss >> h >> i >> j >> k;
					found = h.find("cow=");
					if (found != std::string::npos)
					{
						for (int xj = 0; xj < 5; xj++)
							FeedRelayOff[xj] = "0";

						if (strcmp(h.c_str(), "cow=UNKNOWN") == 0)
						{
							feedanimal = "cow=UNKNOWN ";
							if (strcmp(i.c_str(), "head[1]") == 0)
							{
								string l;
								iss >> l;
								if (strcmp(j.c_str(), "off") != 0) {
									FeedRelayOn[0] = l.substr(1, l.size() - 3);
									ExpectedRelayT[0] = k.substr(0, k.size() - 2);
								}
								else
								{
									FeedRelayOn[0] = "0";
									ExpectedRelayT[0] = "0";
								}
							}
							else if (strcmp(i.c_str(), "head[2]") == 0)
							{
								string l;
								iss >> l;
								if (strcmp(j.c_str(), "off") != 0) {
									FeedRelayOn[1] = l.substr(1, l.size() - 3);
									ExpectedRelayT[1] = k.substr(0, k.size() - 2);
								}
								else
								{
									FeedRelayOn[1] = "0";
									ExpectedRelayT[1] = "0";
								}
							}
							else if (strcmp(i.c_str(), "head[3]") == 0)
							{
								string l;
								iss >> l;
								if (strcmp(j.c_str(), "off") != 0) {
									FeedRelayOn[2] = l.substr(1, l.size() - 3);
									ExpectedRelayT[2] = k.substr(0, k.size() - 2);
								}
								else
								{
									FeedRelayOn[2] = "0";
									ExpectedRelayT[2] = "0";
								}
							}
							else if (strcmp(i.c_str(), "head[4]") == 0)
							{
								string l;
								iss >> l;
								if (strcmp(j.c_str(), "off") != 0) {
									FeedRelayOn[3] = l.substr(1, l.size() - 3);
									ExpectedRelayT[3] = k.substr(0, k.size() - 2);
								}
								else
								{
									FeedRelayOn[3] = "0";
									ExpectedRelayT[3] = "0";
								}
							}
							else if (strcmp(i.c_str(), "head[5]") == 0)
							{
								string l;
								iss >> l;
								if (strcmp(j.c_str(), "off") != 0) {
									FeedRelayOn[4] = l.substr(1, l.size() - 3);
									ExpectedRelayT[4] = k.substr(0, k.size() - 2);
								}
								else
								{
									FeedRelayOn[4] = "0";
									ExpectedRelayT[4] = "0";
								}
							}
						}
						else
						{
							feedanimal = h + " " + i;
							if (strcmp(j.c_str(), "head[1]") == 0)
							{
								string l, m;
								iss >> l >> m;
								if (strcmp(k.c_str(), "off") != 0)
								{
									FeedRelayOn[0] = m.substr(1, m.size() - 3);
									ExpectedRelayT[0] = l.substr(0, l.size() - 2);
								}
								else
								{
									FeedRelayOn[0] = "0";
									ExpectedRelayT[0] = "0";
								}
							}
							else if (strcmp(j.c_str(), "head[2]") == 0)
							{
								string l, m;
								iss >> l >> m;
								if (strcmp(k.c_str(), "off") != 0) {
									FeedRelayOn[1] = m.substr(1, m.size() - 3);
									ExpectedRelayT[1] = l.substr(0, l.size() - 2);
								}
								else {
									FeedRelayOn[1] = "0";
									ExpectedRelayT[1] = "0";
								}

							}
							else if (strcmp(j.c_str(), "head[3]") == 0)
							{
								string l, m;
								iss >> l >> m;
								if (strcmp(k.c_str(), "off") != 0) {
									FeedRelayOn[2] = m.substr(1, m.size() - 3);
									ExpectedRelayT[2] = l.substr(0, l.size() - 2);
								}
								else {
									FeedRelayOn[2] = "0";
									ExpectedRelayT[2] = "0";
								}
							}
							else if (strcmp(j.c_str(), "head[4]") == 0)
							{
								string l, m;
								iss >> l >> m;
								if (strcmp(k.c_str(), "off") != 0) {
									FeedRelayOn[3] = m.substr(1, m.size() - 3);
									ExpectedRelayT[3] = l.substr(0, l.size() - 2);
								}
								else {
									FeedRelayOn[3] = "0";
									ExpectedRelayT[3] = "0";
								}
							}
							else if (strcmp(j.c_str(), "head[5]") == 0)
							{
								string l, m;
								iss >> l >> m;
								if (strcmp(k.c_str(), "off") != 0) {
									FeedRelayOn[4] = m.substr(1, m.size() - 3);
									ExpectedRelayT[4] = l.substr(0, l.size() - 2);
								}
								else {
									FeedRelayOn[4] = "0";
									ExpectedRelayT[4] = "0";
								}
							}
						}
					}
					break;
				}

				found = a.find("feed");
				if (found != std::string::npos)
				{
					string h, i, j, k;
					iss >> h >> i >> j >> k;
					bool printornot = true;

					if (strcmp(i.c_str(), "1") == 0)
					{
						FeedRelayOff[0] = k.substr(1, k.size() - 3);						
					}
					else if (strcmp(i.c_str(), "2") == 0)
					{
						FeedRelayOff[1] = k.substr(1, k.size() - 3);						
					}
					else if (strcmp(i.c_str(), "3") == 0)
					{
						FeedRelayOff[2] = k.substr(1, k.size() - 3);
						//output_file << FeedRelayOff[1] << '\t' << "(" << atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << ")" << std::endl;
					}
					else if (strcmp(i.c_str(), "4") == 0)
					{
						FeedRelayOff[3] = k.substr(1, k.size() - 3);
						//output_file << FeedRelayOff[1] << '\t' << "(" << atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << ")" << std::endl;
					}
					else if (strcmp(i.c_str(), "5") == 0)
					{
						FeedRelayOff[4] = k.substr(1, k.size() - 3);						
					}

					for (int xj = 0; xj < 5; xj++)
					{
						if (strcmp(ExpectedRelayT[xj].c_str(), "0") != 0 && strcmp(FeedRelayOff[xj].c_str(), "0") == 0)
						{
							printornot = false;
							break;
						}
					}

					if (printornot == true)
					{
						//unknown cow: one more Tab
						if (strcmp(feedanimal.c_str(), "cow=UNKNOWN") == 0)
						{
							output_file << feedanimal << '\t'<<'\t'
								<< atoi(ExpectedRelayT[0].c_str()) << '\t'
								<< atoi(FeedRelayOff[0].c_str()) - atoi(FeedRelayOn[0].c_str()) << '\t'
								<< atoi(ExpectedRelayT[1].c_str()) << '\t'
								<< atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << '\t'
								<< atoi(ExpectedRelayT[2].c_str()) << '\t'
								<< atoi(FeedRelayOff[2].c_str()) - atoi(FeedRelayOn[2].c_str()) << '\t'
								<< atoi(ExpectedRelayT[3].c_str()) << '\t'
								<< atoi(FeedRelayOff[3].c_str()) - atoi(FeedRelayOn[3].c_str()) << '\t'
								<< atoi(ExpectedRelayT[4].c_str()) << '\t'
								<< atoi(FeedRelayOff[4].c_str()) - atoi(FeedRelayOn[4].c_str()) << std::endl;
						}
						else
						{
							output_file << feedanimal << '\t'
								<< atoi(ExpectedRelayT[0].c_str()) << '\t'
								<< atoi(FeedRelayOff[0].c_str()) - atoi(FeedRelayOn[0].c_str()) << '\t'
								<< atoi(ExpectedRelayT[1].c_str()) << '\t'
								<< atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << '\t'
								<< atoi(ExpectedRelayT[2].c_str()) << '\t'
								<< atoi(FeedRelayOff[2].c_str()) - atoi(FeedRelayOn[2].c_str()) << '\t'
								<< atoi(ExpectedRelayT[3].c_str()) << '\t'
								<< atoi(FeedRelayOff[3].c_str()) - atoi(FeedRelayOn[3].c_str()) << '\t'
								<< atoi(ExpectedRelayT[4].c_str()) << '\t'
								<< atoi(FeedRelayOff[4].c_str()) - atoi(FeedRelayOn[4].c_str()) << std::endl;
						}
					}
				}

			}
#if 1
			else if (i == 7)
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
					break;
				}

				found = a.find("FBM");
				if (found != std::string::npos)
				{
					iss >> a;
					found = a.find("TCP");
					if (found != std::string::npos && strcmp(FeedRelayOn[0].c_str(), "") != 0)
					{
						//output_file << FeedRelayOn[0] << '\t' << FeedRelayOff[0] << '\t' << atoi(FeedRelayOff[0].c_str()) - atoi(FeedRelayOn[0].c_str()) << '\t'
						//	<< FeedRelayOn[1] << '\t' << FeedRelayOff[1] << '\t' << atoi(FeedRelayOff[1].c_str()) - atoi(FeedRelayOn[1].c_str()) << std::endl;
						FeedRelayOn[0] = "";
						FeedRelayOff[0] = "";
						FeedRelayOn[1] = "";
						FeedRelayOff[1] = "";
					}
				}				
			}
			else if (i == 8)
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
			}
#endif
		}
	}

    return 0;
}

