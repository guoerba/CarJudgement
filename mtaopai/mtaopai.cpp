// mtaopai.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "CarJudgedAlgorithm.h"
#include "JsonParser.h"
#include <time.h>
/*
#pragma comment(lib,".\\mysql\\lib\\libmysql.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlclient.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlrouter_lib.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlservices.lib")
*/
/*
#define HOST "10.3.9.52"
#define USER "root"
#define PASSWORD "root"
#define PORT 3306
#define MYDB "db_ga_viid"
*/
#define HOST "localhost"
#define USER "root"
#define PASSWORD "guo"
#define PORT 3306
#define MYDB "db_ga_viid"

const char *json1 = "{\"TaskCode\":\"0\",\"LicensePlate\":\"川X21191\",\"Host\":\"localhost\",\"User\":\"root\",\"Password\":\"guo\",\"Port\":\"3306\",\"Database\":\"db_ga_viid\"}";
const char *json2 = "{\"TaskCode\":\"0\",\"VehicleColor\":\"蓝色\",\"VehicleBrand\":\"奥迪\",\"StartAnalysisTime\":\"2018-10-29 5:10:40\",\"EndAnalysisTime\":\"2018-10-30 15:10:15\",\"Host\":\"localhost\",\"User\":\"root\",\"Password\":\"guo\",\"Port\":\"3306\",\"Database\":\"db_ga_viid\"}";

int func()
{
	
	std::cout << _MSC_VER << std::endl;
	JsonParser(std::string(json1));
	//JsonParser(std::string(json2));
	//CarJudgedAlgorithm cj(HOST,USER,PASSWORD,PORT,MYDB);


	return 0;
}

 void receive(const char* sError, int nError, char* sData, int nLen, void* pAmqp, void* pFuncData)
{
	 if (sData)
	 {
		std::cout << "in callback!"  << std::endl;
		JsonParser(std::string(sData,nLen));
		// JsonParser(std::string(json2));
		std::cout << std::endl;
	 }
			
	 
}

int main()
{	
    std::cout << "Hello World!\n"; 
	//func();

	CAmqpImply imply;
	imply.BeginRecieve("10.3.9.236", 5672, "root", "root", 1, 1, "fanout", "FakePlateTest", 1, 0, 0, "FakePlateTest", 1, 0, 0, "FakePlateTestRoutingKey",receive);
	while (1);
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
