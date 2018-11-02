#pragma once

struct Setting
{
	/*Dword*/
	DWORD	dwLastPauseTick;

	/*int*/
	int Code;

	/*char*/
	char nickname[26];
	char officer[26];
	char reason[510];
	char send[1024];
	char result[2048];

	/*2 bytes*/
	unsigned short usPlayerId;

	/*1 bytes*/
	unsigned char byteCheckID;
};


void initSetting();
extern struct Setting Set;