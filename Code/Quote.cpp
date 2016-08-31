#include "Quote.h"
#include "TapAPIError.h"
#include "QuoteConfig.h"
#include <stdio.h>
#include <iostream>
#include <string.h>

#pragma warning (disable:4996)

//�������룬�������0���������Ӧ�������ƺʹ�����
#define CHECK_ERROR(errorCode)\
if (TAPIERROR_SUCCEED != errorCode)\
{\
	std::cout << "Client " << m_clientID << " " << __FUNCTION__ << " error: " << errorCode << std::endl; \
}

Quote::Quote(void) :
m_pAPI(NULL),
m_bIsAPIReady(false),
m_uiSessionID(0)
{
}

Quote::Quote(int clientID) :
m_pAPI(NULL),
m_bIsAPIReady(false),
m_clientID(clientID),
m_uiSessionID(0)
{
	char dt[21];
	Date(dt);
	sprintf(m_filePath, "./QuoteAPILog%d", m_clientID);
	sprintf(m_logFile, "%s/%s.log", m_filePath, dt);
}

Quote::~Quote(void)
{
	FreeTapQuoteAPI(m_pAPI);
}

//����APIʵ��
int Quote::CreateQuoteAPI()
{
	TAPIINT32 iResult = TAPIERROR_SUCCEED;
	TapAPIApplicationInfo stAppInfo;
	strncpy(stAppInfo.AuthCode, DEFAULT_AUTHCODE, sizeof(DEFAULT_AUTHCODE));
	strncpy(stAppInfo.KeyOperationLogPath, m_filePath, sizeof(m_filePath));

	m_pAPI = CreateTapQuoteAPI(&stAppInfo, iResult);

	if (NULL == m_pAPI)
	{
		std::cout << "CreateAPI field on clientID " << m_clientID << " and the error code is " << iResult << std::endl;
		return iResult;
	}
	return 0;
}

//��¼�������������ûص�������ip�˿ڡ�ִ�е�¼����
void Quote::Login()
{
	if (NULL == m_pAPI)
	{
		std::cout << "Error: m_pAPI is NULL, clientID is" << m_clientID << std::endl;
		return;
	}

	int res = m_pAPI->SetAPINotify(this);

	res = m_pAPI->SetHostAddress(DEFAULT_IP, DEFAULT_PORT);

	TapAPIQuoteLoginAuth stLoginAuth;
	memset(&stLoginAuth, 0, sizeof(stLoginAuth));
	strncpy(stLoginAuth.UserNo, DEFAULT_USERNAME, sizeof(DEFAULT_USERNAME));
	strncpy(stLoginAuth.Password, DEFAULT_PASSWORD, sizeof(DEFAULT_PASSWORD));
	stLoginAuth.ISModifyPassword = APIYNFLAG_NO;
	stLoginAuth.ISDDA = APIYNFLAG_NO;

	res = m_pAPI->Login(&stLoginAuth);

	CHECK_ERROR(res);
}

//��¼ִ�н��
void TAP_CDECL Quote::OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info)
{
	if (TAPIERROR_SUCCEED == errorCode)
	{
		std::cout << "clientID " << m_clientID << " ��¼�ɹ����ȴ�API��ʼ��..." << std::endl;
	}
	else
	{
		std::cout << "clientID " << m_clientID << " ��¼ʧ�ܣ�������:" << errorCode << std::endl;
	}
}

//API׼���������յ��˻ص�ʱ���������̣߳���ѯƷ�֡���ѯ��Լ�����ĺ�Լ
void TAP_CDECL Quote::OnAPIReady()
{
	Start();
	std::cout << "clientID " << m_clientID << " API��ʼ�����" << std::endl;
}

//���ӶϿ����������߳��ͷ���Դ
void TAP_CDECL Quote::OnDisconnect(TAPIINT32 reasonCode)
{
	Start();
	std::cout << "clientID " << m_clientID << " API�Ͽ�,�Ͽ�ԭ��:" << reasonCode << std::endl;
}

