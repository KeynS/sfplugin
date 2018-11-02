#include "main.h"

enum RequestType
{
	GET,
	POST
};

struct RequestData
{
	RequestType type;
	char *site;
	char *file;
	char message[255];

	RequestData(char message[255], RequestType type = POST, char *site = "srp-logs.ru", char *file = "loges.php")
	{
		strcpy_s(this->message, message);
		this->type = type;
		this->site = site;
		this->file = file;
	}
};

#pragma region Functions
char* UTF8_to_ANSI(const char* szU8)
{
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);
	wszString[wcsLen] = '\0';

	int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* szAnsi = new char[ansiLen + 1];
	::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), szAnsi, ansiLen, NULL, NULL);
	szAnsi[ansiLen] = '\0';

	return szAnsi;
}

char* ANSI_to_UTF8(const char* ansi)
{
	int inlen = ::MultiByteToWideChar(CP_ACP, NULL, ansi, strlen(ansi), NULL, 0);
	wchar_t* wszString = new wchar_t[inlen + 1];
	::MultiByteToWideChar(CP_ACP, NULL, ansi, strlen(ansi), wszString, inlen);
	wszString[inlen] = '\0';

	int outlen = ::WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* utf8 = new char[outlen + 1];
	::WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), utf8, outlen, NULL, NULL);
	utf8[outlen] = '\0';

	return utf8;
}

void Say(char *text, ...)
{
	va_list ap;
	char    tmp[128];
	memset(tmp, 0, 128);

	va_start(ap, text);
	vsprintf(tmp, text, ap);
	va_end(ap);

	SF->getSAMP()->getPlayers()->pLocalPlayer->Say(tmp);
}

void StringFindEnd(char *string, char *search, char stop_symbol, char output[])
{
	char	*find = strstr(string, search);
	int		start_pos = strlen(search);
	int		pos;

	if (strstr(string, search))
	{
		for (pos = 0; pos < 100; pos++)
		{
			if (find[start_pos + pos] == stop_symbol)
				break;

			output[pos] = find[start_pos + pos];
		}

		output[pos] = '\0';
	}
	else
		strcpy(output, "");
}

void StringFind(char *string, char *search, char *stop_symbol, char output[])
{
	char	*find = strstr(string, search);
	int		start_pos = strlen(search);
	int		pos;

	if (strstr(string, search))
	{
		for (pos = 0; pos < 100; pos++)
		{
			if (find[start_pos + pos] == *stop_symbol)
				break;

			output[pos] = find[start_pos + pos];
		}

		output[pos] = '\0';
	}
	else
		strcpy(output, "");
}

void StringReplace(char str[], char a[], char b[], char output[])
{
	int        i;
	int        j;
	int        pos = 0;

	for (i = 0; str[i]; i++)
	{
		for (j = 0; str[i + j] && a[j]; j++)
			if (str[i + j] != a[j])
				break;

		if (!a[j])
		{
			i += j - 1;
			for (j = 0; b[j]; j++)
				output[pos++] = b[j];
		}
		else
			output[pos++] = str[i];

		output[pos] = '\0';
	}
}

void showDialog(int send, int dialogID, int typedialog, char *caption, char *text, char *button1, char *button2)
{
	uint32_t func = SF->getSAMP()->getSAMPAddr() + 0x6B9C0;
	uint32_t data = SF->getSAMP()->getSAMPAddr() + 0x21A0B8;

	__asm mov eax, dword ptr[data]
		__asm mov ecx, dword ptr[eax]
		__asm push send
	__asm push button2
	__asm push button1
	__asm push text
	__asm push caption
	__asm push typedialog
	__asm push dialogID
	__asm call func
}

