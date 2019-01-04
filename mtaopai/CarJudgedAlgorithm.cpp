#include "pch.h"
#include "CarJudgedAlgorithm.h"

#pragma comment(lib,".\\mysql\\lib\\libmysql.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlclient.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlrouter_lib.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlservices.lib")

CarJudgedAlgorithm::CarJudgedAlgorithm()
{
}


CarJudgedAlgorithm::CarJudgedAlgorithm(const char * host
									 , const char * user
									 , const char * password
									 , unsigned int port
									 , const char * database
									 , int taskcode)
	: TaskCode(taskcode)
	, json(std::string())
	, ok(false)
{
	ConnectDatabase(host, user, password, port, database) ? mysql_ok = true : mysql_ok = false;
	//FakePlateVehicles("川X21191");
	//FakePlateVehicles("蓝色", "奥迪", "2018-10-29 5:10:40", "2018-10-30 15:10:15");
}

CarJudgedAlgorithm::CarJudgedAlgorithm(std::string host
									 , std::string user
									 , std::string password
									 , unsigned int port
									 , std::string database
									 , int taskcode)
	: TaskCode(taskcode)
	, json(std::string())
	, ok(false)
	/*: host(host)
	, user(user)
	, password(password)
	, port(port)
	, database(database)*/
	
{
	ConnectDatabase(host.c_str(), user.c_str(), password.c_str(), port, database.c_str()) ? mysql_ok = true : mysql_ok = false;
	std::string().swap(host);
	std::string().swap(user);
	std::string().swap(password);
	std::string().swap(database);
}


CarJudgedAlgorithm::~CarJudgedAlgorithm()
{
	/*std::string().swap(host);
	std::string().swap(user);
	std::string().swap(password);
	std::string().swap(database);*/
	json.clear();
	std::string().swap(json);
	delete mysql;
}

int CarJudgedAlgorithm::test()
{
	MYSQL *mysql = NULL;

	if ((mysql = mysql_init(mysql)) == NULL)//初始mysql句柄
	{
		std::cerr << "Cannot initialize mysql!!!" << std::endl;
		mysql_close(mysql);
		return -1;
	}

	if (mysql_real_connect(mysql, HOST, USER, PASSWORD, NULL, PORT, NULL, 0) == NULL)//和数据库建立连接（打开数据库）
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::cout << "successfully connect to mysql!!!" << std::endl;
	if (mysql_select_db(mysql, MYDB))//选择数据库(use database)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}


	//const char *str = "select MotorVehicleID from PlateNo";
	std::string qstr = (boost::format("select MotorVehicleID from MotorVehicle where PlateNo=\"%s\" ") % "辽AJ6W33").str();
	std::cout << "查询指令: " << qstr << std::endl;
	std::string str = boost::locale::conv::between(qstr.c_str(), "UTF-8", "GBK");
	if (mysql_query(mysql, str.c_str()))//数据库查询操作
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	str.clear();
	qstr.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	for (unsigned int i = 0; i < field_num; i++)
	{
		std::cout << fields[i].name << std::endl;//读取所有列名
	}

	MYSQL_ROW row;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		for (unsigned int i = 0; i < field_num; i++)
		{
			if (row[i])
			{
				std::string s = boost::locale::conv::between(row[i], "GBK", "UTF-8");
				std::cout << fields[i].name << "\t:" << s.c_str() << std::endl;
				s.clear();
			}
			else
				std::cout << fields[i].name << "\t:" << "NULL" << std::endl;
		}
		//cout << "-------------------------------------------------------------------------------" << endl;
	}

	mysql_free_result(results);//释放数据表
	while (1);

	mysql_close(mysql);//关闭连接
	return 0;
}

MYSQL * CarJudgedAlgorithm::ConnectDatabase(const char * host, const char * user, const char * password, unsigned int port, const char * database)
{
	mysql = new MYSQL;

	if ((mysql = mysql_init(mysql)) == NULL)//初始mysql句柄
	{
		std::cerr << "Cannot initialize mysql!!!" << std::endl;
		EncapsulteMysqlError("Cannot initialize mysql!!!",mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return NULL;
	}

	if (mysql_real_connect(mysql, host, user, password, NULL, port, NULL, 0) == NULL)//和数据库建立连接（打开数据库）
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Cannot connect to database!!!",mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return NULL;
	}


	std::cout << "successfully connect to mysql!!!" << std::endl;

	if (mysql_select_db(mysql, database))//选择数据库(use database)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Error merges when select database!!!",mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return NULL;
	}

	return mysql;
}

int CarJudgedAlgorithm::CloseDatabase(MYSQL * mysql)
{
	mysql_close(mysql);
	return 0;
}

std::string CarJudgedAlgorithm::GetResults()
{
	return json;
}

