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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] {01b729}%s {FFFFFF}�������� � �� � ��������: {01b729}%s", Set.nickname, Set.reason);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] {01b729}%s {FFFFFF}��� ��������� � ��", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}���������/���������{FFFFFF}] {01b729}%s {FFFFFF}�������� � ��� ��������� / ���������", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}����{FFFFFF}] {01b729}%s {FFFFFF}�������� � ��� �����", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] {01b729}%s {FFFFFF}�������� � ������ �������������", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] {01b729}%s {FFFFFF}��� ��������� � ������ �������������", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] {01b729}%s {FFFFFF}������� ������ �� ������ �������������.", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] {01b729}%s {FFFFFF}�� ��������� � ������ �������������.", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}����������{FFFFFF}] {01b729}%s {FFFFFF}�������� � ��� ����������", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}׸���� ������{FFFFFF}] {01b729}%s{FFFFFF} �� ������� � ��", Set.nickname);
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

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] �����:{01b729} %s {ffffff} || ���:{01b729} %s {ffffff}", Set.nickname, nick);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] �������: {01b729}%s", reason);
	}
}

void DellBlackList(void *data)
{
	RequestData *request_data = static_cast<RequestData*>(data);

	char result[2048];

	SendRequest(request_data, result);

	delete request_data;

	StringFind(request_data->message, "SEND=��&name=", "&officer", Set.nickname);

	if (!strlen(result))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] {01b729}%s {FFFFFF}������� ������ �� ������� ������!", Set.nickname);
	}
	else if (strstr(result, "code: 2"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��{FFFFFF}] {01b729}%s {FFFFFF}�� ��������� � ������ ������!", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else
	{
		showDialog(0, 5, 4, "{095200}LVa Homecoming by M.Friedmann v0.6", result, "�������", "�������");
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 0"))
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] � {01b729}%s {FFFFFF}��� ���������!", Set.nickname);
	}
	else
	{
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}��������{FFFFFF}] %s", result);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(send, "SEND=��������&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(send, "SEND=��������&name=%s&sub_send=logs&reason=�������� � 1 �� 0 ���������&officer=%s&proverka=1&kolvo= ", Set.nickname, Set.officer);
		StringFind(send, "&reason=", "&officer=", Set.send);
		sprintf_s(send, "SEND=��������&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo= ", Set.nickname, ANSI_to_UTF8(Set.send), Set.officer);
		RequestData *request_data = new RequestData(send);
		_beginthread(Vig, NULL, (void *)request_data);
	}
	else if (strstr(result, "code: 2"))
	{
		int res = Set.Code + 1;
		sprintf_s(send, "�������� � %d �� %d ���������", res, Set.Code);
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=logs&reason=%s&officer=%s&proverka=1&kolvo= ", Set.nickname, ANSI_to_UTF8(send), Set.officer);

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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 0"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ������� �����.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] �������: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}1{ffffff} �� {01b729}3");
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=2", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ������� �����.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] �������: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}2{ffffff} �� {01b729}3");
	}
	else if (strstr(result, "code: 2"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1&kolvo=3", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigAdd, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ������� �����.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] �������: {01b729}%s.", Set.reason);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}3{ffffff} �� {01b729}3");
	}
	else if (strstr(result, "code: 3"))
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] � {01b729}%s {FFFFFF}3 �� 3 ���������", Set.nickname);
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
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] �� ������� ����������� � ������");
	}
	else if (strstr(result, "code: 0"))
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] �  {01b729}%s {ffffff} ��������� ���!", Set.nickname);
	}
	else if (strstr(result, "code: 1"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=del&kolvo= &officer=%s&proverka=1", Set.nickname, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ���������� ��������� ���������.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}0{ffffff} �� {01b729}3");
	}
	else if (strstr(result, "code: 2"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=del&kolvo=1&officer=%s&proverka=1", Set.nickname, Set.officer);
		Set.Code = 1;
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ���������� ��������� ���������.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}1{ffffff} �� {01b729}3");
	}
	else if (strstr(result, "code: 3"))
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=del&kolvo=2&officer=%s&proverka=1", Set.nickname, Set.officer);
		Set.Code = 2;
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(VigDel, NULL, (void *)request_data);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {9607c1}�������{FFFFFF}] {01b729}%s {FFFFFF} - ���������� ��������� ���������.", Set.nickname);
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "{FFFFFF} [{01b729}LVa{FFFFFF} | {9607c1}�������{FFFFFF}] ���������� ��������� : {01b729}2{ffffff} �� {01b729}3");
	}
}
#pragma endregion