void SendRequest(RequestData *data, char(&result)[2048])
{
	char fileAndMessage[256];
	DWORD size;

	memset(&result, NULL, sizeof(result));

	HINTERNET hSession = InternetOpen("App", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (hSession != NULL)
	{
		HINTERNET hConnect = InternetConnect(hSession, data->site, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

		if (hConnect != NULL)
		{
			if (data->type == RequestType::GET)
				sprintf_s(fileAndMessage, "%s?%s", data->file, data->message);

			HINTERNET hRequest = HttpOpenRequest(hConnect, (data->type == RequestType::POST ? "POST" : "GET"), (data->type == RequestType::POST ? data->file : fileAndMessage), NULL, NULL, 0, 0, 1);

			if (hRequest != NULL)
			{
				LPCSTR header;

				header = "Accept: */*";
				HttpAddRequestHeaders(hRequest, header, strlen(header), HTTP_ADDREQ_FLAG_ADD);
				header = "Content-Type: application/x-www-form-urlencoded;";
				HttpAddRequestHeaders(hRequest, header, strlen(header), HTTP_ADDREQ_FLAG_ADD);

				if (HttpSendRequest(hRequest, NULL, 0, (data->type == RequestType::POST ? data->message : NULL), (data->type == RequestType::POST ? strlen(data->message) : NULL)))
				{
					if (InternetReadFile(hRequest, result, sizeof(result), &size))
						result[size] = 0;
				}
			}

			InternetCloseHandle(hRequest);
		}

		InternetCloseHandle(hConnect);
	}

	InternetCloseHandle(hSession);
}
#pragma endregion

#pragma region Thread
void Blacklist(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] {01b729}%s {FFFFFF}добавлен в ЧС с причиной: {01b729}%s", Set.nickname, Set.reason);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] {01b729}%s {FFFFFF}уже находится в ЧС", Set.nickname);
	}
}

void Ranks(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Повышение/Понижения{FFFFFF}] {01b729}%s {FFFFFF}добавлен в лог повышений / понижений", Set.nickname);
	}
}

void Invite(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Приём{FFFFFF}] {01b729}%s {FFFFFF}добавлен в лог приёма", Set.nickname);
	}
}

void addcont(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Контракт{FFFFFF}] {01b729}%s {FFFFFF}добавлен в список контрактников", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Контракт{FFFFFF}] {01b729}%s {FFFFFF}уже находится в списке контрактников", Set.nickname);
	}
}

void delcont(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Контракт{FFFFFF}] {01b729}%s {FFFFFF}успешно удален из списка контрактников.", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Контракт{FFFFFF}] {01b729}%s {FFFFFF}не находился в списке контрактников.", Set.nickname);
	}
}

void Uninvite(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Увольнение{FFFFFF}] {01b729}%s {FFFFFF}добавлен в лог увольнений", Set.nickname);
	}
}

void Check(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Чёрный список{FFFFFF}] {01b729}%s{FFFFFF} не состоит в ЧС", Set.nickname);
	}
	else
	{
		wchar_t        *wsValid;

		wsValid = new wchar_t[strlen(result) + 1];

		MultiByteToWideChar(CP_UTF8, 0, result, -1, wsValid, strlen(result) + 1);
		WideCharToMultiByte(CP_ACP, NULL, wsValid, -1, result, strlen(result) + 1, NULL, NULL);

		delete[] wsValid;

		char nick[24];
		char reason[255];

		StringFind(result, "name=", "][", nick);
		StringFind(result, "][", "]", reason);

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] Игрок:{01b729} %s {ffffff} || Внёс:{01b729} %s {ffffff}", Set.nickname, nick);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] Причина: {01b729}%s", reason);
	}
}

void DellBlackList(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	StringFind(request_data->message, "SEND=чс&name=", "&officer", Set.nickname);

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] {01b729}%s {FFFFFF}успешно удален из черного списка!", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}ЧС{FFFFFF}] {01b729}%s {FFFFFF}не находился в черном списке!", Set.nickname);
	}
}


void CheckRang(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	char check[2048];

	//char *check = new char [strlen(result)];

	StringReplace(result, "\\n", "\n", check);
	StringReplace(check, "\\t", "\t", result);

	//delete [] check;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else
	{
		showDialog(0, 5, 4, "{095200}LVa Homecoming by M.Friedmann v0.6", result, "Выбрать", "Закрыть");
	}
}

void CheckVig(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 0"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговоры{FFFFFF}] У {01b729}%s {FFFFFF}нет выговоров!", Set.nickname);
	}
	else
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговоры{FFFFFF}] %s", result);
	}
}

void Vig(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;
}

void VigAdd(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];
	char send[255];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(send, "SEND=выговоры&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(send);
		_beginthread(Vig, NULL, (void *)request_data);
	}
	else if (strstr(result, "code: 2"))
	{
		StringReplace(Set.send, "add", "logs", send);

		RequestData *request_data = new RequestData(send);
		_beginthread(Vig, NULL, (void *)request_data);
	}

}

