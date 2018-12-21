#pragma once
#include <amqp.h>
#include<string>

// ������Ķ���ͨѶģʽ
class CAmqpSample
{
public:
	CAmqpSample();
	virtual ~CAmqpSample();

public:
	// ��������
	bool Connect(
		const char *hostname,// ���з�������ַ����"192.168.1.66"
		int port,// ���з������˿ںţ���5008
		const char* user,// ���з�������¼�û������硰admin��
		const char* password,// ���з�������¼����
		int channel//ͨѶ�ŵ���һ��Ϊ1 
		);

	// ����������
	bool CreateExchange(const char* exchange, const char* type = "fanout", 
		bool bDurable = 1, bool bAutoDelete = 0, bool bInternal = 0);

	// �������ն��в���
	bool Binding(const char* exchange, const char* bindingkey, const char* queue,
		bool bDurable = 1, bool bAutoDelete = 0, bool bExclusive = 0);

	// return =AMQP_STATUS_OK�ɹ���=AMQP_STATUS_TIMEOUT��ʱ��<0��������
	//  nSec:��ʱʱ�䣨�룩���Ƿ���ҪӦ������ACK��RPCģʽ�У�
	int Consume(long nSec = 1);

	// ��ȡ����
	int GetDataLength();
	char* GetData();

	// �������ݣ�routingkeyһ��ΪBinding�����е�bindingkey����Ҳ���ü�*�ķ�ʽ����·��
	bool RpcSend(const char *exchange, const char *routingkey, const char *message);

protected:
	amqp_connection_state_t m_conn;
	int m_nChannel;
	amqp_bytes_t m_queuename;
	bool m_bNoAck;
	bool m_bHasData;
	amqp_envelope_t m_envelope;
};

// Ackģʽ�����ֶ��ظ�ģʽ
class CAmqpAck : public CAmqpSample
{
public:
	CAmqpAck();
	virtual ~CAmqpAck();

public:
	// ����Ҫ�ֶ��ظ�ʱ������ʹ�������RPCģʽʱ�����յ���Ϣ���ֶ��ظ���Ϣ�ѱ���������
	bool Ack();

	// ����Ҫ�ֶ��ظ�ʱ������ʹ�������RPCģʽʱ�����յ���Ϣ���ֶ��ظ���Ϣδ����������
	// bRequeue�Ƿ���Ϣ���·Żص����У��Ա��������ն˽���
	bool UnAck(bool bRequeue = true);
};

// Rpcģʽ
class CAmqpRpc : public CAmqpAck
{
public:
	CAmqpRpc();
	~CAmqpRpc();

	// ��RPCģʽ�£�ʹ������󶨺���
	bool Binding(const char* queue, bool bDurable = 1, bool bAutoDelete = 0, bool bExclusive = 0);

	// ��RPCģʽ�£�ʹ��������ͺ����ڽ��պ����лش�����
	// �������а������ö���Ϣ����RPCģʽ��ֻ�����䷢�����ݸ�������
	// ע�⣺RPCģʽ�±���Send���ݣ�ֻ��һ�Σ��ڶ���Send�޷����գ�������Է�һֱ�ȴ�����ACK���ö��п���ɾ��������Ϣ���������ȷ�������Ȼ�����ACK()
	bool RpcSend(const char *message);
};



//////////// ������rabbitmq�Ļ������ܷ�װ�࣬�������佨�����ӣ��ú����߳�����������
//////////// Ϊ�˷���ʹ����ѧϰ������Ϊ��һ����װ���Դ���̨�̣߳���һ�����ú�����һ�����ջص�����ʵ��ͨѶ



// CAmqpImply��ΪCAmqpSample��CAmqpRpc��Ӧ���࣬�����佨��������������ݵĻ�������
// ����ʵ�ֳ�����Զ������ӣ���֤���Ӳ���һֱ�Ͽ�
// ����Ϊ������Ϣ���ܣ���Ҫ������������CAmqpSample��CAmqpRpc�����ֲ�ʵ����������һֱά�֣����������쳣�жϺ���Ҫ�������ӣ���
// Ȼ�����Connect���ӣ�������Send�����������ݵ�ָ����exchange��routingkey��RPCģʽ�µ�queue��
// ����ʹ�ûص�����ReceiveCallBack�е�pAmqp����ʵ��������������������ע�ⲻҪ�����ص�������Χ

// �������ݵĻص��������壬sError!=NULLʱ�д���nError�����룬
// sData���ݣ���sError==NULLʱ�����ݣ���nLen���ݳ��ȣ�
// pAmqpģʽ��ΪCAmqpSample����ACKģʽ��ΪCAmqpAck����RPCģʽ��ΪCAmqpRpc����
// ���ڵ���Ack��UnAck��Ӧ��Ҳ��ʹ����Send�����������ݶ����½�����
//��ע�⣺pAmqpֻ���ڻص�������Send��Ack��UnAck�����˻ص�����������ӱ�ϵͳ�ر�
// ���Ҫ�ڻص������ⷢ��������Ҫ�û���CAmqpSample��ʱConnect����������Send���������Ӳ���Ack��UnAck��Ӧ��
// pFuncData����ص�������ָ�룬�����ڻص������е�����������
typedef void(*ReceiveCallBack)(const char* sError, int nError, char* sData, int nLen, void* pAmqp, void* pFuncData);

class CAmqpImply
{
public:
	CAmqpImply();
	~CAmqpImply();

public:
	void BeginRecieve(
		// ���Ӳ���
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// ����������
		bool bIsCreateExchange,// �Ƿ���Ҫ����·�ɣ�����Ҫ��˵��exchange�����е�·�ɣ�
		const char* exchangetype,// = "fanout"
		const char* exchange,
		bool bExchangeDurable,
		bool bExchangeAutoDelete,
		bool bExchangeInternal,
		// ���ն��в���
		const char* queue, 
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// ��ֵ
		const char* bindingkey,
		// ����
		ReceiveCallBack func,
		void* pFuncData = NULL
		);

	void BeginRecieve_Ack(
		// ���Ӳ���
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// ����������
		bool bIsCreateExchange,// �Ƿ���Ҫ����·�ɣ�����Ҫ��˵��exchange�����е�·�ɣ�
		const char* exchangetype,// = "topic"
		const char* exchange,
		bool bExchangeDurable,
		bool bExchangeAutoDelete,
		bool bExchangeInternal,
		// ���ն��в���
		const char* queue,
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// ��ֵ
		const char* bindingkey,
		// ����
		ReceiveCallBack func,
		void* pFuncData = NULL
		);

	void BeginRecieve_Rpc(
		// ���Ӳ���
		const char *hostname,
		int port,
		const char* user,
		const char* password,
		int channel,
		// ���ն��в���
		const char* queue,
		bool bQueueDurable,
		bool bQueueAutoDelete,
		bool bQueueExclusive,
		// ����
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

	int m_nMQMode;// 0=��ͨģʽ��1=ACKģʽ��2=RPCģʽ
	bool m_bEndThread;
	bool m_bIsEnd;
	void* m_hThread;
};