int CarJudgedAlgorithm::CorrelationAnalysis(const char * plateno, const char * st, const char * et)
{
	std::cout << "in correlationanalysis!" << std::endl;
	std::string query = (boost::format("SELECT b.PlateNo AS plateno,"
		"COUNT(b.PlateNo) AS count "
		"FROM (SELECT AppearTime,"
			"PlateNo,"
			"DeviceID "
			"FROM MotorVehicle "
			"WHERE PlateNo=\"%s\" AND AppearTime BETWEEN \"%s\" AND \"%s\") a "
		"LEFT JOIN (SELECT AppearTime,"
			"PlateNo,"
			"DeviceID "
			"FROM MotorVehicle "
			"WHERE PlateNo<>\"%s\") b "
		"ON a.DeviceID = b.DeviceID "
		"WHERE "
		"CASE "
			"WHEN a.AppearTime < b.AppearTime "
				"THEN TIMESTAMPDIFF(SECOND, a.AppearTime, b.AppearTime) < 60 "
			"WHEN a.AppearTime > b.AppearTime "
				"THEN TIMESTAMPDIFF(SECOND, b.AppearTime, a.AppearTime) < 60 "
		"END "
		"GROUP BY plateno "
		"ORDER BY count DESC") % plateno % st % et % plateno).str();
	query = GBKtoU8(query);
	std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	int max = atoi(mysql_fetch_row(results)[fieldsnameMap["count"]]);
	mysql_data_seek(results, 0);
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		if (atoi(row[fieldsnameMap["count"]]) < max - CorrelationThresold)
			break;
		targetplates.push_back(U8toGBK(row[fieldsnameMap["plateno"]]));
	}
		
	EncapsulateCorrelationAnalysistoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::TrajectoryCollision(std::vector<std::string> tollgates, const char * st, const char * et)
{
	std::string t;
	for (int i = 0, isize = tollgates.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(tollgates[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);

	std::cout << "in TrajectoryCollision!" << std::endl;
	std::string query = (boost::format("SELECT PlateNo,COUNT(PlateNo) AS count "
		"FROM MotorVehicle WHERE DeviceID IN(%s) AND AppearTime BETWEEN \'%s\' AND \'%s\' "
		"GROUP BY PlateNo ORDER BY count DESC ") % t % st % et).str();
	query = GBKtoU8(query);
	std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	int max = atoi(mysql_fetch_row(results)[fieldsnameMap["count"]]);
	mysql_data_seek(results, 0);
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		if (atoi(row[fieldsnameMap["count"]]) < max - TrajectoryThresold)
			break;
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));
	}

	EncapsulateTrajectoryCollisiontoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::FirstTimeEnterTown(std::vector<std::string> tollgates, const char * st, const char * et)
{
	std::string t;
	for (int i = 0, isize = tollgates.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(tollgates[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);

	std::cout << "in FirstTimeEnterTown!" << std::endl;
	std::string query = (boost::format("SELECT PlateNo,COUNT(PlateNo) AS count "
		"FROM MotorVehicle WHERE DeviceID IN(%s) AND AppearTime BETWEEN \'%s\' AND \'%s\' "
		"GROUP BY PlateNo HAVING count=1") % t % st % et).str();
	query = GBKtoU8(query);
	std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));

	EncapsulateFirstTimeEnterTowntoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::ActInNight(
	std::vector<std::string> tollgates
	, const char * st
	, const char * et
	, const char *sdt
	, const char *edt
	, const char *snt
	, const char *ent)
{
	std::string t;
	for (int i = 0, isize = tollgates.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(tollgates[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);
	
	std::string query;
	if (NightRange(snt, ent))//判断夜晚是否持续到凌晨
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_daytime = date1;
		st_daytime.append(" ");
		st_daytime.append(sdt);
		std::string et_daytime = date1;
		et_daytime.append(" ");
		et_daytime.append(edt);
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(snt);
		std::string et_nighttime = date1;
		et_nighttime.append(" 23:59:59");
		std::string st_nighttime_1 = date2;
		st_nighttime_1.append(" 00:00:00");
		std::string et_nighttime_1 = date2;
		et_nighttime_1.append(" ");
		et_nighttime_1.append(ent);
		/*const char *st_daytime = "2018-10-28 06:00:01";
		const char *et_daytime = "2018-10-28 20:59:59";
		const char *st_nighttime = "2018-10-28 21:00:00";
		const char *et_nighttime = "2018-10-28 23:59:59";
		const char *st_nighttime_1 = "2018-10-28 00:00:00";
		const char *et_nighttime_1 = "2018-10-28 06:00:01";*/

		std::cout << "in ActinNight!" << std::endl;
		query = (boost::format("SELECT IFNULL(night.PlateNo,daytime.PlateNo) AS PlateNo,"
			"IFNULL(daytime.count,0) AS daytime,IFNULL(night.count,0) AS nighttime,"
			"IFNULL(night.count / daytime.count, 100) AS property FROM "
			"(SELECT PlateNo, COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN(%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') "
			"GROUP BY PlateNo ORDER BY count) daytime "
			"RIGHT JOIN "
			"(SELECT PlateNo, COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN(%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND(DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') "
			"OR DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\')) "
			"GROUP BY PlateNo) night "
			"ON night.PlateNo = daytime.PlateNo "
			"HAVING property > %d") % t % st % et % st_daytime % et_daytime % t % st % et % st_nighttime % et_nighttime % st_nighttime_1 % et_nighttime_1 % 1).str();
		query = GBKtoU8(query); std::cout << "after ActinNight!" << std::endl;
		std::cout << "查询语句1:     " << query.c_str() << std::endl;
	}
	else
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_daytime = date1;
		st_daytime.append(" ");
		st_daytime.append(sdt);
		std::string et_daytime = date1;
		et_daytime.append(" ");
		et_daytime.append(edt);
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(snt);
		std::string et_nighttime = date1;
		et_nighttime.append(" ");
		et_nighttime.append(ent);

		std::cout << "in ActinNight!" << std::endl;
		query = (boost::format("SELECT IFNULL(night.PlateNo,daytime.PlateNo) AS PlateNo,"
			"IFNULL(daytime.count,0) AS daytime,IFNULL(night.count,0) AS nighttime,"
			"IFNULL(night.count / daytime.count, 100) AS property FROM "
			"(SELECT PlateNo, COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN(%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') "
			"GROUP BY PlateNo ORDER BY count) daytime "
			"RIGHT JOIN "
			"(SELECT PlateNo, COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN(%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') "
			"GROUP BY PlateNo) night "
			"ON night.PlateNo = daytime.PlateNo "
			"HAVING property > %d") % t % st % et % st_daytime % et_daytime % t % st % et % st_nighttime % et_nighttime % 1).str();
		query = GBKtoU8(query); std::cout << "after ActinNight!" << std::endl;
		std::cout << "查询语句1:     " << query.c_str() << std::endl;
	}
	
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));

	/*for (auto i : targetplates)
	{
		std::cout << " " << i;
	}*/

	EncapsulateFirstTimeEnterTowntoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::ActInNightF(std::vector<std::string> tollgates, const char * st, const char * et, const char * snt, const char * ent, int thershold)
{
	std::string t;
	for (int i = 0, isize = tollgates.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(tollgates[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);

	std::string query;
	if (NightRange(snt, ent))//判断夜晚是否持续到凌晨
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(snt);
		std::string et_nighttime = date1;
		et_nighttime.append(" 23:59:59");
		std::string st_nighttime_1 = date2;
		st_nighttime_1.append(" 00:00:00");
		std::string et_nighttime_1 = date2;
		et_nighttime_1.append(" ");
		et_nighttime_1.append(ent);

		query = (boost::format(
			"SELECT PlateNo,COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN (%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND (DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') "
			"OR DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\')) "
			"GROUP BY PlateNo "
			"HAVING count > %d") % t % st % et % st_nighttime % et_nighttime % st_nighttime_1 % et_nighttime_1 % thershold).str();
		query = GBKtoU8(query); std::cout << "after ActinNight!" << std::endl;
		std::cout << "查询语句1:     " << query.c_str() << std::endl;
	}
	else
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(snt);
		std::string et_nighttime = date1;
		et_nighttime.append(" ");
		et_nighttime.append(ent);

		std::cout << "in ActinNight!" << std::endl;
		query = (boost::format(
			"SELECT PlateNo,COUNT(AppearTime) AS count FROM motorvehicle WHERE DeviceID IN (%s) "
			"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
			"AND (DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\') AND DATE_FORMAT(\'%s\', \'%%H:%%i:%%S\')) "
			"GROUP BY PlateNo "
			"HAVING count > %d") % t % st % et % st_nighttime % et_nighttime % thershold).str();
		query = GBKtoU8(query); std::cout << "after ActinNight!" << std::endl;
		std::cout << "查询语句1:     " << query.c_str() << std::endl;
	}

	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));

	EncapsulateFirstTimeEnterTowntoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::HiddenVehicle(std::vector<std::string> tollgates, std::vector<std::string> platenos, const char * st, int bt, int ft)
{
	std::cout << "in correlationanalysis!" << std::endl;

	std::string t;
	for (int i = 0, isize = tollgates.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(tollgates[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);

	std::string p;
	for (int i = 0, isize = platenos.size(); i < isize; i++)
	{
		p.append("\'");
		p.append(platenos[i]);
		p.append("\',");
	}
	p.resize(p.size() - 1);

	boost::posix_time::ptime startTime(boost::posix_time::time_from_string(st));
	boost::gregorian::days btime(bt);
	std::string backtracingTime = boost::posix_time::to_iso_extended_string(startTime - btime);
	replace(backtracingTime, 'T', ' ');
	boost::gregorian::days dtime(ft);
	std::string followupTime = boost::posix_time::to_iso_extended_string(startTime + dtime);
	replace(followupTime, 'T', ' ');

	std::string query = (boost::format("SELECT "
		"IFNULL(beforecase.PlateNo,aftercase.PlateNo) AS PlateNo,"
		"IFNULL(beforecase.count, 0) AS beforecase,"
		"IFNULL(aftercase.count, 0) AS aftercase,"
		"IFNULL(aftercase.count / beforecase.count,100) AS property FROM "
		"(SELECT PlateNo,COUNT(AppearTime) AS count FROM motorvehicle WHERE "
		"DeviceID IN(%s) AND AppearTime BETWEEN \'%s\' AND \'%s\' "
		"GROUP BY PlateNo "
		"ORDER BY count) beforecase "
		"LEFT JOIN"
		"(SELECT PlateNo,COUNT(AppearTime) AS count FROM motorvehicle "
		"WHERE	DeviceID IN(%s) "
		"AND AppearTime BETWEEN \'%s\' AND \'%s\' "
		"GROUP BY PlateNo) aftercase " 
		"ON beforecase.PlateNo = aftercase.PlateNo "
		"WHERE beforecase.PlateNo NOT IN(%s)") % t % backtracingTime % st % t % st % followupTime % p).str();
	query = GBKtoU8(query);
	std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));

	EncapsulateCorrelationAnalysistoJson(targetplates);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::FootholdAnalysis(std::vector<std::string> vehicles, const char * st, const char * et, const char * sft, const char * eft, int enduringtime)
{
	std::cout << "in FootholdAnalysis!" << std::endl;

	std::string t;
	for (int i = 0, isize = vehicles.size(); i < isize; i++)
	{
		t.append("\'");
		t.append(vehicles[i]);
		t.append("\',");
	}
	t.resize(t.size() - 1);

	std::string query;
	if (NightRange(sft, eft))//判断夜晚是否持续到凌晨
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(sft);
		std::string et_nighttime = date1;
		et_nighttime.append(" 23:59:59");
		std::string st_nighttime_1 = date2;
		st_nighttime_1.append(" 00:00:00");
		std::string et_nighttime_1 = date2;
		et_nighttime_1.append(" ");
		et_nighttime_1.append(eft);
		query = (boost::format("SELECT "
			"mv.PlateNo AS PlateNo,"
			"mv.AppearTime AS PassTime,"
			"UNIX_TIMESTAMP(mv.AppearTime) AS Ts,"
			"IFNULL(mv.DeviceID, 0) AS DeviceID,"
			"IFNULL(tollgate.Latitude, 0) AS Latitude,"
			"IFNULL(tollgate.Longitude, 0) AS Longitude,"
			"ct.color_name AS Color,"
			"bt.`name` AS Brand,"
			"IFNULL(mv.SubImageList, 0) AS Image "
			"FROM motorvehicle mv "
			"LEFT JOIN "
			"lane ON lane.ApeID = mv.DeviceID "
			"LEFT JOIN "
			"tollgate ON tollgate.TollgateID = lane.TollgateID "
			"LEFT JOIN "
			"colortype ct ON ct.color_value = mv.VehicleColor "
			"LEFT JOIN "
			"vehiclebrandtype bt ON bt.`value` = mv.VehicleBrand "
			"WHERE PlateNo IN(%s) "
			"AND mv.AppearTime BETWEEN \'%s\' AND \'%s\' "//st,et
			"AND (DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\') "
			"AND DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\') "
			"OR DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\') "
			"AND DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\')) "
			"ORDER BY PlateNo,Ts") % t % st % et % st_nighttime % et_nighttime % st_nighttime_1 % et_nighttime_1).str();
	}
	else
	{
		std::string date1 = split(st, " ")[0];
		std::string date2 = split(et, " ")[0];
		std::string st_daytime = date1;
		std::string st_nighttime = date1;
		st_nighttime.append(" ");
		st_nighttime.append(sft);
		std::string et_nighttime = date1;
		et_nighttime.append(" ");
		et_nighttime.append(eft);
	
		query = (boost::format("SELECT "
			"mv.PlateNo AS PlateNo,"
			"mv.AppearTime AS PassTime,"
			"UNIX_TIMESTAMP(mv.AppearTime) AS Ts,"
			"IFNULL(mv.DeviceID, 0) AS DeviceID,"
			"IFNULL(tollgate.Latitude, 0) AS Latitude,"
			"IFNULL(tollgate.Longitude, 0) AS Longitude,"
			"ct.color_name AS Color,"
			"bt.`name` AS Brand,"
			"IFNULL(mv.SubImageList, 0) AS Image "
			"FROM motorvehicle mv "
			"LEFT JOIN "
			"lane ON lane.ApeID = mv.DeviceID "
			"LEFT JOIN "
			"tollgate ON tollgate.TollgateID = lane.TollgateID "
			"LEFT JOIN "
			"colortype ct ON ct.color_value = mv.VehicleColor "
			"LEFT JOIN "
			"vehiclebrandtype bt ON bt.`value` = mv.VehicleBrand "
			"WHERE PlateNo IN(%s) "
			"AND mv.AppearTime BETWEEN \'%s\' AND \'%s\' "//st,et
			"AND (DATE_FORMAT(AppearTime, \'%%H:%%i:%%S\') BETWEEN DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\') "
			"AND DATE_FORMAT(\'%s\',\'%%H:%%i:%%S\') "
			"ORDER BY PlateNo,Ts") % t % st % et % st_nighttime % et_nighttime).str();
	}
	//query = GBKtoU8(query);
	//std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	unsigned int row_num = mysql_num_rows(results);
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	std::vector<std::string> oldtmp(field_num), tmp(field_num);
	MYSQL_ROW row = mysql_fetch_row(results);
	if (!row || row_num < 2)
	{
		std::cout << "no results found!" << std::endl;
		return -1;
	}

	unsigned long long lasttime = enduringtime * 3600;

	boost::property_tree::ptree tree, datatree, listtree, footholdstree;
	tree.put("TaskCode", TaskCode);
	for (int i = 0, isize = field_num; i < isize; i++)
		oldtmp[i] = row[i];
	int footholds_num = 0;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		std::cout << "in circle" << std::endl;
		if (atoll(row[fieldsnameMap["Ts"]]) - atoll(oldtmp[fieldsnameMap["Ts"]].c_str()) >= lasttime)
		{
			std::cout << "in condition 1" << std::endl;
			if (!std::string(row[fieldsnameMap["PlateNo"]]).compare(oldtmp[fieldsnameMap["PlateNo"]]))
			{
				std::cout << "if" << std::endl;
				boost::property_tree::ptree tempfootholdtree1, tempfootholdtree2, footholdtree;
				tempfootholdtree1.put("PassTime", oldtmp[fieldsnameMap["PassTime"]]);
				tempfootholdtree1.put("DeviceID", oldtmp[fieldsnameMap["DeviceID"]]);
				tempfootholdtree1.put("Longitude", oldtmp[fieldsnameMap["Longitude"]]);
				tempfootholdtree1.put("Latitude", oldtmp[fieldsnameMap["Latitude"]]);
				tempfootholdtree1.put("Image", oldtmp[fieldsnameMap["Image"]]);
				footholdtree.push_back(std::make_pair("", tempfootholdtree1));

				tempfootholdtree2.put("PassTime", row[fieldsnameMap["PassTime"]]);
				tempfootholdtree2.put("DeviceID", row[fieldsnameMap["DeviceID"]]);
				tempfootholdtree2.put("Longitude", row[fieldsnameMap["Longitude"]]);
				tempfootholdtree2.put("Latitude", row[fieldsnameMap["Latitude"]]);
				tempfootholdtree2.put("Image", row[fieldsnameMap["Image"]]);
				footholdtree.push_back(std::make_pair("", tempfootholdtree2));
				footholdstree.push_back(std::make_pair("", footholdtree));
				
			}
			else
			{
				std::cout << "else" << std::endl;
				if (footholdstree.size())
				{
					boost::property_tree::ptree templisttree;
					templisttree.put("LicensePlate", oldtmp[fieldsnameMap["PlateNo"]]);
					templisttree.put("VehicleColor", oldtmp[fieldsnameMap["Color"]]);
					templisttree.put("VehicleBrand", oldtmp[fieldsnameMap["Brand"]]);
					templisttree.put_child("Footholds", footholdstree);
					templisttree.put("FootholdsNumber", footholdstree.size());
					listtree.push_back(std::make_pair("", templisttree));
					footholdstree.clear();
				}
			}
		}
		
		oldtmp.clear();
		oldtmp.resize(field_num);
		for (int i = 0, isize = field_num; i < isize; i++)
			oldtmp[i] = row[i];
	}
	if (footholdstree.size())
	{
		boost::property_tree::ptree tmplisttree;
		tmplisttree.put("LicensePlate", oldtmp[fieldsnameMap["PlateNo"]]);
		tmplisttree.put("VehicleColor", oldtmp[fieldsnameMap["Color"]]);
		tmplisttree.put("VehicleBrand", oldtmp[fieldsnameMap["Brand"]]);
		tmplisttree.put_child("Footholds", footholdstree);
		tmplisttree.put("FootholdsNumber", footholdstree.size());
		listtree.push_back(std::make_pair("", tmplisttree));
	}

	oldtmp.clear();
	datatree.put_child("list", listtree);
	datatree.put("listNumber", listtree.size());
	tree.put_child("Data", datatree);

	std::stringstream ssr;
	boost::property_tree::write_json(ssr, tree);
	json = ssr.str();

	ok = true;
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::FakePlateVehicles(const char *plateNo)
{
	std::string query = (boost::format("select"
		" Tollgate.Longitude"
		",Tollgate.Latitude"
		",unix_timestamp(MotorVehicle.AppearTime) as time"
		" from MotorVehicle"
		" join Lane on MotorVehicle.DeviceID = Lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = Lane.TollgateID"
		" where MotorVehicle.plateNo=\"%s\""
		" order by MotorVehicle.AppearTime desc") % (plateNo)).str();
	query = GBKtoU8(query);
	std::cout << "查询语句1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql,query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	my_ulonglong table_rows = mysql_num_rows(results);//查到的数据表中的行数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string,unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	int rlen = 0;
	Coordinate *paths = new Coordinate[table_rows];
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		paths[rlen++] = Coordinate(row[fieldsnameMap[std::string("Longitude")]]
							   , row[fieldsnameMap[std::string("Latitude")]]
						       , row[fieldsnameMap[std::string("time")]]);
	}

	mysql_free_result(results);
	if (IsFakePlateVehicle(paths, table_rows))
	{
		//存在套牌现象,将该车辆信息数据封装成json，并上传
		EncapsulateFakePlatetoJson(plateNo);
	}
	delete[] paths;
	//std::map<std::string,unsigned int>().swap(fieldsnameMap);
	return 0;
}

