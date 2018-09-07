#include "stdafx.h"
#include "func.h"
#include <windef.h>
#include <nb30.h>
#include <wlanapi.h>

#include "setdebugnew.h"

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib,"netapi32")



AString UTF8ToGBK(const AString& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	WCHAR* wszGBK = new WCHAR[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	AString strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	return strTemp;
}


AString GBKToUTF8(const AString& strGBK)
{
	string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

AString& replace_all(AString& str, const AString& old_value, const AString& new_value)
{
	while (true)   {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}
AString& replace_all_distinct(AString& str, const AString& old_value, const AString& new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length())   {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}


AString CreateUUID()
{
	GUID guid;
	if (CoCreateGuid(&guid))
	{
		LOGERROR("create guid error\n");
		return "";
	}
	return Printf("%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
}


AString GetMAC()
{
	NCB ncb;
	typedef struct _ASTAT_
	{
		ADAPTER_STATUS   adapt;
		NAME_BUFFER   NameBuff[30];
	}ASTAT, *PASTAT;

	ASTAT Adapter;

	typedef struct _LANA_ENUM
	{
		UCHAR   length;
		UCHAR   lana[MAX_LANA];
	}LANA_ENUM;

	LANA_ENUM lana_enum;
	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char *)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);

	if (uRetCode != NRC_GOODRET)
		return "";

	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			break;
	}

	if (uRetCode != NRC_GOODRET)
		return "";

	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy_s((char*)ncb.ncb_callname, sizeof("*"), "*");
	ncb.ncb_buffer = (unsigned char *)&Adapter;
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);

	if (uRetCode != NRC_GOODRET)
		return "";

	return Printf("%02X:%02X:%02X:%02X:%02X:%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]);
}

AStringVector GetBSSIDs()
{
	AStringVector vecBSSIDs;

	// Declare and initialize variables.

	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;      //    
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;
	int iRet = 0;

	WCHAR GuidString[39] = { 0 };

	unsigned int i, j, k;

	/* variables used for WlanEnumInterfaces  */

	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
	PWLAN_INTERFACE_INFO pIfInfo = NULL;

	PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
	PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;

	int iRSSI = 0;

	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult == ERROR_SUCCESS)
	{
		dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
		if (dwResult != ERROR_SUCCESS) 
		{
			LOG("WlanEnumInterfaces failed with error: %u\n", dwResult);
			// You can use FormatMessage here to find out why the function failed
		}
		else 
		{
			for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
				pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

				dwResult = WlanGetAvailableNetworkList(hClient,
					&pIfInfo->InterfaceGuid, 0, NULL, &pBssList);

				if (dwResult != ERROR_SUCCESS) {
					LOG("WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
					dwRetVal = 1;
					// You can use FormatMessage to find out why the function failed
				}
				else 
				{
					for (j = 0; j < pBssList->dwNumberOfItems; j++)
					{
						pBssEntry = (WLAN_AVAILABLE_NETWORK *)& pBssList->Network[j];

						PWLAN_BSS_LIST ppWlanBssList;

						DWORD dwResult2 = WlanGetNetworkBssList(hClient, &pIfInfo->InterfaceGuid,
							&pBssEntry->dot11Ssid,
							pBssEntry->dot11BssType,
							pBssEntry->bSecurityEnabled,
							NULL,
							&ppWlanBssList);

						if (dwResult2 == ERROR_SUCCESS)
						{
							for (int z = 0; z < ppWlanBssList->dwNumberOfItems; z++)
							{
								WLAN_BSS_ENTRY bssEntry = ppWlanBssList->wlanBssEntries[z];

								AString bssid = Printf("%02X:%02X:%02X:%02X:%02X:%02X",
									bssEntry.dot11Bssid[0],
									bssEntry.dot11Bssid[1],
									bssEntry.dot11Bssid[2],
									bssEntry.dot11Bssid[3],
									bssEntry.dot11Bssid[4],
									bssEntry.dot11Bssid[5]);

								vecBSSIDs.push_back(bssid);
							}
						}
					}
				}
			}

		}
	}
	else
	{
		LOG("WlanOpenHandle failed with error: %u\n", dwResult);
		// You can use FormatMessage here to find out why the function failed
	}

	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}

	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}

	return vecBSSIDs;
}