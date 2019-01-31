#pragma once

#include "mysql/include/mysql.h"
#include <iostream>
#include <boost/locale.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/timer.hpp>
#include <boost/thread.hpp>
#include <string>
#include <map>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <thread>

#if 1
#define table_lane			std::string("Lane")
#define table_tollgate		std::string("Tollgate")
#define table_motorvehicle	std::string("MotorVehicle")
#define table_color			std::string("ColorType")
#define table_brand			std::string("VehicleBrandType")
#else
#define table_lane			std::string("lane")
#define table_tollgate		std::string("tollgate")
#define table_motorvehicle	std::string("motorVehicle")
#define table_color			std::string("colortype")
#define table_brand			std::string("vehiclebrandtype")
#endif

#define PI				(acos(-1))//圆周率
#define EarthRadius		6371004//地球半径
#define Threshold		25//m/s->90km/h

#define HOST "localhost"
#define USER "root"
#define PASSWORD "guo"
#define PORT 3306
#define MYDB "db_ga_viid"

#define CorrelationThresold		3
#define TrajectoryThresold		1

#define AngleToArc(x)	(x*PI/180)



struct TraceList {
	std::string appeartime;
	std::string tollgate;
	std::string longitude;
	std::string latitude;
	std::string image;
	TraceList() {};
	TraceList(const char *time, const char *tollgate, const char *longitude, const char *latitude, const char *image) :
		appeartime(time), tollgate(tollgate), longitude(longitude), latitude(latitude), image(image) {};
};

struct DataList {
	std::string licenseplate;
	std::string vehiclecolor;
	std::string vehiclebrand;
	std::vector<TraceList> trace;
	DataList() {};
	DataList(const char *lic, const char *color, const char *brand, TraceList t) :
		licenseplate(lic), vehiclecolor(color), vehiclebrand(brand) 
	{
		trace.push_back(t);
	};
};

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
	CarJudgedAlgorithm(const char *host, const char *user, const char *password, unsigned int port, const char *database, int taskcode);
	CarJudgedAlgorithm(std::string host, std::string user, std::string password, unsigned int port, std::string database, int taskcode);
	~CarJudgedAlgorithm();
	int test();
	MYSQL *ConnectDatabase(const char *host, const char *user, const char *password, unsigned int port, const char *database);
	int CloseDatabase(MYSQL *mysql);
	std::string GetResults();//得到最近的结果

	int FakePlateVehicles(const char *plateNo);
	int FakePlateVehicles_Thread(const char *plateNo);
	int FakePlateVehicles(const char *vColor, const char *vBrand, const char *st, const char *et);
	int CorrelationAnalysis(const char *plateno, const char *st, const char *et);//关联分析（伴随车）
	int TrajectoryCollision(std::vector<std::string> tollgates, const char *st, const char *et);//轨迹碰撞
	int FirstTimeEnterTown(std::vector<std::string> tollgates, const char *st, const char *et);//首次入城
	int ActInNight(std::vector<std::string> tollgates, const char *st, const char *et, const char *sdt, const char *edt,const char *snt,const char *ent);//昼伏夜出
	int ActInNightF(std::vector<std::string> tollgates, const char *st, const char *et, const char *snt, const char *ent,int thershold);//频繁夜出
	int HiddenVehicle(std::vector<std::string> tollgates, std::vector<std::string> platenos, const char *st, int bt, int ft);//隐匿车辆
	int FootholdAnalysis(std::vector<std::string> vehicles, const char *st, const char *et, const char *sft, const char *eft, int enduringtime);//落脚点分析

	int EncapsulateError(const char *error);

	bool IsReadable() { return ok; }
	bool IsConnectedtoMysql() { return mysql_ok; }
private:

	void thread(const char *plateNo, unsigned long step);
	void CommitThread(std::string plateno, unsigned long step,bool *ok);

	/*std::string host;
	std::string user;
	std::string password;
	std::string database;
	int port;*/
	bool IsFakePlateVehicle(Coordinate *path, unsigned int size);
	bool IsFakePlateVehicle(std::vector<Coordinate> &path);
	std::vector<std::string> IsFakePlateVehicle(std::map< std::string, std::vector<Coordinate> > &paths, std::vector<std::string> &plates);
	double distance(Coordinate A, Coordinate B);
	bool HasPlate(const std::vector<std::string> &plates, const std::string &plateno);
	void FindFakePlateVehicle(MYSQL_RES *results, MYSQL_FIELD fields,int rowcount, std::map<std::string, unsigned int> fieldsnameMap,std::string plateno);
	int EncapsulateFakePlatetoJson(const char *plateNo);
	int EncapsulateFakePlatetoJson(std::vector < std::string > fp);
	int EncapsulateFakePlatetoJsonThread(std::vector < std::string > fp);
	int EncapsulateJson(MYSQL_RES *results);

	int EncapsulateCorrelationAnalysistoJson(std::vector< std::string > plates);
	int EncapsulateTrajectoryCollisiontoJson(std::vector< std::string > plates);
	int EncapsulateFirstTimeEnterTowntoJson(std::vector< std::string > plates);

	int EncapsulteMysqlError(const char *error, unsigned int errorno, const char * errormsg);
	
	bool NightRange(const char *snt,const char *ent);
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

	inline std::vector<std::string> split(std::string str, std::string pattern)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		str += pattern;//扩展字符串以方便操作
		int size = str.size();

		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}
	inline void replace(std::string &str, char from, char to) {
		for (int i = 0, isize = str.size(); i < isize; i++)
		{
			if (str[i] == from)
				str[i] = to;
		}
	}
	inline int gethour(std::string time) {
		//return atoi(split(split(datetime, " ")[1], "-")[0].c_str());
		return atoi(split(time, ":")[0].c_str());
	} 

	MYSQL *mysql;
	MYSQL_RES *res;
	std::string json;
	int TaskCode;
	bool ok;
	bool mysql_ok;
};