int CarJudgedAlgorithm::FakePlateVehicles(const char * vColor, const char * vBrand, const char * st, const char * et)
{
	std::string query = (boost::format("select"
		" Tollgate.Longitude"
		",Tollgate.Latitude"
		",MotorVehicle.PlateNo"
		",unix_timestamp(MotorVehicle.AppearTime)"
		" from MotorVehicle"
		" join Lane on MotorVehicle.DeviceID = Lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = Lane.TollgateID"
		" join ColorType on MotorVehicle.VehicleColor = ColorType.color_value"
		" join VehicleBrandType on MotorVehicle.VehicleBrand = VehicleBrandType.value"
		" where ColorType.color_name=\"%s\" and VehicleBrandType.name=\"%s\" and"
		" (MotorVehicle.AppearTime between \'%s\' and \'%s\')"
		" order by MotorVehicle.AppearTime desc") % vColor % vBrand % st % et).str();//MotorVehicle.AppearTime
	query = GBKtoU8(query);
	std::cout << "查询语句: " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();
	std::string().swap(query);

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	my_ulonglong table_rows = mysql_num_rows(results);//查到的数据表中的行数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;//列->索引表
	
	MYSQL_ROW row;//弃用原方法，将数据结构构建为一个链表，链表中的元素包括一个车辆的号码，以及该车辆的Coordinate数组或（动态数组）
	//int rlen = 0,plateCount = 0;
	std::map< std::string, std::vector<Coordinate> > paths;
	std::vector<std::string> plates;
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		if (!HasPlate(plates, std::string(row[fieldsnameMap[std::string("PlateNo")]])))//plateno没有或现有的PlateNo和比较的PlateNo不同	
			plates.push_back(std::string(row[fieldsnameMap[std::string("PlateNo")]]));		//WRONG!!!!!!	
		/*for (unsigned int i = 0; i < field_num; i++)
		{
			if (row[i])
			{
				std::string s = boost::locale::conv::between(row[i], "GBK", "UTF-8");
				std::cout << fields[i].name << "\t:" << s.c_str() << std::endl;
				
				s.clear();
				std::string().swap(s);
			}
			else
				std::cout << fields[i].name << "\t:" << "NULL" << std::endl;
		}	*/	
		paths[std::string(row[fieldsnameMap[std::string("PlateNo")]])].push_back(Coordinate(row[fieldsnameMap[std::string("Longitude")]]
																						  , row[fieldsnameMap[std::string("Latitude")]]
																						  , row[fieldsnameMap[std::string("unix_timestamp(MotorVehicle.AppearTime)")]]));
	}

	mysql_data_seek(results, 0);
	std::vector < std::string > fp = IsFakePlateVehicle(paths, plates);//所有有嫌疑的套牌
	EncapsulateFakePlatetoJson(fp);
	mysql_free_result(results);
	std::map<std::string, unsigned int>().swap(fieldsnameMap);
	return 0;
}

