#include "pch.h"
#include "JsonParser.h"
typedef boost::error_info<struct tag_err_no, int> err_no;
typedef boost::error_info<struct tag_err_str, std::string> err_str;



JsonParser::JsonParser(std::string s)
	: itree(NULL)
	, otree(NULL)
{
	JsonParser(s.c_str());
}

JsonParser::JsonParser(const char * json)
	: itree(NULL)
	, otree(NULL)
{
	itree = new boost::property_tree::ptree;
	otree = new boost::property_tree::ptree;
	//boost::locale::conv::
	//std::stringstream ss(boost::locale::conv::between(json, "UTF-8", "GBK"));
	std::stringstream ss(json);
	std::cout << "in JsonParser:!\n" << ss.str() << std::endl;
	try {
		boost::property_tree::read_json<boost::property_tree::ptree>(ss, *itree);//解析出错可能是因为json格式有误
	}
	catch (my_exception &e)
	{
		std::cout << *boost::get_error_info<err_str>(e) << std::endl;//boost异常
        std::cout << e.what() << std::endl;//std异常
	}

	std::stringstream ssr;
	boost::property_tree::write_json(ssr, *itree);
	std::cout << "已解析json/n" << boost::locale::conv::between(ssr.str(),"GBK", "UTF-8") << std::endl;

	auto it = itree->find("TaskCode");
	
	if (it == itree->not_found())
	{
		std::cout << "no TaskCode found !!!we need TaskCode,TaskCode!!!!!!" << std::endl;
	}
	else
	{		
	
		int TaskCode = itree->get<int>("TaskCode");
		std::cout << "TaskCode: " << TaskCode << std::endl;
		switch (TaskCode)
		{
		case 0:ExtractFakePlatesfromJson(TaskCode); break;
		case 1:ExtractCorrelationAnalysisfromJson(TaskCode); break;
		case 2:ExtractTrajectoryCollisionfromJson(TaskCode); break;
		case 3:ExtractFirstTimeEnterTownfromJson(TaskCode); break;
		case 4:ExtractActInNightfromJson(TaskCode); break;
		case 5:ExtractActInNightFfromJson(TaskCode); break;
		}
		
	}
}

JsonParser::JsonParser(const JsonParser & copy)
{
	boost::property_tree::ptree *ptree = new boost::property_tree::ptree;
	*ptree = *(copy.itree);
}


JsonParser::~JsonParser()
{
	if(itree != NULL)
		delete itree;
	if (otree != NULL)
		delete otree;
}

void JsonParser::ReadJsonFromStream(std::string s)
{
		std::stringstream ss(boost::locale::conv::between(s, "UTF-8", "GBK").c_str());//遇到中文时需要把GBK格式转换为UTF-8格式，boost::json解析才不会出错
		boost::property_tree::ptree t;
		boost::property_tree::read_json<boost::property_tree::ptree>(ss, t);

		//std::cout << boost::format("element[%s]: %s") % "LicensePlate" % t.get<std::string>("LicensePlate") << std::endl;
		auto it = t.find("LicensePlate");
		if (it == t.not_found())
		{
			std::cout << "not find key LicensePlate" << std::endl;
		}
		else
			std::cout << boost::format("element[%s]: %s") % "LicensePlate" % t.get<std::string>("LicensePlate") << std::endl;

		std::stringstream ssr;
		boost::property_tree::write_json(ssr, t);
		std::cout << boost::locale::conv::between(ssr.str(),"GBK","UTF-8") << std::endl;
}

void JsonParser::ExtractFakePlatesfromJson(int task)
{
	auto it = itree->find("LicensePlate");
	if (it == itree->not_found())//高级研判
	{
		CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
			, itree->get<std::string>("User").c_str()
			, itree->get<std::string>("Password").c_str()
			, itree->get<int>("Port")
			, itree->get<std::string>("Database").c_str()
			, boost::locale::conv::between(itree->get<std::string>("VehicleColor"), "GBK", "UTF-8").c_str()
			, boost::locale::conv::between(itree->get<std::string>("VehicleBrand"), "GBK", "UTF-8").c_str()
			, itree->get<std::string>("StartAnalysisTime").c_str()
			, itree->get<std::string>("EndAnalysisTime").c_str()
		    , task);
		std::cout << boost::locale::conv::between(c.GetResults(),"GBK","UTF-8") << std::endl;
		CAmqpSample amqp;
		std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
		std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
		std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
	}
	else//车牌研判
	{
		/*std::string host = itree->get<std::string>("Host");
		std::string user = itree->get<std::string>("User");
		std::string password = itree->get<std::string>("Password");
		int Port = itree->get<int>("Port");
		std::string db = itree->get<std::string>("Database");
		std::string lp = itree->get<std::string>("LicensePlate");
		std::cout << boost::format("host:%s\nuser:%s\npassword:%s\nport:%d\nDatabase:%s\nLicensePlate:%s\n") % host % user % password % Port % db % lp;*/
		CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
						   , itree->get<std::string>("User").c_str()
						   , itree->get<std::string>("Password").c_str()
						   , itree->get<int>("Port")
						   , itree->get<std::string>("Database").c_str()
						   , boost::locale::conv::between(itree->get<std::string>("LicensePlate"), "GBK", "UTF-8").c_str()
						   , task);
		
		std::cout << boost::locale::conv::between(c.GetResults(), "GBK", "UTF-8") << std::endl;
		CAmqpSample amqp;
		std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
		std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
		std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
		//测试
		/*std::stringstream ssr;
		boost::property_tree::write_json(ssr, *otree);
		std::cout << boost::locale::conv::between(ssr.str(), "GBK", "UTF-8") << std::endl;
	}*/
	}
}

