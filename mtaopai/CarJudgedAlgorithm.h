#pragma once

#include "mysql/include/mysql.h"
#include <iostream>
#include <boost/locale.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <string>
#include <map>
#include <algorithm>
#include <math.h>
#include <stdlib.h>

#define PI				(acos(-1))//圆周率
#define EarthRadius		6371004//地球半径
#define Threshold		25//m/s->90km/h

#define HOST "localhost"
#define USER "root"
#define PASSWORD "guo"
#define PORT 3306
#define MYDB "db_ga_viid"

#define AngleToArc(x)	(x*PI/180)



struct Coordinate {
	double longitude;				//经度
	double latitude;				//纬度
	time_t time;					//当前时间
	Coordinate() {};
	Coordinate(double lo, double la, time_t t) :longitude(lo), latitude(la), time(t) {};
	Coordinate(const char *lo, const char *la, const char *t) {
		longitude = atof(lo);
		latitude = atof(la);
		time = atoll(t);
	}
};

class CarJudgedAlgorithm
{
	typedef enum CarAlgorithm {
		fakeplate,
		GuanLian
	};
public:
	CarJudgedAlgorithm();
	CarJudgedAlgorithm(const char *json);
	CarJudgedAlgorithm(const char *host, const char *user, const char *password, unsigned int port, const char *database);
	CarJudgedAlgorithm(std::string host, std::string user, std::string password, unsigned int port, std::string database);
	CarJudgedAlgorithm(const char *host, const char *user, const char *password, unsigned int port, const char *database, const char *plateNo);
	CarJudgedAlgorithm(const char *host, const char *user, const char *password, unsigned int port, const char *database, const char * vColor, const char * vBrand, const char * st, const char * et);
	~CarJudgedAlgorithm();
	int test();
	MYSQL *ConnectDatabase(const char *host, const char *user, const char *password, unsigned int port, const char *database);
	int CloseDatabase(MYSQL *mysql);
	std::string GetResults();

private:
	/*std::string host;
	std::string user;
	std::string password;
	std::string database;
	int port;*/
	int FakePlateVehicles(const char *plateNo);
	int FakePlateVehicles(const char *vColor, const char *vBrand, const char *st, const char *et);
	bool IsFakePlateVehicle(Coordinate *path, unsigned int size);
	bool IsFakePlateVehicle(std::vector<Coordinate> &path);
	std::vector<std::string> IsFakePlateVehicle(std::map< std::string, std::vector<Coordinate> > &paths, std::vector<std::string> &plates);
	double distance(Coordinate A, Coordinate B);
	bool HasPlate(const std::vector<std::string> &plates, const std::string &plateno);
	void FindFakePlateVehicle(MYSQL_RES *results, MYSQL_FIELD fields,int rowcount, std::map<std::string, unsigned int> fieldsnameMap,std::string plateno);
	int EncapsulateFakePlatetoJson(const char *plateNo);
	int EncapsulateFakePlatetoJson(std::vector < std::string > fp);
	int EncapsulateJson(MYSQL_RES *results);

	
	inline std::string U8toGBK(std::string s) { return boost::locale::conv::between(s, "GBK", "UTF-8"); };
	inline std::string GBKtoU8(std::string s) { return boost::locale::conv::between(s, "UTF-8", "GBK"); };
	template<class T>
	inline T ** create2DArray(T **array2D, unsigned int range_x, unsigned int range_y)
	{
		T **arry2D = new T *[range_x];
		for (int i = 0; i < range_x; i++)
		{
			array2D[i] = new T[range_y];
		}
		for (int i = 0; i < range_x; i++)
		{
			for (int j = 0; j < range_y; j++)
			{
				array2D[i][j] = 0;
			}
		}
		return arry2D;
	}
	template<>
	inline Coordinate ** create2DArray(Coordinate **array2D, unsigned int range_x, unsigned int range_y)
	{
		Coordinate **arry2D = new Coordinate *[range_x];
		for (int i = 0; i < range_x; i++)
		{
			array2D[i] = new Coordinate[range_y];
		}
		for (int i = 0; i < range_x; i++)
		{
			for (int j = 0; j < range_y; j++)
			{
				array2D[i][j] = Coordinate("0","0","0");
			}
		}
		return arry2D;
	}
	template<class T>
	inline void delete2DArray(T ** a, unsigned int range_x)
	{
		for (int i = 0; i < range_x; i++)
		{
			delete[] a[i];
		}
		delete[] a;
		a = NULL;
	}



	MYSQL *mysql;
	MYSQL_RES *res;
	std::string json;
};