bool CarJudgedAlgorithm::IsFakePlateVehicle(Coordinate * path, unsigned int size)//判断一个车牌是否存在套牌现象
{
	if (size < 2)
	{
		std::cout << "path is less than 2,can't judge!!!" << std::endl;
		return false;
	}
	for (unsigned int i = 0; i < size - 1; i++)
	{
		double d = distance(path[i], path[i + 1]);
		double v = d / (abs(path[i].time - path[i + 1].time));
		std::cout << (boost::format("distance,speed in time between %d and %d is %f,%f") % path[i].time % path[i + 1].time % d % v).str().c_str() << std::endl;
		if (v > Threshold)
			return true;
	}
	return false;
}

bool CarJudgedAlgorithm::IsFakePlateVehicle(std::vector<Coordinate> &path)//判断一个车牌是否存在套牌现象
{
	return IsFakePlateVehicle(path.data(), path.size());
}

//判断多个车牌是否有套牌现象的车出现
std::vector<std::string> CarJudgedAlgorithm::IsFakePlateVehicle(std::map<std::string, std::vector<Coordinate>>& paths, std::vector<std::string>& plates)
{
	std::vector<std::string> fakeplates;
	for (int i = 0, isize = plates.size(); i < isize; i++)
	{
		if (IsFakePlateVehicle(paths[plates[i]]))		
			fakeplates.push_back(U8toGBK(plates[i]));
		/*for (int j = 0, jsize = paths[plates[i]].size(); j < jsize; j++)//测试代码
		{
			std::cout << (boost::format("paths[%s] = %f,%f,%d\n")
				% U8toGBK(plates[i]).c_str()
				% paths[plates[i]][j].longitude
				% paths[plates[i]][j].latitude
				% paths[plates[i]][j].time).str().c_str()
				<< std::endl;
		}*/
	}
	return fakeplates;
}

