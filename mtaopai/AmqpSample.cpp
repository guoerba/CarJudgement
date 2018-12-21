#include "pch.h"
#include "AmqpSample.h"
#include "amqp_tcp_socket.h"
#include "amqp_time.h"

CAmqpSample::CAmqpSample()
{
	m_conn = NULL;
	m_nChannel = 0;
	m_queuename.bytes = NULL;
	m_bNoAck = true;
	m_bHasData = false;
}

CAmqpSample::~CAmqpSample()
{
	if (m_bHasData)
	{
		amqp_destroy_envelope(&m_envelope);
	}

	if (m_queuename.bytes)
	{
		amqp_bytes_free(m_queuename);
	}

	if (m_conn)
	{
		amqp_channel_close(m_conn, m_nChannel, AMQP_REPLY_SUCCESS);
		amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS);
		amqp_destroy_connection(m_conn);
	}
}

bool CAmqpSample::Connect(const char *hostname, int port, const char* user, const char* password, int channel)
{
	m_conn = amqp_new_connection();

	amqp_socket_t *Mysocket = amqp_tcp_socket_new(m_conn);

	if (!Mysocket)
	{
		return false;
	}

	int status = amqp_socket_open(Mysocket, hostname, port);

	if (status < 0)
	{
		return false;
	}

	amqp_login(m_conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, user, password);

	amqp_channel_open(m_conn, channel);

	m_nChannel = channel;

	return true;
}

bool CAmqpSample::CreateExchange(const char* exchange, const char* type, 
	bool bDurable, bool bAutoDelete, bool bInternal)
{
	amqp_exchange_declare_ok_t* pRes = amqp_exchange_declare(
		m_conn,
		m_nChannel,
		amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(type),
		0,
		bDurable,
		bAutoDelete,
		bInternal,
		amqp_empty_table);

	return true;
}

bool CAmqpSample::Binding(const char * exchange, const char * bindingkey,const char * queue, 
	bool bDurable, bool bAutoDelete, bool bExclusive)
{
	amqp_queue_declare_ok_t *r = amqp_queue_declare(
		m_conn,
		m_nChannel,
		amqp_cstring_bytes(queue),
		0,
		bDurable,
		bExclusive,
		bAutoDelete,
		amqp_empty_table);

	m_queuename = amqp_bytes_malloc_dup(r->queue);

	if (m_queuename.bytes == NULL)
	{
		return false;
	}

	amqp_queue_bind_ok_t* pBindRes = amqp_queue_bind(
		m_conn,
		m_nChannel,
		m_queuename,
		amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(bindingkey),
		amqp_empty_table);

	amqp_basic_qos_ok_t* pqosRes = amqp_basic_qos(m_conn, m_nChannel, 0, 1, 0);// 每次从队列接收1条数据

	amqp_basic_consume_ok_t* pConsumeRes = amqp_basic_consume(
		m_conn,
		m_nChannel,
		m_queuename,
		amqp_empty_bytes,
		0,
		m_bNoAck,
		bExclusive,
		amqp_empty_table);

	return true;
}