#pragma region CMD
void CALLBACK blacklist(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/blacklist [���/��] [�������]");
	}

	strcpy_s(Set.reason, (szParams += strlen(Set.nickname) + 1));

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3 && strlen(Set.reason) > 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=��&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(Blacklist, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/blacklist [���/��] [�������]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(Set.reason) > 3)
	{
		sprintf_s(Set.send, "SEND=��&name=%s&sub_send=add&reason=%s&officer=%s&proverka=1", Set.nickname, ANSI_to_UTF8(Set.reason), Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(Blacklist, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/blacklist [���/��] [�������]");
	}
}

void CALLBACK delcont(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delcont [���/��]");
	}

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=delete&officer=%s&proverka=1", Set.nickname, Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(delcont, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delcont [���/��]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=delete&officer=%s&proverka=1", Set.nickname, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(delcont, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delcont [���/��]");
	}
}

void CALLBACK addcont(std::string param)
{
	char date[40];
	char date1[10], date2[10], date3[15];
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s %s", &Set.nickname, &date) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addcont [���/��] [���� (������: 27.01.2018)]");
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
			sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=add&data=%s-%s-%s&officer=%s&proverka=1", Set.nickname, date3, date2, date1, Set.officer);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(addcont, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addcont [���/��] [���� (������: 27.01.2018)]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(date) > 3)
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=add&data=%s-%s-%s&officer=%s&proverka=1", Set.nickname, date3, date2, date1, Set.officer);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(addcont, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addcont [���/��] [���� (������: 27.01.2018)]");
	}
}

void CALLBACK addvig(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addvig [���/��] [�������]");
	}
	strcpy_s(Set.reason, (szParams += strlen(Set.nickname) + 1));

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3 && strlen(Set.reason) > 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=check", Set.nickname);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(CheckersVig, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addvig [���/��] [�������]");
		}
	}
	else if (strlen(Set.nickname) > 3 && strlen(Set.reason) > 3)
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=check", Set.nickname);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(CheckersVig, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/addvig [���/��] [�������]");
	}
}

void CALLBACK delvig(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delvig [���/��]");
	}

	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		int i = std::stoi(Set.nickname);
		if (i <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(i));
			sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=check", Set.nickname);
			RequestData *request_data = new RequestData(Set.send);
			_beginthread(DelVigCheck, NULL, (void *)request_data);
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delvig [���/��]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		sprintf_s(Set.send, "SEND=��������&name=%s&sub_send=check", Set.nickname);
		RequestData *request_data = new RequestData(Set.send);
		_beginthread(DelVigCheck, NULL, (void *)request_data);
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/delvig [���/��]");
	}
}