double CarJudgedAlgorithm::distance(Coordinate A, Coordinate B)
{
	double Lat1 = AngleToArc(A.latitude);//WA
	double Lat2 = AngleToArc(B.latitude);//WB
	double a = Lat1 - Lat2;//WA-WB
	double b = AngleToArc(A.longitude) - AngleToArc(B.longitude);//JA-JB
	double s = 2 * EarthRadius*asin(sqrt(pow(sin(a / 2), 2) + cos(Lat1)*cos(Lat2)*pow(sin(b / 2), 2)));
	return s;
}
bool CarJudgedAlgorithm::HasPlate(const std::vector<std::string> &plates,const std::string &plateno)
{
	for (int i = 0, isize = plates.size(); i < isize; i++)
	{
		if (plates.at(i) == plateno)
			return true;
	}
	return false;
}
//详情请见https://blog.csdn.net/xiejm2333/article/details/73297004
void CarJudgedAlgorithm::FindFakePlateVehicle(MYSQL_RES *results, MYSQL_FIELD fields, int rowcount, std::map<std::string, unsigned int> fieldsnameMap, std::string plateno)
{
	mysql_data_seek(results, 0);

	MYSQL_ROW row;
	/*while (row = mysql_fetch_row(results))
	{
		if(row[fieldsnameMap["Pla"]])
	}*/
}
int CarJudgedAlgorithm::EncapsulateFakePlatetoJson(const char * plateNo)
{
	std::string query = (boost::format("select"
		" VehicleBrandType.name as brand"
		",ColorType.color_name as color"
		",Tollgate.name as tollgate"
		",Tollgate.Longitude as longitude"
		",Tollgate.Latitude as latitude"
		",MotorVehicle.plateNo"
		",MotorVehicle.SubImageList as image"
		",MotorVehicle.AppearTime as time"
		" from MotorVehicle"
		" join Lane on MotorVehicle.DeviceID = Lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = Lane.TollgateID"
		" join ColorType on MotorVehicle.VehicleColor = ColorType.color_value"
		" join VehicleBrandType on MotorVehicle.VehicleBrand = VehicleBrandType.value"
		" where MotorVehicle.plateNo=\"%s\""
		" order by MotorVehicle.AppearTime desc") % (plateNo)).str();
	std::cout << "查询语句2: " << query.c_str() << std::endl;
	query = GBKtoU8(query);
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	EncapsulateJson(results);
	mysql_free_result(results);
	return 0;
}

