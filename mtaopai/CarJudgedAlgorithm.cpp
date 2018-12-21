#include "pch.h"
#include "CarJudgedAlgorithm.h"

#pragma comment(lib,".\\mysql\\lib\\libmysql.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlclient.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlrouter_lib.lib")
#pragma comment(lib,".\\mysql\\lib\\mysqlservices.lib")

CarJudgedAlgorithm::CarJudgedAlgorithm()
{
}


CarJudgedAlgorithm::CarJudgedAlgorithm(const char * host, const char * user, const char * password, unsigned int port, const char * database)
{
	json = std::string();
	ConnectDatabase(host, user, password, port, database);
	//FakePlateVehicles("��X21191");
	//FakePlateVehicles("��ɫ", "�µ�", "2018-10-29 5:10:40", "2018-10-30 15:10:15");
}

CarJudgedAlgorithm::CarJudgedAlgorithm(std::string host, std::string user, std::string password, unsigned int port, std::string database)
	/*: host(host)
	, user(user)
	, password(password)
	, port(port)
	, database(database)*/
	
{
	json = std::string();
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
									 , const char * plateNo)
{
	json = std::string();
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
									 , const char * et)
{
	json = std::string();
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


int CarJudgedAlgorithm::FakePlateVehicles(const char *plateNo)
{
	std::string query = (boost::format("select"
		" Tollgate.Longitude"
		",Tollgate.Latitude"
		",unix_timestamp(MotorVehicle.AppearTime) as time"
		" from MotorVehicle"
		" join lane on MotorVehicle.DeviceID = lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = lane.TollgateID"
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
		" join lane on MotorVehicle.DeviceID = lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = lane.TollgateID"
		" join colortype on MotorVehicle.VehicleColor = colortype.color_value"
		" join vehiclebrandtype on MotorVehicle.VehicleBrand = vehiclebrandtype.value"
		" where colortype.color_name=\"%s\" and vehiclebrandtype.name=\"%s\" and"
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
		" vehiclebrandtype.name as brand"
		",colortype.color_name as color"
		",Tollgate.name as tollgate"
		",Tollgate.Longitude as longitude"
		",Tollgate.Latitude as latitude"
		",MotorVehicle.plateNo"
		",MotorVehicle.SubImageList as image"
		",MotorVehicle.AppearTime as time"
		" from MotorVehicle"
		" join lane on MotorVehicle.DeviceID = lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = lane.TollgateID"
		" join colortype on MotorVehicle.VehicleColor = colortype.color_value"
		" join vehiclebrandtype on MotorVehicle.VehicleBrand = vehiclebrandtype.value"
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
		" vehiclebrandtype.name as brand"
		",colortype.color_name as color"
		",Tollgate.name as tollgate"
		",Tollgate.Longitude as longitude"
		",Tollgate.Latitude as latitude"
		",MotorVehicle.plateNo"
		",MotorVehicle.SubImageList as image"
		",MotorVehicle.AppearTime as time"
		" from MotorVehicle"
		" join lane on MotorVehicle.DeviceID = lane.ApeID"
		" join Tollgate on Tollgate.TollgateID = lane.TollgateID"
		" join colortype on MotorVehicle.VehicleColor = colortype.color_value"
		" join vehiclebrandtype on MotorVehicle.VehicleBrand = vehiclebrandtype.value"
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
	tree.put("TaskCode", 0);
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
