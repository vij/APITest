#ifndef QUOTE_H
#define QUOTE_H

#include "Tools.h"
#include "TapQuoteAPI.h"
#include <list>

class Quote : public ITapQuoteAPINotify, public TThread
{
public:
	Quote(void);
	Quote(int clientID);
	~Quote(void);

	int CreateQuoteAPI();	//创建API实例
	void Login();			//登录
	int QryCommodity();		//查询品种
	int QryContract();		//查询合约
	int SubscribeQuote();	//订阅
	int UnSubscribeQuote();	//退订

public:
	void TAP_CDECL OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info);
	void TAP_CDECL OnAPIReady();
	void TAP_CDECL OnDisconnect(TAPIINT32 reasonCode);
	void TAP_CDECL OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info);
	void TAP_CDECL OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info);
	void TAP_CDECL OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info);
	void TAP_CDECL OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info);
	void TAP_CDECL OnRtnQuote(const TapAPIQuoteWhole *info);

private:
	void run();				//线程执行函数

private:
	ITapQuoteAPI*	m_pAPI;				//API指针
	TAPIUINT32		m_uiSessionID;		//sessionID
	bool			m_bIsAPIReady;		//API是否Ready标记
	int				m_clientID;			//每个SPI实例ID
	char			m_filePath[100];	//keyopration日志文件目录
	char			m_logFile[100];		//运行日志文件
	std::list<TapAPIQuoteCommodityInfo> CommodityList;	//查询到的品种list
	std::list<TapAPIQuoteContractInfo> ContractList;	//查询到的合约list
};

#endif // QUOTE_H
