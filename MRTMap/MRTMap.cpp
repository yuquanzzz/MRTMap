// MRTMap.cpp
//
#include <algorithm>    // ONLY used for converting upper/lowercase string
#include <sstream>		// string stream
#include <fstream>		// file io
#include <stdio.h>
#include <string>
#include "Connection.h"
#include "Station.h"
#include "Line.h"
#include "List.h"
#include "Dictionary.h"
#include "Dictionary_Station.h"
#include "Dictionary_Line.h"
#include "List_New.h"
#include "Graph.h"

// global variables
List_New<Station> stnList;
Dictionary_Station stnNameToStationDict;
Dictionary_Line stnLineToLineDict;
Dictionary stnCodeToStnNameDict;
Dictionary stnCodeInitialToLineNameDict;

// converts a given string to lowercase
string toLowercase(string s)
{
	for_each(s.begin(), s.end(), [](char& c) {
		c = tolower(c);
		});
	return s;
}

// fare price
string fare(int i)
{
	float dist;
	dist = (float)i / 1000;

	if (dist < 3.3){ return "$1.70"; }
	else if (dist < 6.3){ return "$1.90"; }
	else if (dist < 9.3){ return "$2.10"; }
	else if (dist < 11.3){ return "$2.30"; }
	else if (dist < 15.3){ return "$2.50"; }
	else if (dist < 19.3){ return "$2.60"; }
	else if (dist < 23.3){ return "$2.70"; }
	else{ return "$2.80"; }
}

// startup procedure
void startup()
{
	string stnCode, stnName, stnLineName, dist, line;
	List stnCodeList;
	List_Station stnLineList;
	ifstream f;
	int lineNum = 1;

	/* We use the data in stations.csv to convert station codes to names,
	as well as converting station names to station objects. We will also
	add station objects to a list with all stations of the network */

	// open Stations.csv file
	f.open("Stations.csv");

	// get each line of file
	while (getline(f, line))
	{
		// split into station code and station name
		istringstream s(line);
		getline(s, stnCode, ',');
		getline(s, stnName, ',');

		// add stnCode (key) and stnName (item) to dictionary
		stnCodeToStnNameDict.add(stnCode, stnName);

		// add stnNameLowercase (key) and Station (item) to dictionary

		// check if station in dictionary
		if (stnNameToStationDict.get(toLowercase(stnName)) == nullptr) // not found
		{
			Station* newStn = new Station(stnName, stnCode);	// create new station
			stnNameToStationDict.add(toLowercase(stnName), newStn);	// add station to dictionary
			stnList.add(newStn);	// add station to station list
		}
		else // found
		{
			Station* existingStn = stnNameToStationDict.get(toLowercase(stnName)); // get existing station
			existingStn->addCode(stnCode); // add in new station code
		}
	}

	f.close();

	/* We will use Routes.csv to associate stations to specific lines,
	and to add the connections (distances) to the stations,
	and associating initials of a station (e.g. CC) to a certian line */

	// open Routes.csv
	f.open("Routes.csv");

	while (getline(f, line))
	{
		if (lineNum % 2 == 1) // station line name and station codes line
		{
			// split into station line name and station code
			istringstream s(line);
			getline(s, stnLineName, ',');

			// check if station line in directory

			if (stnLineToLineDict.get(toLowercase(stnLineName)) == nullptr) // not found
			{
				Line* newStnLine = new Line(stnLineName); // create new station line
				stnLineToLineDict.add(toLowercase(stnLineName), newStnLine); // add station line to dictionary
			}
			Line* stnLine = stnLineToLineDict.get(toLowercase(stnLineName)); // get existing station line

			// add stations to a line
			while (getline(s, stnCode, ','))
			{
				stnName = stnCodeToStnNameDict.get(stnCode); // get station name using station code
				Station* stn = stnNameToStationDict.get(toLowercase(stnName));	// get station object with station name
				stnLine->add(stn); // add station to station line
				stnLineList.add(stn); // add line of stations to a list for adding connection later
				stnCodeList.add(stnCode.substr(0, 2));	// add initial (e.g CC) to the list for adding connection later
				stnCodeInitialToLineNameDict.add(stnCode.substr(0, 2), stnLineName); // associate initial (e.g CC) to station line names
			}
		}
		else // distance between stations line
		{
			// split into different distances
			istringstream s(line);

			// connection of first station
			Station* firstStn = stnLineList.get(0); // get first station
			getline(s, dist, ',');
			Connection* firstConn = new Connection(stnLineList.get(1), stoi(dist), stnCodeList.get(1)); // create new connection with front station
			firstStn->addConnection(firstConn); // add connection to first station

			// connections of stations in between
			for (int i = 1; i < stnLineList.getLength()-1; ++i)
			{
				Station* bwStn = stnLineList.get(i); // stations in between
				Connection* frontConn = new Connection(stnLineList.get(i - 1), stoi(dist), stnCodeList.get(i - 1)); // create new connection with prev station
				bwStn->addConnection(frontConn); // add connection
				getline(s, dist, ','); // get distance to next station
				Connection* backConn = new Connection(stnLineList.get(i + 1), stoi(dist), stnCodeList.get(i + 1)); // create new conneciton with nxt station
				bwStn->addConnection(backConn); // add connection
			}
			
			// connection of last station
			Station* lastStn = stnLineList.get(stnLineList.getLength() - 1); // get last stations
			Connection* lastConn = new Connection(stnLineList.get(stnLineList.getLength()-2), stoi(dist), stnCodeList.get(stnCodeList.getLength() - 2));
			lastStn->addConnection(lastConn);

			// clear stnLineList & stnCodeList for next line
			for (int i = stnLineList.getLength(); i > 0; --i)
			{
				stnLineList.remove(i - 1);
				stnCodeList.remove(i - 1);
			}
		}
		lineNum++;
	}

	f.close();
}