int CarJudgedAlgorithm::EncapsulateFakePlatetoJson(std::vector<std::string> fp)
{
	if (fp.empty())
	{
		std::cout << "no target vehicles found!" << std::endl;
		EncapsulteMysqlError("Datastructure fakeplates is empty!!!", -1, "");
		return -1;
	}

	std::string plates;
	for (int i= 0 , isize = fp.size(); i < isize; i++)
	{
		plates.append("\'");
		plates.append(fp[i]);
		plates.append("\',");
	}
	plates.resize(plates.size() - 1);

	std::string query = (boost::format("select"
		" VehicleBrandType.name as brand"
		",ColorType.color_name as color"
		",Tollgate.name as tollgate"
		",Tollgate.Longitude as longitude"
		",Tollgate.Latitude as latitude"
		",MotorVehicle.plateNo"
		",MotorVehicle.SubImageList as image"
		",MotorVehicle.AppearTime as time"
		" from MotorVehicle"
		" join Lane on MotorVehicle.DeviceID = Lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = Lane.TollgateID"
		" join ColorType on MotorVehicle.VehicleColor = ColorType.color_value"
		" join VehicleBrandType on MotorVehicle.VehicleBrand = VehicleBrandType.value"
		" where MotorVehicle.plateNo in (%s)"
		" order by MotorVehicle.plateNo desc") % (plates)).str();
	std::cout << "查询语句2: " << query.c_str() << std::endl;
	query = GBKtoU8(query);
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Query error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}
	query.clear();
	std::string().swap(query);

	MYSQL_RES *results = mysql_store_result(mysql);//取出查到的数据表（需要手动释放内存）
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_RES error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	EncapsulateJson(results);

	mysql_free_result(results);
	//std::map<std::string, unsigned int>().swap(fieldsnameMap);
	return 0;
}