void VigDel(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];
	char send[255];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(send, "SEND=выговоры&name=%s&sub_send=logs&reason=удаление с 1 до 0 выговоров&officer=%s&proverka=1&kolvo= ", Set.nickname, Set.officer);
		StringFind(send, "&reason=", "&officer=", Set.send);
		sprintf_s(send, "SEND=выговоры&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo= ", Set.nickname, ANSI_to_UTF8(Set.send), Set.officer);
		RequestData *request_data = new RequestData(send);
		_beginthread(Vig, NULL, (void *)request_data);
	}
	else if (strstr(result, "code: 2"))
	{
		int res = Set.Code + 1;
		sprintf_s(send, "удаление с %d до %d выговоров", res, Set.Code);
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo= ", Set.nickname, ANSI_to_UTF8(send), Set.officer);

		RequestData *request_data = new RequestData(Set.send);
		_beginthread(Vig, NULL, (void *)request_data);
	}

}

void CheckersVig(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 0"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - выговор внесён.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] Причина: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}1{ffffff} из {01b729}3");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=2", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - выговор внесён.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] Причина: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}2{ffffff} из {01b729}3");
	}
	else if (strstr(result, "code: 2"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=3", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - выговор внесён.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] Причина: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}3{ffffff} из {01b729}3");
	}
	else if (strstr(result, "code: 3"))
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] У {01b729}%s {FFFFFF}3 из 3 выговоров", Set.nickname);
	}
}

void DelVigCheck(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Не удалось соедениться с сайтом");
	}
	else if (strstr(result, "code: 0"))
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] У  {01b729}%s {ffffff} выговоров нет!", Set.nickname);
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=del&kolvo= &officer=%s&proverka=1", Set.nickname, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - статистика выговоров обновлена.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}0{ffffff} из {01b729}3");
	}
	else if (strstr(result, "code: 2"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=del&kolvo=1&officer=%s&proverka=1", Set.nickname, Set.officer);
		Set.Code = 1;
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - статистика выговоров обновлена.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}1{ffffff} из {01b729}3");
	}
	else if (strstr(result, "code: 3"))
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=del&kolvo=2&officer=%s&proverka=1", Set.nickname, Set.officer);
		Set.Code = 2;
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}Выговор{FFFFFF}] {01b729}%s {FFFFFF} - статистика выговоров обновлена.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}Выговор{FFFFFF}] Количество выговоров : {01b729}2{ffffff} из {01b729}3");
	}
}
#pragma endregion

#pragma region CMD
void CALLBACK blacklist(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/blacklist [ник/ид] [причина]");
	}

	strcpy_s(Set.reason, (szParams += strlen(Set.nickname) + 1));

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3 && strlen(Set.reason) > 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=чс&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(Blacklist, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/blacklist [ник/ид] [причина]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(Set.reason) > 3)
	{
		sprintf_s(Set.send, "SEND=чс&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(Blacklist, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/blacklist [ник/ид] [причина]");
	}
}

void CALLBACK delcont(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delcont [ник/ид]");
	}

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=контракт&name=%s&sub_send=delete&officer=%s&proverka=1", Set.nickname, Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(delcont, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delcont [ник/ид]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		sprintf_s(Set.send, "SEND=контракт&name=%s&sub_send=delete&officer=%s&proverka=1", Set.nickname, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(delcont, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delcont [ник/ид]");
	}
}

void CALLBACK addcont(std::string param)
{
	char date[40];
	char date1[10], date2[10], date3[15];
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s %s", &Set.nickname, &date) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addcont [ник/ид] [дата (Пример: 27.01.2018)]");
	}

	StringFindEnd(date, "", '.', date1);
	StringFindEnd(date, ".", '.', date2);
	strcpy_s(date3, (szParams += strlen(Set.nickname) + 7));

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3 && strlen(date) > 9)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=контракт&name=%s&sub_send=add&data=%s-%s-%s&officer=%s&proverka=1", Set.nickname, date3, date2, date1, Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(addcont, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addcont [ник/ид] [дата (Пример: 27.01.2018)]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(date) > 3)
	{
		sprintf_s(Set.send, "SEND=контракт&name=%s&sub_send=add&data=%s-%s-%s&officer=%s&proverka=1", Set.nickname, date3, date2, date1, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(addcont, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addcont [ник/ид] [дата (Пример: 27.01.2018)]");
	}
}

void CALLBACK addvig(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addvig [ник/ид] [причина]");
	}
	strcpy_s(Set.reason, (szParams += strlen(Set.nickname) + 1));

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3 && strlen(Set.reason) > 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=check", Set.nickname);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(CheckersVig, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addvig [ник/ид] [причина]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(Set.reason) > 3)
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=check", Set.nickname);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(CheckersVig, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/addvig [ник/ид] [причина]");
	}
}

