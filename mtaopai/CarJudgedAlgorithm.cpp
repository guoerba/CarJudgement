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
{
	ConnectDatabase(host, user, password, port, database);
	//FakePlateVehicles("��X21191");
	//FakePlateVehicles("��ɫ", "�µ�", "2018-10-29 5:10:40", "2018-10-30 15:10:15");
}

CarJudgedAlgorithm::CarJudgedAlgorithm(std::string host
									 , std::string user
									 , std::string password
									 , unsigned int port
									 , std::string database
									 , int taskcode)
	: TaskCode(taskcode)
	, json(std::string())
	/*: host(host)
	, user(user)
	, password(password)
	, port(port)
	, database(database)*/
	
{
	ConnectDatabase(host.c_str(), user.c_str(), password.c_str(), port, database.c_str());
	std::string().swap(host);
	std::string().swap(user);
	std::string().swap(password);
	std::string().swap(database);
}

CarJudgedAlgorithm::CarJudgedAlgorithm(const char * host
									 , const char * user
									 , const char * password
									 , unsigned int port
									 , const char * database
									 , const char * plateNo
									 , int taskcode)
	: TaskCode(taskcode)
	, json(std::string())
{
	ConnectDatabase(host, user, password, port, database);
	FakePlateVehicles(plateNo);
}

CarJudgedAlgorithm::CarJudgedAlgorithm(const char * host
									 , const char * user
									 , const char * password
									 , unsigned int port
									 , const char * database
									 , const char * vColor
									 , const char * vBrand
									 , const char * st
									 , const char * et
									 , int taskcode)
	: TaskCode(taskcode)
	, json(std::string())
{
	ConnectDatabase(host, user, password, port, database);
	FakePlateVehicles(vColor,vBrand,st,et);
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

	if ((mysql = mysql_init(mysql)) == NULL)//��ʼmysql���
	{
		std::cerr << "Cannot initialize mysql!!!" << std::endl;
		mysql_close(mysql);
		return -1;
	}

	if (mysql_real_connect(mysql, HOST, USER, PASSWORD, NULL, PORT, NULL, 0) == NULL)//�����ݿ⽨�����ӣ������ݿ⣩
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::cout << "successfully connect to mysql!!!" << std::endl;
	if (mysql_select_db(mysql, MYDB))//ѡ�����ݿ�(use database)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}


	//const char *str = "select MotorVehicleID from PlateNo";
	std::string qstr = (boost::format("select MotorVehicleID from MotorVehicle where PlateNo=\"%s\" ") % "��AJ6W33").str();
	std::cout << "��ѯָ��: " << qstr << std::endl;
	std::string str = boost::locale::conv::between(qstr.c_str(), "UTF-8", "GBK");
	if (mysql_query(mysql, str.c_str()))//���ݿ��ѯ����
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	str.clear();
	qstr.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	for (unsigned int i = 0; i < field_num; i++)
	{
		std::cout << fields[i].name << std::endl;//��ȡ��������
	}

	MYSQL_ROW row;
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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

	mysql_free_result(results);//�ͷ����ݱ�
	while (1);

	mysql_close(mysql);//�ر�����
	return 0;
}

