#ifndef __FUNC_H_
#define __FUNC_H_

#pragma once

extern AString& replace_all(AString& str, const AString& old_value, const AString& new_value);

extern AString& replace_all_distinct(AString& str, const AString& old_value, const AString& new_value);

extern AString UTF8ToGBK(const AString& strUTF8);

extern AString GBKToUTF8(const AString& strGBK);

//Éú³ÉUUID
extern AString CreateUUID();

extern AString GetMAC();

extern AStringVector GetBSSIDs();

#endif