void CALLBACK delvig(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delvig [ник/ид]");
	}

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=check", Set.nickname);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(DelVigCheck, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delvig [ник/ид]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		sprintf_s(Set.send, "SEND=выговоры&name=%s&sub_send=check", Set.nickname);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(DelVigCheck, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/delvig [ник/ид]");
	}
}

void CALLBACK tr(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/tr [ник/ид]");
	}

	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		char str[48];
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(str, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			Say("/offmfilter name %s", str);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/tr [ник/ид]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		Say("/offmfilter name %s", Set.nickname);
	}
}

void CALLBACK check(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/check [ник/ид]");
	}
	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		Set.usPlayerId = std::stoi(Set.nickname);
		if (Set.usPlayerId <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(Set.usPlayerId));
			showDialog(1, 7000, 2, (char *)std::string("Действия над: {01b729}" + std::string(Set.nickname)).c_str(), "[1] Проверить на ЧС\n[2] Повышения/Понижения\n[3] Удалить из ЧС\n[4] Проверить выговоры", "Выбрать", "Закрыть");
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/check [ник/ид]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		showDialog(1, 7000, 2, (char *)std::string("Действия над: {01b729}" + std::string(Set.nickname)).c_str(), "[1] Проверить на ЧС\n[2] Повышения/Понижения\n[3] Удалить из ЧС\n[4] Проверить выговоры", "Выбрать", "Закрыть");
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] Используй: {01b729}/check [ник/ид]");
	}
}

void CALLBACK help(std::string param)
{
	SF->getSAMP()->getDialog()->ShowDialog(4, 0, "{095200}LVa Homecoming by M.Friedmann v0.6", "{095200}/blacklist [ник/ид] [причина]{ffffff} - добавить в чёрный список Армии;\n{095200}/check [ник/ид]{ffffff} - проверка логов на игрока;\n{095200}/addvig [ник/ид] [причина]{ffffff} - выдать выговор\n{095200}/delvig [ник/ид]{ffffff} - снять выговор;\n{095200}/addcont [ник/ид] [дата]{ffffff} - добавить в список контрактников;\n{095200}/delcont [ник/ид]{ffffff} - удалить из списка контрактников;\n{095200}/tr [ник/ид]{ffffff} - фильтр offmembers'a;", "Выбрать", "Закрыть");
}

#pragma endregion

#pragma region RPC
bool CALLBACK incomingRPC(stRakNetHookParams *params)
{
	if (params->packetId == ScriptRPCEnumeration::RPC_ScrApplyAnimation)
	{
		if (GetTickCount() < Set.dwLastPauseTick + 2000)
		{
			return false;
		}
	}
	if (params->packetId == ScriptRPCEnumeration::RPC_ScrClientMessage)
	{
		uint32_t        dwStrLen, dwColor;
		char            szMsg[256];

		params->bitStream->ResetReadPointer();
		params->bitStream->Read(dwColor);
		params->bitStream->Read(dwStrLen);
		if (dwStrLen >= sizeof(szMsg))
			dwStrLen = sizeof(szMsg) - 1;
		params->bitStream->Read(szMsg, dwStrLen);
		szMsg[dwStrLen] = '\0';
		params->bitStream->ResetReadPointer();
		if (strstr(szMsg, "Вы повысили/понизили") && dwColor == 1790050218)
		{
			char rang[10];
			StringFind(szMsg, "Вы повысили/понизили ", " до", Set.nickname);
			StringFind(szMsg, " до ", " ранга", rang);
			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
			sprintf_s(Set.send, "SEND=ранг&name=%s&rank=%s&officer=%s&proverka=1", Set.nickname, rang, Set.officer);
			RequestData *request_data = new RequestData(Set.send);

			_beginthread(Ranks, NULL, (void *)request_data);
		}
		if (strstr(szMsg, "Вы пригласили") && strstr(szMsg, "присоединиться к Army LV") && dwColor == 1790050218)
		{
			StringFind(szMsg, "Вы пригласили ", " присоединиться к Army LV", Set.nickname);
			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->GetPlayerName(SF->getSAMP()->getPlayers()->sLocalPlayerID));
			sprintf_s(Set.send, "SEND=инвайт&name=%s&officer=%s&proverka=1", Set.nickname, Set.officer);
			RequestData *request_data = new RequestData(Set.send);

			_beginthread(Invite, NULL, (void *)request_data);
		}
		if (strstr(szMsg, "Вы выгнали") && strstr(szMsg, "из организации") && strstr(szMsg, "Причина") && dwColor == 1790050218)
		{
			StringFind(szMsg, "Вы выгнали ", " из организации", Set.nickname);
			StringFindEnd(szMsg, "Причина: ", '\0', Set.reason);

			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->GetPlayerName(SF->getSAMP()->getPlayers()->sLocalPlayerID));
			sprintf_s(Set.send, "SEND=увал&name=%s&officer=%s&reason=%s&proverka=1", Set.nickname, Set.officer, ANSI_to_UTF8(Set.reason));
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(Uninvite, NULL, (void *)request_data);

		}
		if (strstr(szMsg, "Warning(opcode 0x812)"))
		{
			Set.dwLastPauseTick = GetTickCount();
			return true;
		}
	}

	return true;
}

