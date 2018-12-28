#pragma once
#include "CarJudgedAlgorithm.h"
#include "AmqpSample.h"
struct my_exception :         /* 自定义异常类 */
	     virtual std::exception,   /* 虚继承，struct默认public继承 */
	     virtual boost::exception  /* 虚继承，struct默认public继承 */
{
	   /* 空实现，不需要实现代码 */};
 /* 异常信息的类型 */

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

	boost::property_tree::ptree *itree;
	boost::property_tree::ptree *otree;
};