int CAmqpSample::Consume(long nSec)
{
	if (m_bHasData)
	{
		amqp_destroy_envelope(&m_envelope);
	}

	m_bHasData = false;

	amqp_frame_t frame;
	amqp_rpc_reply_t ret;

	timeval timeOut;
	timeOut.tv_sec = nSec;
	timeOut.tv_usec = 0;

	amqp_maybe_release_buffers(m_conn);

	ret = amqp_consume_message(m_conn, &m_envelope, &timeOut, 0);

 	if (AMQP_STATUS_TIMEOUT == ret.library_error)
 	{
 		amqp_destroy_envelope(&m_envelope);
 		return AMQP_STATUS_TIMEOUT;
 	}

	if (AMQP_RESPONSE_NORMAL != ret.reply_type)
	{
		if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
			AMQP_STATUS_UNEXPECTED_STATE == ret.library_error)
		{
			int nRes = amqp_simple_wait_frame(m_conn, &frame);
			if (AMQP_STATUS_OK != nRes)
			{
				return nRes;
			}

			if (AMQP_FRAME_METHOD == frame.frame_type)
			{
				switch (frame.payload.method.id)
				{
				case AMQP_BASIC_ACK_METHOD:
					/* if we've turned publisher confirms on, and we've published a message
					* here is a message being confirmed
					*/
					return -0x0205;
				case AMQP_BASIC_RETURN_METHOD:
					/* if a published message couldn't be routed and the mandatory flag was set
					* this is what would be returned. The message then needs to be read.
					*/
				{
					amqp_message_t message;
					ret = amqp_read_message(m_conn, frame.channel, &message, 0);
					if (AMQP_RESPONSE_NORMAL != ret.reply_type)
					{
						return -0x0206;
					}
					amqp_destroy_message(&message);
				}

				return -0x0207;

				case AMQP_CHANNEL_CLOSE_METHOD:
					/* a channel.close method happens when a channel exception occurs, this
					* can happen by publishing to an exchange that doesn't exist for example
					*
					* In this case you would need to open another channel redeclare any queues
					* that were declared auto-delete, and restart any consumers that were attached
					* to the previous channel
					*/
					return -0x0208;

				case AMQP_CONNECTION_CLOSE_METHOD:
					/* a connection.close method happens when a connection exception occurs,
					* this can happen by trying to use a channel that isn't open for example.
					*
					* In this case the whole connection must be restarted.
					*/

					return -0x0209;

				case AMQP_CHANNEL_CLOSE_OK_METHOD:

					return -0x020A;

				default:
					return -0x020B;
				}
			}
		}
		else
		{
			std::string stdError = amqp_error_string2(ret.library_error);
			return -0x020C;
		}
	}
	else
	{
		m_bHasData = true;
		return AMQP_STATUS_OK;
	}

	return AMQP_STATUS_OK;
}

int CAmqpSample::GetDataLength()
{
	return int(m_envelope.message.body.len);
}

char* CAmqpSample::GetData()
{
	return (char*)m_envelope.message.body.bytes;
}

bool CAmqpSample::RpcSend(const char *exchange, const char *routingkey, const char *message)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */

	int nRes = amqp_basic_publish(
		m_conn,
		m_nChannel,
		amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(routingkey),
		0,
		0,
		&props,
		amqp_cstring_bytes(message));

	return AMQP_STATUS_OK == nRes;
}

CAmqpAck::CAmqpAck()
{
	m_bNoAck = false;
}

CAmqpAck::~CAmqpAck()
{

}

bool CAmqpAck::Ack()
{
	if (m_envelope.delivery_tag == NULL)
	{
		return false;
	}

	return amqp_basic_ack(m_conn, m_envelope.channel, m_envelope.delivery_tag, 0) == 0;
}

bool CAmqpAck::UnAck(bool bRequeue)
{
	if (m_envelope.delivery_tag == NULL)
	{
		return false;
	}

	return amqp_basic_nack(m_conn, m_envelope.channel, m_envelope.delivery_tag, 0, bRequeue) == 0;
}



CAmqpRpc::CAmqpRpc()
{

}

CAmqpRpc::~CAmqpRpc()
{

}

bool CAmqpRpc::Binding(const char* queue, bool bDurable, bool bAutoDelete, bool bExclusive)
{
	amqp_queue_declare_ok_t *r = amqp_queue_declare(
		m_conn,
		m_nChannel,
		amqp_cstring_bytes(queue),
		0,
		bDurable,
		bExclusive,
		bAutoDelete,
		amqp_empty_table);

	if (!r)
	{
 		return false;
	}

	m_queuename = amqp_bytes_malloc_dup(r->queue);

	if (!m_queuename.bytes)
	{
		return false;
	}

	amqp_basic_consume_ok_t* pConsumeRes = amqp_basic_consume(
		m_conn,
		m_nChannel,
		m_queuename,
		amqp_empty_bytes,
		0,
		m_bNoAck,
		bExclusive,
		amqp_empty_table);

	return true;
}

bool CAmqpRpc::RpcSend(const char *message)
{
	int nRes = amqp_basic_publish(
		m_conn,
		m_envelope.channel,
		amqp_cstring_bytes(""),
		m_envelope.message.properties.reply_to,
		0,
		0,
		&(m_envelope.message.properties),
		amqp_cstring_bytes(message));

	return AMQP_STATUS_OK == nRes;
}

