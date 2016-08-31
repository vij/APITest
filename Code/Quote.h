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

	int CreateQuoteAPI();	//����APIʵ��
	void Login();			//��¼
	int QryCommodity();		//��ѯƷ��
	int QryContract();		//��ѯ��Լ
	int SubscribeQuote();	//����
	int UnSubscribeQuote();	//�˶�

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
	void run();				//�߳�ִ�к���

private:
	ITapQuoteAPI*	m_pAPI;				//APIָ��
	TAPIUINT32		m_uiSessionID;		//sessionID
	bool			m_bIsAPIReady;		//API�Ƿ�Ready���
	int				m_clientID;			//ÿ��SPIʵ��ID
	char			m_filePath[100];	//keyopration��־�ļ�Ŀ¼
	char			m_logFile[100];		//������־�ļ�
	std::list<TapAPIQuoteCommodityInfo> CommodityList;	//��ѯ����Ʒ��list
	std::list<TapAPIQuoteContractInfo> ContractList;	//��ѯ���ĺ�Լlist
};

#endif // QUOTE_H
