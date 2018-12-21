#pragma once
#include <amqp.h>
#include<string>

// 最基本的队列通讯模式
class CAmqpSample
{
public:
	CAmqpSample();
	virtual ~CAmqpSample();

public:
	// 建立连接
	bool Connect(
		const char *hostname,// 队列服务器地址，如"192.168.1.66"
		int port,// 队列服务器端口号，如5008
		const char* user,// 队列服务器登录用户名，如“admin”
		const char* password,// 队列服务器登录密码
		int channel//通讯信道，一般为1 
		);

	// 创建交换器
	bool CreateExchange(const char* exchange, const char* type = "fanout", 
		bool bDurable = 1, bool bAutoDelete = 0, bool bInternal = 0);

	// 创建接收对列并绑定
	bool Binding(const char* exchange, const char* bindingkey, const char* queue,
		bool bDurable = 1, bool bAutoDelete = 0, bool bExclusive = 0);

	// return =AMQP_STATUS_OK成功，=AMQP_STATUS_TIMEOUT超时，<0其它错误
	//  nSec:超时时间（秒），是否需要应答（用于ACK与RPC模式中）
	int Consume(long nSec = 1);

	// 获取数据
	int GetDataLength();
	char* GetData();

	// 发送数据，routingkey一般为Binding函数中的bindingkey，但也可用加*的方式进行路由
	bool RpcSend(const char *exchange, const char *routingkey, const char *message);

protected:
	amqp_connection_state_t m_conn;
	int m_nChannel;
	amqp_bytes_t m_queuename;
	bool m_bNoAck;
	bool m_bHasData;
	amqp_envelope_t m_envelope;
};

// Ack模式，即手动回复模式
class CAmqpAck : public CAmqpSample
{
public:
	CAmqpAck();
	virtual ~CAmqpAck();

public:
	// 当需要手动回复时，或者使用下面的RPC模式时，接收到消息后手动回复消息已被正常处理
	bool Ack();

	// 当需要手动回复时，或者使用下面的RPC模式时，接收到消息后手动回复消息未被正常处理，
	// bRequeue是否将消息重新放回到队列，以便其它接收端接收
	bool UnAck(bool bRequeue = true);
};

// Rpc模式
class CAmqpRpc : public CAmqpAck
{
public:
	CAmqpRpc();
	~CAmqpRpc();

	// 在RPC模式下，使用这个绑定函数
	bool Binding(const char* queue, bool bDurable = 1, bool bAutoDelete = 0, bool bExclusive = 0);

	// 在RPC模式下，使用这个发送函数在接收函数中回传数据
	// 本发送中包含调用端信息，在RPC模式下只能用其发送数据给调用者
	// 注意：RPC模式下必须Send数据（只能一次，第二次Send无法接收），否则对方一直等待，而ACK是让队列可以删除本条消息，因此因该先发送数据然后调用ACK()
	bool RpcSend(const char *message);
};



//////////// 以上是rabbitmq的基本功能封装类，可以用其建立连接，用后天线程来接收数据
//////////// 为了方便使用与学习，以下为进一步封装，自带后台线程，用一个启用函数与一个接收回调函数实现通讯



// CAmqpImply类为CAmqpSample与CAmqpRpc的应用类，包含其建立连接与接收数据的基本流程
// 本类实现出错后自动重连接，保证连接不会一直断开
// 本类为接收消息功能，如要发送数据请用CAmqpSample与CAmqpRpc建立局部实例（不建议一直维持，那样连接异常中断后还需要重新连接），
// 然后调用Connect连接，最后调用Send函数发送数据到指定的exchange与routingkey或RPC模式下的queue中
// 或者使用回调函数ReceiveCallBack中的pAmqp参数实例（见下文详述），但注意不要超出回调函数范围

// 接收数据的回调函数定义，sError!=NULL时有错误，nError错误码，
// sData数据（当sError==NULL时有数据），nLen数据长度，
// pAmqp模式下为CAmqpSample对象，ACK模式下为CAmqpAck对象，RPC模式下为CAmqpRpc对象
// 用于调用Ack与UnAck响应，也可使用其Send函数发送数据而不新建连接
//（注意：pAmqp只能在回调函数中Send、Ack与UnAck，出了回调函数后该连接被系统关闭
// 如果要在回调函数外发送数据需要用户用CAmqpSample临时Connect建立连接来Send，但该连接不能Ack与UnAck响应）
// pFuncData传入回调函数的指针，用于在回调函数中调用其它方法
typedef void(*ReceiveCallBack)(const char* sError, int nError, char* sData, int nLen, void* pAmqp, void* pFuncData);

class CAmqpImply
{
public:
	CAmqpImply();
	~CAmqpImply();

public:
	void BeginRecieve(
		// 连接参数
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// 交换机参数
		bool bIsCreateExchange,// 是否需要创建路由（不需要则说明exchange是已有的路由）
		const char* exchangetype,// = "fanout"
		const char* exchange,
		bool bExchangeDurable,
		bool bExchangeAutoDelete,
		bool bExchangeInternal,
		// 接收队列参数
		const char* queue, 
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// 绑定值
		const char* bindingkey,
		// 其它
		ReceiveCallBack func,
		void* pFuncData = NULL
		);

	void BeginRecieve_Ack(
		// 连接参数
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// 交换机参数
		bool bIsCreateExchange,// 是否需要创建路由（不需要则说明exchange是已有的路由）
		const char* exchangetype,// = "topic"
		const char* exchange,
		bool bExchangeDurable,
		bool bExchangeAutoDelete,
		bool bExchangeInternal,
		// 接收队列参数
		const char* queue,
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// 绑定值
		const char* bindingkey,
		// 其它
		ReceiveCallBack func,
		void* pFuncData = NULL
		);

	void BeginRecieve_Rpc(
		// 连接参数
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// 接收队列参数
		const char* queue,
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// 其它
		ReceiveCallBack func,
		void* pFuncData = NULL
		);

public:
	std::string m_hostname;
	int m_port;
	std::string m_user;
	std::string m_password;
	int m_channel;
	bool m_bIsCreateExchange;
	std::string m_exchange;
	std::string m_exchangetype;
	bool m_bExchangeDurable;
	bool m_bExchangeAutoDelete;
	bool m_bExchangeInternal;
	std::string m_queue;
	bool m_bQueueDurable;
	bool m_bQueueAutoDelete;
	bool m_bQueueExclusive;
	std::string m_bindingkey;
	ReceiveCallBack m_func;
	void* m_pFuncData;

	int m_nMQMode;// 0=普通模式，1=ACK模式，2=RPC模式
	bool m_bEndThread;
	bool m_bIsEnd;
	void* m_hThread;
};