int CarJudgedAlgorithm::EncapsulateJson(MYSQL_RES * results)
{
	unsigned int field_num = mysql_num_fields(results);//查到的数据表中的列数
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//返回查到的数据表中的所有列信息，并保存为结构体形式
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		EncapsulteMysqlError("Datastructure MYSQL_FIELD error!!!", mysql_errno(mysql), mysql_error(mysql));
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;//列->索引表

	MYSQL_ROW row;//弃用原方法，将数据结构构建为一个链表，链表中的元素包括一个车辆的号码，以及该车辆的Coordinate数组或（动态数组）
	std::string currentPlate = std::string();
	boost::property_tree::ptree tree, datatree, listtree, tracetree, templisttree;
	tree.put("TaskCode", TaskCode);
	while (row = mysql_fetch_row(results))//查到的数据表中的所有数据，并一行一行的顺序读取
	{
		std::string tempstr = (row[fieldsnameMap["plateNo"]]);
		if (currentPlate.empty())
		{
			currentPlate = (row[fieldsnameMap["plateNo"]]);
			templisttree.put("LicensePlate", currentPlate);
			templisttree.put("VehicleColor", (row[fieldsnameMap["color"]]));
			templisttree.put("VehicleBrand", (row[fieldsnameMap["brand"]]));
		}
		else if (tempstr != currentPlate)
		{
			currentPlate = (row[fieldsnameMap["plateNo"]]);
			templisttree.put_child("Trace", tracetree);
			templisttree.put("TraceNumber", tracetree.size());
			listtree.push_back(std::make_pair("", templisttree));
			templisttree.clear();
			templisttree.put("LicensePlate", currentPlate);
			templisttree.put("VehicleColor", (row[fieldsnameMap["color"]]));
			templisttree.put("VehicleBrand", (row[fieldsnameMap["brand"]]));
			tracetree.clear();
		}
		boost::property_tree::ptree temptracetree;
		temptracetree.put("PassTime", (row[fieldsnameMap["time"]]));
		temptracetree.put("TollgateName", (row[fieldsnameMap["tollgate"]]));
		temptracetree.put("Longitude", (row[fieldsnameMap["longitude"]]));
		temptracetree.put("Latitude", (row[fieldsnameMap["latitude"]]));
		temptracetree.put("Image", (row[fieldsnameMap["image"]]));
		tracetree.push_back(std::make_pair("", temptracetree));
	}
	templisttree.put_child("Trace", tracetree);
	templisttree.put("TraceNumber", tracetree.size());
	listtree.push_back(std::make_pair("", templisttree));
	datatree.put_child("list", listtree);
	datatree.put("listNumber", listtree.size());
	tree.put_child("data", datatree);

	std::stringstream ssr;
	boost::property_tree::write_json(ssr, tree);
	json = ssr.str();
	ok = true;
	return 0;
}

int CarJudgedAlgorithm::EncapsulateCorrelationAnalysistoJson(std::vector<std::string> plates)
{
	return EncapsulateFakePlatetoJson(plates);
}

int CarJudgedAlgorithm::EncapsulateTrajectoryCollisiontoJson(std::vector<std::string> plates)
{
	return EncapsulateFakePlatetoJson(plates);
}

int CarJudgedAlgorithm::EncapsulateFirstTimeEnterTowntoJson(std::vector<std::string> plates)
{
	return EncapsulateFakePlatetoJson(plates);
}

int CarJudgedAlgorithm::EncapsulteMysqlError(const char *error, unsigned int errorno, const char * errormsg)
{
	boost::property_tree::ptree tree;
	tree.put("TaskCode", -1);
	tree.put("Error", error);
	tree.put("ErrorNumber", errorno);
	tree.put("ErrorMsg", errormsg);

	std::cout << "in error dealing" << std::endl;
	
	std::stringstream ssr;
	boost::property_tree::write_json(ssr, tree);
	json = ssr.str();

	return 0;
}

bool CarJudgedAlgorithm::NightRange(const char * snt, const char * ent)
{
	return (gethour(snt) - gethour(ent)) >= 0 ? true : false;
}