DWORD  WINAPI  ReceiveThread(void* p)
{
	CAmqpImply* pImply = (CAmqpImply*)p;
	pImply->m_bEndThread = false;
	pImply->m_bIsEnd = false;

	while (true)
	{
		if (pImply->m_bEndThread)
		{
			pImply->m_bIsEnd = true;
			return 0;
		}

		if (pImply->m_nMQMode == 0) // 普通模式
		{
			CAmqpSample qp;
			if (!qp.Connect(pImply->m_hostname.c_str(),
				pImply->m_port,
				pImply->m_user.c_str(),
				pImply->m_password.c_str(),
				pImply->m_channel)
				)
			{
				(*(pImply->m_func))("连接失败，正在重新连接", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			if (pImply->m_bIsCreateExchange)
			{
				if (!qp.CreateExchange(pImply->m_exchange.c_str(), pImply->m_exchangetype.c_str(),
					pImply->m_bExchangeDurable, pImply->m_bExchangeAutoDelete, pImply->m_bExchangeInternal))
				{
					(*(pImply->m_func))("CreateExchange失败，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
					Sleep(1000);
					continue;
				}
			}

			if (!qp.Binding(pImply->m_exchange.c_str(),	pImply->m_bindingkey.c_str(), pImply->m_queue.c_str(),
				pImply->m_bQueueDurable, pImply->m_bQueueAutoDelete, pImply->m_bQueueExclusive))
			{
				(*(pImply->m_func))("Binding错误，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			while (1)
			{
				if (pImply->m_bEndThread)
				{
					pImply->m_bIsEnd = true;
					return 0;
				}

				int nRes = qp.Consume(1);

				if (nRes == AMQP_STATUS_TIMEOUT)
				{
					continue;
				}

				if (nRes == AMQP_STATUS_OK)
				{
					(*(pImply->m_func))(NULL, 0, qp.GetData(), qp.GetDataLength(), &qp, pImply->m_pFuncData);
					continue;
				}

				if (nRes < 0)
				{
					(*(pImply->m_func))("错误码：%d", nRes, NULL, 0, &qp, pImply->m_pFuncData);
					break;
				}
			}
		}

		if (pImply->m_nMQMode == 1) // ACK模式
		{
			CAmqpAck qp;
			if (!qp.Connect(pImply->m_hostname.c_str(),
				pImply->m_port,
				pImply->m_user.c_str(),
				pImply->m_password.c_str(),
				pImply->m_channel)
				)
			{
				(*(pImply->m_func))("连接失败，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			if (pImply->m_bIsCreateExchange)
			{
				if (!qp.CreateExchange(pImply->m_exchange.c_str(), pImply->m_exchangetype.c_str(),
					pImply->m_bExchangeDurable, pImply->m_bExchangeAutoDelete, pImply->m_bExchangeInternal))
				{
					(*(pImply->m_func))("CreateExchange失败，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
					Sleep(1000);
					continue;
				}
			}

			if (!qp.Binding(pImply->m_exchange.c_str(), pImply->m_bindingkey.c_str(), pImply->m_queue.c_str(),
				pImply->m_bQueueDurable, pImply->m_bQueueAutoDelete, pImply->m_bQueueExclusive))
			{
				(*(pImply->m_func))("Binding错误，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			while (1)
			{
				if (pImply->m_bEndThread)
				{
					pImply->m_bIsEnd = true;
					return 0;
				}

				int nRes = qp.Consume(1);

				if (nRes == AMQP_STATUS_TIMEOUT)
				{
					continue;
				}

				if (nRes == AMQP_STATUS_OK)
				{
					(*(pImply->m_func))(NULL, 0, qp.GetData(), qp.GetDataLength(), &qp, pImply->m_pFuncData);
					break;
				}

				if (nRes < 0)
				{
					(*(pImply->m_func))("错误码：%d", nRes, NULL, 0, &qp, pImply->m_pFuncData);
					break;
				}
			}
		}

		if (pImply->m_nMQMode != 0) // Rpc模式
		{
			CAmqpRpc qp;
			if (!qp.Connect(pImply->m_hostname.c_str(),
				pImply->m_port,
				pImply->m_user.c_str(),
				pImply->m_password.c_str(),
				pImply->m_channel)
				)
			{
				(*(pImply->m_func))("连接失败，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			if (!qp.Binding(pImply->m_queue.c_str(),
				pImply->m_bQueueDurable, pImply->m_bQueueAutoDelete, pImply->m_bQueueExclusive))
			{
				(*(pImply->m_func))("Binding错误，正在重新连接！", 0, NULL, 0, &qp, pImply->m_pFuncData);
				Sleep(1000);
				continue;
			}

			while (1)
			{
				if (pImply->m_bEndThread)
				{
					pImply->m_bIsEnd = true;
					return 0;
				}

				int nRes = qp.Consume(1);

				if (nRes == AMQP_STATUS_TIMEOUT)
				{
					continue;
				}

				if (nRes == AMQP_STATUS_OK)
				{
					(*(pImply->m_func))(NULL, 0, qp.GetData(), qp.GetDataLength(), &qp, pImply->m_pFuncData);
					break; //如果是Rpc模式，则每次接收响应后需要重新连接
				}

				if (nRes < 0)
				{
					(*(pImply->m_func))("错误码：%d", nRes, NULL, 0, &qp, pImply->m_pFuncData);
					break;
				}
			}
		}


	}

	pImply->m_bIsEnd = true;
	return 0;
}

CAmqpImply::CAmqpImply()
{
	m_bIsCreateExchange = false;
	m_bIsEnd = true;
	m_hThread = NULL;
}

CAmqpImply::~CAmqpImply()
{
	m_bEndThread = true;

	while (!m_bIsEnd)
	{
		Sleep(50);
	}

	if (!m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CAmqpImply::BeginRecieve(
	const char *hostname, 
	int port, 
	const char* user, 
	const char* password, 
	int channel, 
	bool bIsCreateExchange, 
	const char* exchangetype,/* = "fanout" */ 
	const char* exchange, 
	bool bExchangeDurable,
	bool bExchangeAutoDelete,
	bool bExchangeInternal,
	const char* queue,
	bool bQueueDurable,
	bool bQueueAutoDelete,
	bool bQueueExclusive,
	const char* bindingkey,
	ReceiveCallBack func,
	void* pFuncData)
{
	m_hostname = hostname;
	m_port = port;
	m_user = user;
	m_password = password;
	m_channel = channel;
	m_bIsCreateExchange = bIsCreateExchange;
	m_exchange = exchange;
	m_exchangetype = exchangetype;
	m_bExchangeDurable = bExchangeDurable;
	m_bExchangeAutoDelete = bExchangeAutoDelete;
	m_bExchangeInternal = bExchangeInternal;
	m_queue = queue;
	m_bQueueDurable = bQueueDurable;
	m_bQueueAutoDelete = bQueueAutoDelete;
	m_bQueueExclusive = bQueueExclusive;
	m_bindingkey = bindingkey;
	m_func = func;
	m_pFuncData = pFuncData;

	m_nMQMode = 0;

	m_hThread = CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
}

void CAmqpImply::BeginRecieve_Ack(
	const char *hostname, 
	int port, const char* user, 
	const char* password, 
	int channel, 
	bool bIsCreateExchange,/* 是否需要创建路由（不需要则说明exchange是已有的路由） */ 
	const char* exchangetype,/* = "fanout" */ 
	const char* exchange, 
	bool bExchangeDurable,
	bool bExchangeAutoDelete,
	bool bExchangeInternal,
	const char* queue,
	bool bQueueDurable,
	bool bQueueAutoDelete,
	bool bQueueExclusive,
	const char* bindingkey,
	ReceiveCallBack func, 
	void* pFuncData /*= NULL */)
{
	m_hostname = hostname;
	m_port = port;
	m_user = user;
	m_password = password;
	m_channel = channel;
	m_bIsCreateExchange = bIsCreateExchange;
	m_exchange = exchange;
	m_exchangetype = exchangetype;
	m_bExchangeDurable = bExchangeDurable;
	m_bExchangeAutoDelete = bExchangeAutoDelete;
	m_bExchangeInternal = bExchangeInternal;
	m_queue = queue;
	m_bQueueDurable = bQueueDurable;
	m_bQueueAutoDelete = bQueueAutoDelete;
	m_bQueueExclusive = bQueueExclusive;
	m_bindingkey = bindingkey;
	m_func = func;
	m_pFuncData = pFuncData;

	m_nMQMode = 1;

	m_hThread = CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
}

void CAmqpImply::BeginRecieve_Rpc(
	const char *hostname, 
	int port, 
	const char* user, 
	const char* password, 
	int channel, 
	const char* queue, 
	bool bQueueDurable,
	bool bQueueAutoDelete,
	bool bQueueExclusive,
	ReceiveCallBack func,
	void* pFuncData)
{
	m_hostname = hostname;
	m_port = port;
	m_user = user;
	m_password = password;
	m_channel = channel;
	m_queue = queue;
	m_bQueueDurable = bQueueDurable;
	m_bQueueAutoDelete = bQueueAutoDelete;
	m_bQueueExclusive = bQueueExclusive;
	m_func = func;
	m_pFuncData = pFuncData;

	m_nMQMode = 2;

	m_hThread = CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
}


