#pragma once
#include "CarJudgedAlgorithm.h"
#include "AmqpSample.h"
struct my_exception :         /* �Զ����쳣�� */
	     virtual std::exception,   /* ��̳У�structĬ��public�̳� */
	     virtual boost::exception  /* ��̳У�structĬ��public�̳� */
{
	   /* ��ʵ�֣�����Ҫʵ�ִ��� */};
 /* �쳣��Ϣ������ */

typedef boost::error_info<struct tag_err_no, int> err_no;
typedef boost::error_info<struct tag_err_str,std::string >err_str;

class JsonParser
{
	
public:
	JsonParser(std::string s);
	JsonParser(const char *json);
	JsonParser(const JsonParser& copy);
	~JsonParser();
private:
	void ReadJsonFromStream(std::string s);
	void ExtractFakePlatesfromJson(int task);
	void ExtractCorrelationAnalysisfromJson(int task);
	void ExtractTrajectoryCollisionfromJson(int task);
	void ExtractFirstTimeEnterTownfromJson(int task);
	void ExtractActInNightfromJson(int task);
	void ExtractActInNightFfromJson(int task);
	void ExtractHiddenVehiclefromJson(int task);
	void ExtractFootholdAnalysis(int task);

	void SendJson(const char *json);

	boost::property_tree::ptree *itree;
	boost::property_tree::ptree *otree;
};

