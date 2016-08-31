#include "Quote.h"
#include "TapAPIError.h"
#include "QuoteConfig.h"
#include <stdio.h>
#include <iostream>
#include <string.h>

#pragma warning (disable:4996)

//检查错误码，如果不是0，则输出对应函数名称和错误码
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

//创建API实例
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

//登录函数，包括设置回调、设置ip端口、执行登录三步
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

//登录执行结果
void TAP_CDECL Quote::OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info)
{
	if (TAPIERROR_SUCCEED == errorCode)
	{
		std::cout << "clientID " << m_clientID << " 登录成功，等待API初始化..." << std::endl;
	}
	else
	{
		std::cout << "clientID " << m_clientID << " 登录失败，错误码:" << errorCode << std::endl;
	}
}

//API准备就绪，收到此回调时，开启新线程，查询品种、查询合约、订阅合约
void TAP_CDECL Quote::OnAPIReady()
{
	Start();
	std::cout << "clientID " << m_clientID << " API初始化完成" << std::endl;
}

//连接断开，开启新线程释放资源
void TAP_CDECL Quote::OnDisconnect(TAPIINT32 reasonCode)
{
	Start();
	std::cout << "clientID " << m_clientID << " API断开,断开原因:" << reasonCode << std::endl;
}

//查询品种
int Quote::QryCommodity()
{
	m_uiSessionID = 0;

	int res = m_pAPI->QryCommodity(&m_uiSessionID);

	CHECK_ERROR(res);
	return res;
}

//查询品种应答，将查询到的品种存入CommodityList
void TAP_CDECL Quote::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info)
{
	CHECK_ERROR(errorCode);
	CommodityList.push_back(*info);
}

//查询合约
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

//查询合约应答，将查询到的合约存入ContractList
void TAP_CDECL Quote::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info)
{
	CHECK_ERROR(errorCode);
	if (NULL != info)
		ContractList.push_back(*info);
}

//订阅合约，将查询到的合约全部订阅
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

//订阅应答
void TAP_CDECL Quote::OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info)
{
	if (TAPIERROR_SUCCEED == errorCode)
	{
		if (NULL != info)
		{
			char msg[1024];
			sprintf(msg, "client %d 行情订阅成功: %s %s %c %s %s %lf\n", m_clientID, info->DateTimeStamp, info->Contract.Commodity.ExchangeNo,
				info->Contract.Commodity.CommodityType, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->QLastPrice);
			WriteEventLog(msg, m_logFile);
		}
	}
	else
	{
		std::cout << "clientID " << m_clientID << " 行情订阅失败，错误码：" << errorCode << std::endl;
	}
}

//退订合约
int Quote::UnSubscribeQuote()
{
	return 0;
}

//退订应答
void TAP_CDECL Quote::OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info)
{
	std::cout << __FUNCTION__ << " is called." << std::endl;
}

//行情变化通知
void TAP_CDECL Quote::OnRtnQuote(const TapAPIQuoteWhole *info)
{
	if (NULL != info)
	{
		char msg[1024];
		sprintf(msg, "client %d 行情更新: %s %s %c %s %s %lf\n", m_clientID, info->DateTimeStamp, info->Contract.Commodity.ExchangeNo,
			info->Contract.Commodity.CommodityType, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->QLastPrice);
		WriteEventLog(msg, m_logFile);
	}
}

//线程执行，若登录时执行，查询品种、合约
//sleep2秒，等待合约查询完毕后订阅
//若连接断开时执行，释放资源
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