void JsonParser::ExtractCorrelationAnalysisfromJson(int task)
{
	CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
		, itree->get<std::string>("User").c_str()
		, itree->get<std::string>("Password").c_str()
		, itree->get<int>("Port")
		, itree->get<std::string>("Database").c_str()
	    , task);
	c.CorrelationAnalysis(boost::locale::conv::between(itree->get<std::string>("LicensePlate"), "GBK", "UTF-8").c_str()
		, itree->get<std::string>("StartAnalysisTime").c_str()
		, itree->get<std::string>("EndAnalysisTime").c_str());

	std::cout << boost::locale::conv::between(c.GetResults(), "GBK", "UTF-8") << std::endl;
	CAmqpSample amqp;
	std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
	std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
	std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
}

void JsonParser::ExtractTrajectoryCollisionfromJson(int task)
{
	CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
		, itree->get<std::string>("User").c_str()
		, itree->get<std::string>("Password").c_str()
		, itree->get<int>("Port")
		, itree->get<std::string>("Database").c_str()
		, task);
	boost::property_tree::ptree tollgatesTree = itree->get_child("TollgateID");
	boost::property_tree::ptree tollgatesList = tollgatesTree.get_child("list");
	std::vector<std::string> tollgates;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tollgatesList)
		tollgates.push_back(v.second.get_value<std::string>());
	c.TrajectoryCollision(tollgates
		, itree->get<std::string>("StartAnalysisTime").c_str()
		, itree->get<std::string>("EndAnalysisTime").c_str());
	CAmqpSample amqp;
	std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
	std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
	std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
}

void JsonParser::ExtractFirstTimeEnterTownfromJson(int task)
{
	std::cout << "in extract first time enter town!" << std::endl;
	CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
		, itree->get<std::string>("User").c_str()
		, itree->get<std::string>("Password").c_str()
		, itree->get<int>("Port")
		, itree->get<std::string>("Database").c_str()
		, task);
	boost::property_tree::ptree tollgatesTree = itree->get_child("TollgateID");
	boost::property_tree::ptree tollgatesList = tollgatesTree.get_child("list");
	std::vector<std::string> tollgates;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tollgatesList)
		tollgates.push_back(v.second.get_value<std::string>());
	c.FirstTimeEnterTown(tollgates
		, itree->get<std::string>("StartAnalysisTime").c_str()
		, itree->get<std::string>("EndAnalysisTime").c_str());
	CAmqpSample amqp;
	std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
	std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
	std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
}

void JsonParser::ExtractActInNightfromJson(int task)
{
	std::cout << "in extract first time enter town!" << std::endl;
	CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
		, itree->get<std::string>("User").c_str()
		, itree->get<std::string>("Password").c_str()
		, itree->get<int>("Port")
		, itree->get<std::string>("Database").c_str()
		, task);
	boost::property_tree::ptree tollgatesTree = itree->get_child("TollgateID");
	boost::property_tree::ptree tollgatesList = tollgatesTree.get_child("list");
	std::vector<std::string> tollgates;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tollgatesList)
		tollgates.push_back(v.second.get_value<std::string>());
	c.ActInNight(tollgates
		, itree->get<std::string>("StartAnalysisTime").c_str()
		, itree->get<std::string>("EndAnalysisTime").c_str()
		, itree->get<std::string>("StartDayTime").c_str()
		, itree->get<std::string>("EndDayTime").c_str()
		, itree->get<std::string>("StartNightTime").c_str()
		, itree->get<std::string>("EndNightTime").c_str());
	CAmqpSample amqp;
	std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
	std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
	std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
}

void JsonParser::ExtractActInNightFfromJson(int task)
{
	std::cout << "in extract first time enter town!" << std::endl;
	CarJudgedAlgorithm c(itree->get<std::string>("Host").c_str()
		, itree->get<std::string>("User").c_str()
		, itree->get<std::string>("Password").c_str()
		, itree->get<int>("Port")
		, itree->get<std::string>("Database").c_str()
		, task);
	boost::property_tree::ptree tollgatesTree = itree->get_child("TollgateID");
	boost::property_tree::ptree tollgatesList = tollgatesTree.get_child("list");
	std::vector<std::string> tollgates;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, tollgatesList)
		tollgates.push_back(v.second.get_value<std::string>());
	c.ActInNightF(tollgates
		, itree->get<std::string>("StartAnalysisTime").c_str()
		, itree->get<std::string>("EndAnalysisTime").c_str()
		, itree->get<std::string>("StartNightTime").c_str()
		, itree->get<std::string>("EndNightTime").c_str()
		, itree->get<int>("Thershold"));
	CAmqpSample amqp;
	std::cout << amqp.Connect("10.3.9.236", 5672, "root", "root", 1) << std::endl;
	std::cout << amqp.CreateExchange("FakePlateReceiver") << std::endl;
	std::cout << amqp.RpcSend("FakePlateReceiver", "FakePlateReceiverRoutingKey", c.GetResults().c_str()) << std::endl;
}