void CALLBACK tr(std::string param)
{
	const char *szParams = param.c_str();
	if (sscanf(szParams, "%s", &Set.nickname) < 1)
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/tr [���/��]");
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
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/tr [���/��]");
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
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/check [���/��]");
	}
	strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
	if (strlen(Set.nickname) >= 1 && strlen(Set.nickname) <= 3)
	{
		Set.usPlayerId = std::stoi(Set.nickname);
		if (Set.usPlayerId <= SF->getSAMP()->getPlayers()->ulMaxPlayerID)
		{
			strcpy_s(Set.nickname, SF->getSAMP()->getPlayers()->GetPlayerName(Set.usPlayerId));
			showDialog(1, 7000, 2, (char *)std::string("�������� ���: {01b729}" + std::string(Set.nickname)).c_str(), "[1] ��������� �� ��\n[2] ���������/���������\n[3] ������� �� ��\n[4] ��������� ��������", "�������", "�������");
		}
		else
		{
			return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/check [���/��]");
		}
	}
	else if (strlen(Set.nickname) > 3)
	{
		showDialog(1, 7000, 2, (char *)std::string("�������� ���: {01b729}" + std::string(Set.nickname)).c_str(), "[1] ��������� �� ��\n[2] ���������/���������\n[3] ������� �� ��\n[4] ��������� ��������", "�������", "�������");
	}
	else
	{
		return SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xFF, 0), "{FFFFFF} [{01b729}LVa {FFFFFF}| {e0021c}Error{FFFFFF}] ���������: {01b729}/check [���/��]");
	}
}

void CALLBACK help(std::string param)
{
	SF->getSAMP()->getDialog()->ShowDialog(4, 0, "{095200}LVa Homecoming by M.Friedmann v0.6", "{095200}/blacklist [���/��] [�������]{ffffff} - �������� � ������ ������ �����;\n{095200}/check [���/��]{ffffff} - �������� ����� �� ������;\n{095200}/addvig [���/��] [�������]{ffffff} - ������ �������\n{095200}/delvig [���/��]{ffffff} - ����� �������;\n{095200}/addcont [���/��] [����]{ffffff} - �������� � ������ �������������;\n{095200}/delcont [���/��]{ffffff} - ������� �� ������ �������������;\n{095200}/tr [���/��]{ffffff} - ������ offmembers'a;", "�������", "�������");
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
		if (strstr(szMsg, "�� ��������/��������") && dwColor == 1790050218)
		{
			char rang[10];
			StringFind(szMsg, "�� ��������/�������� ", " ��", Set.nickname);
			StringFind(szMsg, " �� ", " �����", rang);
			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->szLocalPlayerName);
			sprintf_s(Set.send, "SEND=����&name=%s&rank=%s&officer=%s&proverka=1", Set.nickname, rang, Set.officer);
			RequestData *request_data = new RequestData(Set.send);

			_beginthread(Ranks, NULL, (void *)request_data);
		}
		if (strstr(szMsg, "�� ����������") && strstr(szMsg, "�������������� � Army LV") && dwColor == 1790050218)
		{
			StringFind(szMsg, "�� ���������� ", " �������������� � Army LV", Set.nickname);
			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->GetPlayerName(SF->getSAMP()->getPlayers()->sLocalPlayerID));
			sprintf_s(Set.send, "SEND=������&name=%s&officer=%s&proverka=1", Set.nickname, Set.officer);
			RequestData *request_data = new RequestData(Set.send);

			_beginthread(Invite, NULL, (void *)request_data);
		}
		if (strstr(szMsg, "�� �������") && strstr(szMsg, "�� �����������") && strstr(szMsg, "�������") && dwColor == 1790050218)
		{
			StringFind(szMsg, "�� ������� ", " �� �����������", Set.nickname);
			StringFindEnd(szMsg, "�������: ", '\0', Set.reason);

			strcpy_s(Set.officer, SF->getSAMP()->getPlayers()->GetPlayerName(SF->getSAMP()->getPlayers()->sLocalPlayerID));
			sprintf_s(Set.send, "SEND=����&name=%s&officer=%s&reason=%s&proverka=1", Set.nickname, Set.officer, ANSI_to_UTF8(Set.reason));
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
					sprintf_s(send, "SEND=��&name=%s&sub_send=check", Set.nickname);
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
					sprintf_s(send, "SEND=��&name=%s&officer=%s&sub_send=delete&proverka=1", Set.nickname, Set.officer);
					RequestData *request_data = new RequestData(send);
					_beginthread(DellBlackList, NULL, (void *)request_data);
				}
				else if (wListBoxItem == 3 && bButtonID == 1)
				{
					char send[255];
					sprintf_s(send, "SEND=��������&name=%s&sub_send=checker", Set.nickname);
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