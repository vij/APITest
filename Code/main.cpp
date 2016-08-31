#include "Quote.h"
#include <iostream>

#define MAX_CLIENT 3

//程序开始执行，创建多个实例
void StartAPI()
{
	std::cout << GetTapQuoteAPIVersion() << std::endl;

	Quote* QuoteClient[MAX_CLIENT];

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		QuoteClient[i] = new Quote(i + 1);

		if (!QuoteClient[i]->CreateQuoteAPI())
		{
			QuoteClient[i]->Login();
			CommonSleep(500);
		}
	}

	while (true)
	{
		CommonSleep(30000);
	}
}

int main()
{
	StartAPI();

	return 0;
}