//��ѯƷ��
int Quote::QryCommodity()
{
	m_uiSessionID = 0;

	int res = m_pAPI->QryCommodity(&m_uiSessionID);

	CHECK_ERROR(res);
	return res;
}

//��ѯƷ��Ӧ�𣬽���ѯ����Ʒ�ִ���CommodityList
void TAP_CDECL Quote::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info)
{
	CHECK_ERROR(errorCode);
	CommodityList.push_back(*info);
}

//��ѯ��Լ
int Quote::QryContract()
{
	m_uiSessionID = 0;
	TapAPICommodity stContract;
	memset(&stContract, 0, sizeof(stContract));
	strncpy(stContract.ExchangeNo, "ZCE", sizeof(stContract.ExchangeNo));
	stContract.CommodityType = 'F';
	strncpy(stContract.CommodityNo, "SR", sizeof(stContract.CommodityNo));

	int res = m_pAPI->QryContract(&m_uiSessionID, &stContract);

	CHECK_ERROR(res);
	return res;
}

//��ѯ��ԼӦ�𣬽���ѯ���ĺ�Լ����ContractList
void TAP_CDECL Quote::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info)
{
	CHECK_ERROR(errorCode);
	if (NULL != info)
		ContractList.push_back(*info);
}

//���ĺ�Լ������ѯ���ĺ�Լȫ������
int Quote::SubscribeQuote()
{
	m_uiSessionID = 0;
	for (std::list<TapAPIQuoteContractInfo>::iterator iter = ContractList.begin(); ContractList.end() != iter; ++iter)
	{
		int res = m_pAPI->SubscribeQuote(&m_uiSessionID, &(iter->Contract));
		CHECK_ERROR(res);
	}
	return 0;
}

//����Ӧ��
void TAP_CDECL Quote::OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info)
{
	if (TAPIERROR_SUCCEED == errorCode)
	{
		if (NULL != info)
		{
			char msg[1024];
			sprintf(msg, "client %d ���鶩�ĳɹ�: %s %s %c %s %s %lf\n", m_clientID, info->DateTimeStamp, info->Contract.Commodity.ExchangeNo,
				info->Contract.Commodity.CommodityType, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->QLastPrice);
			WriteEventLog(msg, m_logFile);
		}
	}
	else
	{
		std::cout << "clientID " << m_clientID << " ���鶩��ʧ�ܣ������룺" << errorCode << std::endl;
	}
}

//�˶���Լ
int Quote::UnSubscribeQuote()
{
	return 0;
}

//�˶�Ӧ��
void TAP_CDECL Quote::OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info)
{
	std::cout << __FUNCTION__ << " is called." << std::endl;
}

//����仯֪ͨ
void TAP_CDECL Quote::OnRtnQuote(const TapAPIQuoteWhole *info)
{
	if (NULL != info)
	{
		char msg[1024];
		sprintf(msg, "client %d �������: %s %s %c %s %s %lf\n", m_clientID, info->DateTimeStamp, info->Contract.Commodity.ExchangeNo,
			info->Contract.Commodity.CommodityType, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->QLastPrice);
		WriteEventLog(msg, m_logFile);
	}
}

//�߳�ִ�У�����¼ʱִ�У���ѯƷ�֡���Լ
//sleep2�룬�ȴ���Լ��ѯ��Ϻ���
//�����ӶϿ�ʱִ�У��ͷ���Դ
void Quote::run()
{
	if (!m_bIsAPIReady)
	{
		do
		{
			if (QryCommodity())
				break;
			if (QryContract())
				break;
			CommonSleep(2000);
			if (SubscribeQuote())
				break;
		} while (false);
		m_bIsAPIReady = true;
	}
	else
	{
		FreeTapQuoteAPI(m_pAPI);
		m_bIsAPIReady = false;
	}
}