bool CALLBACK OutcomingRPCHandler(stRakNetHookParams *params)
{
	if (params->packetId == RPCEnumeration::RPC_DialogResponse)
	{

		WORD wDialogID;
		BYTE bButtonID;
		WORD wListBoxItem;
		char szInputResp[128 + 1];
		unsigned char iInputRespLen;

		params->bitStream->ResetReadPointer();
		params->bitStream->Read(wDialogID);
		params->bitStream->Read(bButtonID);
		params->bitStream->Read(wListBoxItem);
		params->bitStream->Read(iInputRespLen);
		params->bitStream->Read(szInputResp, iInputRespLen);
		szInputResp[iInputRespLen] = 0;
		params->bitStream->ResetReadPointer();

		switch (wDialogID)
		{
			case 7000:
			{
				if (wListBoxItem == 0 && bButtonID == 1)
				{
					char send[255];
					sprintf_s(send, "SEND=чс&name=%s&sub_send=check", Set.nickname);
					RequestData *request_data = new RequestData(send);
					_beginthread(Check, NULL, (void *)request_data);
				}
				else if (wListBoxItem == 1 && bButtonID == 1)
				{
					char send[255];
					sprintf_s(send, "SEND=logs_rank&name=%s", Set.nickname);
					RequestData *request_data = new RequestData(send);
					_beginthread(CheckRang, NULL, (void *)request_data);
				}
				else if (wListBoxItem == 2 && bButtonID == 1)
				{
					char send[255];
					sprintf_s(send, "SEND=чс&name=%s&officer=%s&sub_send=delete&proverka=1", Set.nickname, Set.officer);
					RequestData *request_data = new RequestData(send);
					_beginthread(DellBlackList, NULL, (void *)request_data);
				}
				else if (wListBoxItem == 3 && bButtonID == 1)
				{
					char send[255];
					sprintf_s(send, "SEND=выговоры&name=%s&sub_send=checker", Set.nickname);
					RequestData *request_data = new RequestData(send);
					_beginthread(CheckVig, NULL, (void *)request_data);
				}
				return false;
			}
		}
	}
	return true;
}
#pragma endregion

void CMD()
{
	static bool init = false;
	if (!init)
	{
		SF->getSAMP()->registerChatCommand("check", check);
		SF->getSAMP()->registerChatCommand("tr", tr);
		SF->getSAMP()->registerChatCommand("blacklist", blacklist);
		SF->getSAMP()->registerChatCommand("help", help);
		SF->getSAMP()->registerChatCommand("addvig", addvig);
		SF->getSAMP()->registerChatCommand("delvig", delvig);
		SF->getSAMP()->registerChatCommand("delcont", delvig);
		SF->getSAMP()->registerChatCommand("addcont", addcont);

		SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_INCOMING_RPC, incomingRPC);
		SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_OUTCOMING_RPC, OutcomingRPCHandler);

		init = true;
	}
}