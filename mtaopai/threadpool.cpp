#include "pch.h"
#include "threadpool.h"


threadpool::threadpool()
{
	boost::thread t = boost::thread();
	boost::thread_group threadpool;
	
	
}


threadpool::~threadpool()
{
}