MYSQL * CarJudgedAlgorithm::ConnectDatabase(const char * host, const char * user, const char * password, unsigned int port, const char * database)
{
	mysql = new MYSQL;

	if ((mysql = mysql_init(mysql)) == NULL)//��ʼmysql���
	{
		std::cerr << "Cannot initialize mysql!!!" << std::endl;
		mysql_close(mysql);
		return NULL;
	}

	if (mysql_real_connect(mysql, host, user, password, NULL, port, NULL, 0) == NULL)//�����ݿ⽨�����ӣ������ݿ⣩
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return NULL;
	}


	std::cout << "successfully connect to mysql!!!" << std::endl;

	if (mysql_select_db(mysql, database))//ѡ�����ݿ�(use database)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
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
	std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
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
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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
	std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
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
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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
	std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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
	std::string query;
	t.resize(t.size() - 1);
	if (NightRange(snt, ent))//�ж�ҹ���Ƿ�������賿
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
		std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
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
		std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	}
	
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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
	if (NightRange(snt, ent))//�ж�ҹ���Ƿ�������賿
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
		std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
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
		std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	}

	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	std::vector<std::string> targetplates;
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
		targetplates.push_back(U8toGBK(row[fieldsnameMap["PlateNo"]]));

	EncapsulateFirstTimeEnterTowntoJson(targetplates);
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
	std::cout << "��ѯ���1:     " << query.c_str() << std::endl;
	if (mysql_query(mysql,query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	my_ulonglong table_rows = mysql_num_rows(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string,unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;

	MYSQL_ROW row;
	int rlen = 0;
	Coordinate *paths = new Coordinate[table_rows];
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
	{
		paths[rlen++] = Coordinate(row[fieldsnameMap[std::string("Longitude")]]
							   , row[fieldsnameMap[std::string("Latitude")]]
						       , row[fieldsnameMap[std::string("time")]]);
	}

	mysql_free_result(results);
	if (IsFakePlateVehicle(paths, table_rows))
	{
		//������������,���ó�����Ϣ���ݷ�װ��json�����ϴ�
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
	std::cout << "��ѯ���: " << query.c_str() << std::endl;
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();
	std::string().swap(query);

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	my_ulonglong table_rows = mysql_num_rows(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;//��->������
	
	MYSQL_ROW row;//����ԭ�����������ݽṹ����Ϊһ�����������е�Ԫ�ذ���һ�������ĺ��룬�Լ��ó�����Coordinate����򣨶�̬���飩
	//int rlen = 0,plateCount = 0;
	std::map< std::string, std::vector<Coordinate> > paths;
	std::vector<std::string> plates;
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
	{
		if (!HasPlate(plates, std::string(row[fieldsnameMap[std::string("PlateNo")]])))//platenoû�л����е�PlateNo�ͱȽϵ�PlateNo��ͬ	
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
	std::vector < std::string > fp = IsFakePlateVehicle(paths, plates);//���������ɵ�����
	EncapsulateFakePlatetoJson(fp);
	mysql_free_result(results);
	std::map<std::string, unsigned int>().swap(fieldsnameMap);
	return 0;
}

bool CarJudgedAlgorithm::IsFakePlateVehicle(Coordinate * path, unsigned int size)//�ж�һ�������Ƿ������������
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

bool CarJudgedAlgorithm::IsFakePlateVehicle(std::vector<Coordinate> &path)//�ж�һ�������Ƿ������������
{
	return IsFakePlateVehicle(path.data(), path.size());
}

//�ж϶�������Ƿ�����������ĳ�����
std::vector<std::string> CarJudgedAlgorithm::IsFakePlateVehicle(std::map<std::string, std::vector<Coordinate>>& paths, std::vector<std::string>& plates)
{
	std::vector<std::string> fakeplates;
	for (int i = 0, isize = plates.size(); i < isize; i++)
	{
		if (IsFakePlateVehicle(paths[plates[i]]))		
			fakeplates.push_back(U8toGBK(plates[i]));
		/*for (int j = 0, jsize = paths[plates[i]].size(); j < jsize; j++)//���Դ���
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
//�������https://blog.csdn.net/xiejm2333/article/details/73297004
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
	std::cout << "��ѯ���2: " << query.c_str() << std::endl;
	query = GBKtoU8(query);
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
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
	std::cout << "��ѯ���2: " << query.c_str() << std::endl;
	query = GBKtoU8(query);
	if (mysql_query(mysql, query.c_str()))
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}
	query.clear();
	std::string().swap(query);

	MYSQL_RES *results = mysql_store_result(mysql);//ȡ���鵽�����ݱ���Ҫ�ֶ��ͷ��ڴ棩
	if (!results)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
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
	unsigned int field_num = mysql_num_fields(results);//�鵽�����ݱ��е�����
	MYSQL_FIELD *fields = mysql_fetch_fields(results);//���ز鵽�����ݱ��е���������Ϣ��������Ϊ�ṹ����ʽ
	if (!fields)
	{
		std::cerr << mysql_errno(mysql) << ":" << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return -1;
	}

	std::map<std::string, unsigned int> fieldsnameMap;
	for (unsigned int i = 0; i < field_num; i++)
		fieldsnameMap[std::string(fields[i].name)] = i;//��->������

	MYSQL_ROW row;//����ԭ�����������ݽṹ����Ϊһ�����������е�Ԫ�ذ���һ�������ĺ��룬�Լ��ó�����Coordinate����򣨶�̬���飩
	std::string currentPlate = std::string();
	boost::property_tree::ptree tree, datatree, listtree, tracetree, templisttree;
	tree.put("TaskCode", TaskCode);
	while (row = mysql_fetch_row(results))//�鵽�����ݱ��е��������ݣ���һ��һ�е�˳���ȡ
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

bool CarJudgedAlgorithm::NightRange(const char * snt, const char * ent)
{
	return (gethour(snt) - gethour(ent)) >= 0 ? true : false;
}