//void addToCSV()
//{
//	string line, code, stnCode, name, stationLineName, line1, stationline, frontcode, backcode, frontdist, backdist, station, dist;
//	List row, row2, lineList;
//	int iterator = 1;
//
//	ifstream originalCSV;
//	ofstream replacementCSV;
//	originalCSV.open("Routes.csv");
//	replacementCSV.open("temp.csv", ios::in);
//
//	while (getline(originalCSV, line))
//	{
//		if (iterator % 2 == 1) // station codes
//		{
//			istringstream ss(line);
//			getline(ss, stationLineName, ',');
//			if (stationLineName == stationline) { //So that we know which line to add the station to
//				cout << stationLineName << endl;
//
//				//for (int i = 0; i < )
//			}
//		}
//		else // distances
//		{
//			istringstream ss(line);
//			while (getline(ss, dist, ','))
//			{
//				row2.add(dist);
//			}
//			// add line to dictionary
//		}
//
//	}
//}

// add stations to routes.csv file function
void addStationCsv()
{
	 
}


int main()
{
	/* startup procedure to load csv files to respective data structures */
	startup();

	/* load train network into graph structure (adjacency list) */
	graph map(&stnList);

	/* used for the various options */
	int option, optionTwo, optionThree;
	int* shortestDist = new int;
	string optionStr;
	string stnName, stnCode, stnLineName, stnConnectCode, stnConnectCode2, startStn, endStn;
	Station* stn;
	Station* stnConnect;
	Station* stnConnect2;
	Connection* newConn;
	Connection* oldConn;
	Line* stnLine;
	List stnLineNames;
	List_Connection connList, connList2;
	List_New<Station> shortestPath;

	// main program
	while (true)
	{
		system("cls"); // clear console
		// MAIN MENU
		cout << "\nMRTMap MENU\n---------------------------\n";
		cout << "[1] Display all stations in a given line\n[2] Display station information\n[3] Add a new station on a given line\n";
		cout << "[4] Display the shortest route and its price, given the source and destination\n[5] Create a line\n";
		cout << "[0] Exit program\n";
		cout << "---------------------------\nSelect an option: ";
		cin >> option;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		// functions
		switch (option)
		{
		case 0:	// exit program
			return 0;
		case 1: // display all stations in a given line

			// get names of all existing lines on the network
			stnLineNames = stnLineToLineDict.getLineNames();

			// print list of lines on the network
			cout << endl << "Station Line(s) on the network" << endl;
			for (int i = 0; i < stnLineNames.getLength(); ++i)
			{
				cout << "[" << i + 1 << "] " << stnLineNames.get(i) << endl;
			}

			// user chooses a line to display from the list
			cout << endl << "Please select the line to be displayed";
			cout << endl << "Enter a number : ";
			cin >> optionTwo;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			// check if option valid
			if (optionTwo > stnLineNames.getLength() || optionTwo < 1)
			{
				cout << endl << "ERROR : Invalid number" << endl;
				system("pause");
				break;
			}

			// print out the station line and its stations
			system("cls"); // clear console
			stnLine = stnLineToLineDict.get(toLowercase(stnLineNames.get(optionTwo - 1)));
			stnLine->print(2);
			system("pause");
			break;
		case 2: // display station information

			// ask user for station to display
			cout << "Enter the station name : ";
			getline(cin, stnName);

			// print out the station details
			system("cls"); // clear console
			stn = stnNameToStationDict.get(toLowercase(stnName));
			if (stn == nullptr)
			{
				cout << endl << "ERROR : station not found" << endl;
				system("pause");
				break;
			}
			stn->print();
			system("pause");
			break;
		case 3: // add a new station on a given line

			// get names of all existing lines on the network
			stnLineNames = stnLineToLineDict.getLineNames();

			// print list of lines on the network
			cout << endl << "Station Line(s) on the network" << endl;
			for (int i = 0; i < stnLineNames.getLength(); ++i)
			{
				cout << "[" << i + 1 << "] " << stnLineNames.get(i) << endl;
			}

			// user chooses a line from the list
			cout << endl << "Please select the line for the new station to be in";
			cout << endl << "Enter a number : ";
			cin >> optionTwo;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			// check if option valid
			if (optionTwo > stnLineNames.getLength() || optionTwo < 1)
			{
				cout << endl << "ERROR : Invalid number" << endl;
				system("pause");
				break;
			}

			// print out line name
			system("cls"); // clear console
			stnLine = stnLineToLineDict.get(toLowercase(stnLineNames.get(optionTwo - 1)));
			cout << "Station Line : " << stnLine->Name() << endl;
			cout << "Note : Entering an existing station will add it to the new line" << endl;

			// ask user for new station name (or existing)
			cout << "Enter the station name : ";
			getline(cin, stnName);

			// check if station exists on the same line
			if (stnNameToStationDict.get(toLowercase(stnName)) != nullptr) // station exists
			{
				// get station object
				stn = stnNameToStationDict.get(toLowercase(stnName));
				if (stnLine->existingStn(stn)) // station exists on same line
				{
					cout << "ERROR : Station already exist on line" << endl;
					system("pause");
					break;
				}

				// print line name and station name
				system("cls"); // clear console
				cout << "Station Line : " << stnLine->Name() << endl;
				cout << "Station : ";
				stn->printMin();
			}
			else // station does not exist
			{

				// print line name and station name
				system("cls"); // clear console
				cout << "Station Line : " << stnLine->Name() << endl;
				cout << "Station : " << stnName << endl;
			}

			// ask user for new station code
			cout << "Enter the new station code : ";
			getline(cin, stnCode);

			// check if station code in use
			if (stnCodeToStnNameDict.get(stnCode) != "")
			{			
				cout << "ERROR : Station code already in use" << endl;
				system("pause");
				break;
			}

			// check if initials of station code used by other lines
			if (stnCodeInitialToLineNameDict.get(stnCode.substr(0, 2)) != "")
			{
				if (stnCodeInitialToLineNameDict.get(stnCode.substr(0, 2)) != stnLine->Name())
				{
					cout << "ERROR : Station code initials used by another Line" << endl;
					system("pause");
					break;
				}
			}
			else // not used by other lines, add it to dictionary
			{
				stnCodeInitialToLineNameDict.add(stnCode.substr(0, 2), stnLine->Name());
			}
			
			// check if there are more than one station on the line
			if (stnLine->Stations().getLength() > 0)
			{

				// display current stations on the line
				stnLine->print(3);

				// ask user for which existing station for new station to be connected to
				cout << "Enter a number for the new station to be connected to : ";
				cin >> optionTwo;
				cin.ignore(numeric_limits<streamsize>::max(), '\n');

				// check if number valid
				if (optionTwo > stnLine->Stations().getLength() || optionTwo < 1)
				{
					cout << endl << "ERROR : Invalid number" << endl;
					system("pause");
					break;
				}

				// get existing station for new station to be connected to
				stnConnect = stnLine->Stations().get(optionTwo - 1);

				// get the correct station code for the selected station to connect
				for (int i = 0; i < stnConnect->Code().getLength(); ++i)
				{
					if (stnCodeInitialToLineNameDict.get(stnConnect->Code().get(i).substr(0, 2)) == stnLine->Name())
					{
						stnConnectCode = stnConnect->Code().get(i);
						break;
					}
				}

				// ask user for distance from new station to existing station
				cout << "Enter distance to the new station in metres : ";
				cin >> optionTwo;
				cin.ignore(numeric_limits<streamsize>::max(), '\n');

				// get list of connections of the selected station that are on the same line
				connList = stnConnect->Connections();
				for (int i = 0; i < connList.getLength(); ++i)
				{
					if (stnCodeInitialToLineNameDict.get(connList.get(i)->LineCode()) == stnLine->Name())
					{
						connList2.add(connList.get(i));
					}
				}

				// if station selected has connections to other stations
				if (connList2.getLength() != 0)
				{
					// ask user if second station required
					cout << "Do you want to connect the new station to a second station? (y/n): ";
					cin >> optionStr;
					cin.ignore(numeric_limits<streamsize>::max(), '\n');

					// user wants second station
					if (optionStr == "y")
					{
						// print out stations that are connected to the existing station
						for (int i = 0; i < connList2.getLength(); ++i)
						{
							cout << "[" << i + 1 << "] ";
							connList2.get(i)->StationObj()->printMin();
						}

						// ask user for second existing station to be connected
						cout << "Enter a number for the new station to be connected to : ";
						cin >> optionThree;
						cin.ignore(numeric_limits<streamsize>::max(), '\n');

						// get second existing station for new station to be connected to
						oldConn = connList2.get(optionThree - 1);
						stnConnect2 = oldConn->StationObj();
						stnConnectCode2 = oldConn->LineCode();

						// ask user for distance from new station to second existing station
						cout << "Enter distance to the new station in metres : ";
						cin >> optionThree;
						cin.ignore(numeric_limits<streamsize>::max(), '\n');

						// delete old connections between the 2 existing stations
						stnConnect->removeConnection(oldConn);
						connList = stnConnect2->Connections();
						for (int i = 0; i < connList.getLength(); ++i)
						{
							if (connList.get(i)->StationObj() == stnConnect && connList.get(i)->LineCode() == stnConnectCode.substr(0, 2))
							{
								stnConnect2->removeConnection(connList.get(i));
								break;
							}
						}

						// check whether station already exists
						if (stnNameToStationDict.get(toLowercase(stnName)) != nullptr) // exists
						{
							// get station object
							stn = stnNameToStationDict.get(toLowercase(stnName));
						}
						else // new station
						{
							// create new station object
							stn = new Station(stnName, stnCode);
							// add station name (key) and station object (value) to dictionary
							stnNameToStationDict.add(toLowercase(stnName), stn);
							stnList.add(stn);	// add station to station list
						}

						// add new connection from new station to second existing station
						newConn = new Connection(stnConnect2, optionThree, stnConnectCode2.substr(0,2));
						stn->addConnection(newConn);

						// add new connection from second existing station to new station
						newConn = new Connection(stn, optionThree, stnCode.substr(0, 2));
						stnConnect2->addConnection(newConn);
					}
				}

				// add station code (key) and station name (value) into dictionary
				stnCodeToStnNameDict.add(stnCode, stnName);

				// check whether station already exists
				if (stnNameToStationDict.get(toLowercase(stnName)) != nullptr) // exists
				{
					// get station object
					stn = stnNameToStationDict.get(toLowercase(stnName));
					stn->addCode(stnCode); // add new station code
				}
				else // new station
				{
					// create new station object
					stn = new Station(stnName, stnCode); 
					// add station name (key) and station object (value) to dictionary
					stnNameToStationDict.add(toLowercase(stnName), stn);
					stnList.add(stn);	// add station to station list
				}

				// add new connection from new station to existing station
				newConn = new Connection(stnConnect, optionTwo, stnConnectCode.substr(0,2));
				stn->addConnection(newConn);

				// add new connection from existing station to new station
				newConn = new Connection(stn, optionTwo, stnCode.substr(0, 2));
				stnConnect->addConnection(newConn);
			}		

			// check whether station already exists
			if (stnNameToStationDict.get(toLowercase(stnName)) != nullptr) // exists
			{
				// get station object
				stn = stnNameToStationDict.get(toLowercase(stnName));
				stn->addCode(stnCode); // add new station code
			}
			else // new station
			{
				// create new station object
				stn = new Station(stnName, stnCode);
				// add station name (key) and station object (value) to dictionary
				stnNameToStationDict.add(toLowercase(stnName), stn);
				stnList.add(stn);	// add station to station list
			}
			stnLine->add(stn);	// add station to given line
			// reload graph
			map = graph(&stnList);

			// print line name and station information
			system("cls"); // clear console
			cout << "Station Line : " << stnLine->Name() << endl;
			cout << "Station : ";
			stn->printMin();
			system("pause");
			break;
		case 4: // display shortest route

			// ask user for starting and destination stations
			cout << "Enter starting station name : ";
			getline(cin, startStn);
			cout << "Enter destination station name : ";
			getline(cin, endStn);

			// check if station names are valid
			if (stnNameToStationDict.get(startStn) == nullptr || stnNameToStationDict.get(endStn) == nullptr)
			{
				cout << "ERROR : Station not found" << endl;
				system("pause");
				break;
			}

			// dijkstra's algorithm to find shortest path
			shortestPath = map.find_path(stnNameToStationDict.get(startStn), stnNameToStationDict.get(endStn), shortestDist);
			
			// print results
			system("cls");	// clear console
			for (int i = shortestPath.getLength(); i > 1; --i)
			{
				connList = shortestPath.get(i - 1)->Connections();
				for (int j = 0; j < connList.getLength(); ++j)
				{
					if (connList.get(j)->StationObj() == shortestPath.get(i - 2))
					{
						cout << connList.get(j)->LineCode() << "| ";
						break;
					}
				}
				shortestPath.get(i - 1)->printMin();
			}
			cout << " --> ";
			shortestPath.get(0)->printMin();
			cout << endl << "Note : Stations with * are interchanges" << endl;
			cout << endl << "Distance : " << (float)*shortestDist/1000 << "KM" << endl;
			cout << "Fare : " << fare(*shortestDist) << endl;
			*shortestDist = 0; // reset shortest distance

			system("pause");
			break;
		case 5: // add a new line

			// ask user for new line name
			cout << "Enter new line name : ";
			getline(cin, optionStr);

			// check if line name already exists
			if (stnLineToLineDict.get(toLowercase(optionStr)) != nullptr)
			{
				cout << "ERROR : Existing line name!" << endl;
				system("pause");
				break;
			}

			// add station line to dictionary
			stnLineToLineDict.add(toLowercase(optionStr), new Line(optionStr));
			break;
		default:
			cout << endl << "ERROR : Invalid option" << endl;
			system("pause");
		}
	}
}
