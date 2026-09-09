#ifndef eSyncMultyPmb_h__
#define eSyncMultyPmb_h__

#pragma region Doxygen
// Doxygen code user manual //
/*!
\page eSyncMultyPmb eSyncMultyPmb 
\brief <STRONG> Модуль синхронизации потока по преамбуле </STRONG>

<BLOCKQUOTE>
\brief \b Функционал \b модуля. 
Модуль реализует детектирование преамбулы. 
</BLOCKQUOTE>

<BLOCKQUOTE>
\par Параметры
- \b Preambl- Преамбулы HEX(преамбулы перечисляются через запятую(пример 11111,aaaaaa,1234576af)), байт ** - любой байт.
- \b Error - Количество допустимых битовых ошибок в преамбуле.
- \b PrevBits - Количество бит которые необходимо подать на выход до преамбулы (целое число байт).
- \b ByteMiss - Количество байт, которое необходимо пропустить после нахождения преамбулы.
- \b FixSkkInv - Количество бит на символ модуляции - для устранения фазовой неоднозначности демодуляции после нахождения преамбулы, 0 - преобразование не применяется.
- \b FlagSyncOn - Флаг запуска поиска синхрокомбинации.
- \b FlagTimeCnt - Флаг отсчета времени перезапуска, для включения режима вычисления позиции преамбулы.
- \b FlagSetDataToOut - Флаг отправки данных на выход до нахождения первой преамбулы.
</BLOCKQUOTE>

<BLOCKQUOTE>
\par Сообщения
\arg Msg_PreamblFind - Сообщение о том, что преамбула найдена (данные сообщения - позиция текущей преамбулы в битах от начала файла).
\arg Msg_PreamblFindNum - Сообщение о номере преамбулы из списка(данные сообщения - номер найденной преамбулы).

</BLOCKQUOTE>

<BLOCKQUOTE>
\par Входные интерфейсы
- Вход 0 - Входная реализация сигнала. Минимальный размер буфера PreamblLen байт.
</BLOCKQUOTE>

<BLOCKQUOTE>
\par Выходные интерфейсы
- Выход 0 - Выходная реализация сигнала.
</BLOCKQUOTE>
<BLOCKQUOTE>
\par Соотношение "Выход/Вход"
- Out0/In0 = 1/1 = 1.0
</BLOCKQUOTE>
\n
*/
#pragma endregion Doxygen

#include "../../include/e_base_defs.h"
#include "../../include/boost/boost_base_defs.h" 
#include "SyncMultyPmb.h"
#include <algorithm>

// Минимальный размер буфера входного интерфейса
#define IN_BUF_0_MIN_LEN 1
// Минимальный размер буфера выходного интерфейса
#define OUT_BUF_0_MIN_LEN 1
// Коэффициент отношения размеров входа и выхода
#define IN_OUT_0_K 2.0

static TModuleDescr g_description(
	STR("eSyncMultyPmb"), 
	STR("Модуль синхронизации по преамбуле.\n Минимальный размер входного буфера PreamblLen + PrevBits/8 байт"), 
	STR("ES Group"), 
	STR("1.3.0.0")
	// Версия модуля X.Y.Z.W (для файла конфигурации тракта значимыми являются только первые две цифры - X.Y)
	// X - концептуальные изменения, изменение интерфейсов модуля 
	// Y - функциональные изменения, изменение параметров
	// Z - исправление ошибок, доработка алгоритмов работы
	// W - номер сборки
);

const unsigned char ReverseByte[256] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8, 0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
	0xdf, 0xde, 0xdd, 0xdc, 0xdb, 0xda, 0xd9, 0xd8, 0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0,
	0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8, 0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0,
	0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8, 0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0,
	0xaf, 0xae, 0xad, 0xac, 0xab, 0xaa, 0xa9, 0xa8, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0,
	0x9f, 0x9e, 0x9d, 0x9c, 0x9b, 0x9a, 0x99, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91, 0x90,
	0x8f, 0x8e, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
	0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
	0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60,
	0x5f, 0x5e, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50,
	0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
	0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
	0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
	0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

class CeSyncMultyPmb
{
private:
	// Контекст модуля
	TModuleContext m_info;
	// Список индексов параметров-сообщений
	struct TParam
	{
		enum Param
		{
			Preambl,
			Error,
			PrevBits,
			ByteMiss,
			FixSkkInv,
			FlagSyncOn,
			FlagTimeCnt,
			FlagSetDataToOut,
			Msg_PreamblFind,
			Msg_BER,
			Msg_PreamblFindNum
		};
	};

	// Внутренние переменные
	int m_iPrPos;
	int m_iCurPmb;
	int	m_iPreamblLen;		
	std::vector<std::vector<uchar>> m_vucPreambul;
	std::vector<std::vector<uchar>> m_vucPmbMask;
	
	unsigned char* m_pucTmpBuf;
	int m_iTmpBufLen;
	CSyncByPreambul m_cFindPmb;
	bool m_bFlagPmbFind;
	bool m_bFlagWorkRetry;
	int m_iNumEndBits;
	unsigned char m_ucEndByte;
	int m_iIndex;
	int m_iAlradyMiss;
	int m_iPrevWritePos;
	int m_iBitWrite;//столько бит уже подано на выход
	int m_iErrors;
	// Переменные из файла конфигурации
	int m_iFlagSyncOn;
	int m_iFlagSetDataToOut;
	int m_iFlagTimeCnt;
	string_type m_sPreambl;
	int m_iError;
	int m_iPrevBits;
	int m_iBitMiss;
	int m_iByteMiss;
	unsigned int m_uiFixSkkInv;
	bool m_flagInvNow;
	uchar m_ucInvMask;
	int m_ibufsize;
	 
public:
	// Конструктор
	CeSyncMultyPmb() 
	{
		m_sPreambl = string_type(STR("00"));
		m_iPreamblLen = 0;
		m_iError = 0;
		m_pucTmpBuf = nullptr;
		m_iTmpBufLen = 0;
		m_bFlagPmbFind = false;
		m_bFlagWorkRetry = false;
		m_iBitWrite = 0;
		m_iNumEndBits = 0;
		m_ucEndByte = 0;
		m_iIndex = 0;
		m_iPrevBits = 0; 
		m_iBitMiss = 0;
		m_iAlradyMiss = 0;
		m_iByteMiss = 0;
		m_iPrevWritePos = 0;
		m_iFlagSyncOn = 1;
		m_iFlagTimeCnt = 0;
		m_iCurPmb = 0;
		m_iFlagSetDataToOut = 0;
		m_iErrors= 0;
		m_uiFixSkkInv = 0;
		m_flagInvNow = false;
		m_ucInvMask = 0xff;
		m_ibufsize = 0;

		// Добавляем параметры модуля
		//добавление стандартного параметра
		m_info.AddParam( TParam::Preambl, STR("Preambl"), STR("Преамбулы HEX(преамбулы перечисляются через запятую(пример 11111,aaaaaa,1234576af)), байт ** - любой байт."), m_sPreambl);
		m_info.AddParam(TParam::Error, STR("Error"), STR("Допустимое количество битовых ошибок"), cast_str(m_iError));
		m_info.AddParam(TParam::PrevBits, STR("PrevBits"), STR("Количество бит, которое надо дать на выход перед преамбулой"), cast_str(m_iPrevBits));
		m_info.AddParam(TParam::ByteMiss, STR("ByteMiss"), STR("Количество байт, которое необходимо пропустить после нахождения преамбулы"), cast_str(m_iBitMiss));
		m_info.AddParam(TParam::FixSkkInv, STR("FixSkkInv"), STR("Количество бит на символ модуляции - для устранения фазовой неоднозначности демодуляции после нахождения преамбулы, 0 - преобразование не применяется"), cast_str(m_uiFixSkkInv));
		m_info.AddParam(TParam::FlagSyncOn, STR("FlagSyncOn"), STR("Флаг запуска поиска синхрокомбинации [1, 0]"), cast_str(m_iFlagSyncOn));
		m_info.AddParam(TParam::FlagTimeCnt, STR("FlagTimeCnt"), STR("Флаг отсчета времени перезапуска, для включения режима вычисления позиции преамбулы[1, 0]"), cast_str(m_iFlagTimeCnt));
		m_info.AddParam(TParam::FlagSetDataToOut, STR("FlagSetDataToOut"), STR("Флаг отправки данных на выход до нахождения первой преамбулы[1, 0]"), cast_str(m_iFlagSetDataToOut));

		// Добавляем сообщения модуля
		m_info.AddParamMsg(TParam::Msg_PreamblFind, STR("PreamblFind"), STR("Сообщение о нахождении преамбулы (данные сообщения - позиция текущей преамбулы в битах от начала файла)"));
		m_info.AddParamMsg(TParam::Msg_PreamblFindNum, STR("PreamblFindNum"), STR("Сообщение о номере преамбулы из списка(данные сообщения - номер найденной преамбулы)"));
		m_info.AddParamMsg(TParam::Msg_BER, STR("BER"), STR("Сообщение сообщение с вероятностью битовой ошибки (данные сообщения - вероятность битовой ошибки)"));
		
		// Копируем описание модуля
		m_info.descr = g_description;

		// Инициализируем матрицу коммутации - (вход)x(выход)
		m_info.matrix.SetInOutCount(1, 1);
		m_info.matrix.SetCell(0, 0, IN_OUT_0_K);

		// Инициализируем минимальный размер входных-выходных буферов
		m_info.matrix.in_buffers.at(0) = IN_BUF_0_MIN_LEN;
		m_info.matrix.out_buffers.at(0) = OUT_BUF_0_MIN_LEN;
	}

	// Деструктор
	~CeSyncMultyPmb() {}
	
	// Функция первоначальной установки параметров модуля
	//  pContext - контекст модуля со списоком параметров
	int Init(const TModuleContext* pContext) 
	{
		std::locale::global(std::locale(""));
		if (!pContext->parameters.empty())
		{
			// Установка входных параметров в контекст модуля
			m_info.SetModuleContext(pContext);

			if (m_info.CheckParam(TParam::Preambl))
				m_sPreambl = m_info.GetParamData(TParam::Preambl);
			if (m_info.CheckParam(TParam::Error))
				m_iError = cast_int(m_info.GetParamData(TParam::Error));
			if (m_info.CheckParam(TParam::PrevBits))
				m_iPrevBits = cast_int(m_info.GetParamData(TParam::PrevBits));
			if (m_info.CheckParam(TParam::FixSkkInv))
				m_uiFixSkkInv = cast_uint(m_info.GetParamData(TParam::FixSkkInv));
			if (m_info.CheckParam(TParam::FlagSyncOn))
				m_iFlagSyncOn = cast_int(m_info.GetParamData(TParam::FlagSyncOn));
			if (m_info.CheckParam(TParam::FlagTimeCnt))
				m_iFlagTimeCnt = cast_int(m_info.GetParamData(TParam::FlagTimeCnt));
			if (m_info.CheckParam(TParam::ByteMiss))
				m_iByteMiss = cast_int(m_info.GetParamData(TParam::ByteMiss));
			if (m_info.CheckParam(TParam::FlagSetDataToOut))
				m_iFlagSetDataToOut = cast_int(m_info.GetParamData(TParam::FlagSetDataToOut));
			m_iBitMiss = m_iByteMiss*8 + m_iPrevBits;

			boost::algorithm::replace_all(m_sPreambl, STR(" "), "");
			std::vector<std::string> vsPmb;
			boost::algorithm::split(vsPmb, m_sPreambl, boost::is_any_of(","), boost::token_compress_on);
			m_vucPreambul.resize(vsPmb.size());
			m_vucPmbMask.resize(vsPmb.size());
			for (int i = 0; i < (int)vsPmb.size(); i++)
			{
				string_type strPmb = str_utils::aToLocal(vsPmb[i].data());
				HexStringToBytes(strPmb, m_vucPreambul[i], m_vucPmbMask[i]);
				if ((int)m_vucPreambul[i].size() >= m_iPreamblLen)
				{
					m_iPreamblLen = (int)m_vucPreambul[i].size();
				}
			}
			if (m_uiFixSkkInv > 0)
			{	// добавление инверсных преамбул
				m_vucPreambul.resize(vsPmb.size() * 2);
				m_vucPmbMask.resize(vsPmb.size() * 2);
				for (size_t i = vsPmb.size(); i < vsPmb.size() * 2; ++i) 
				{
					size_t srcInd = i - vsPmb.size();
					std::vector<uchar> invPmb = m_vucPreambul[srcInd];
					for (int j = 0; j < m_vucPreambul[srcInd].size(); j++)
						invPmb[j] = m_ucInvMask ^ invPmb[j];
					m_vucPreambul[i] = invPmb;
					m_vucPmbMask[i] = m_vucPmbMask[srcInd];
				}
			}
			//HexStringToBytes(m_sPreambl, m_pucPreambl, m_iPreamblLen);
			if(m_pucTmpBuf) delete [] m_pucTmpBuf;
			//выделяем память для темпового буффера для хранения хвоста
			m_pucTmpBuf = new unsigned char [m_iPreamblLen + m_iPrevBits/8];
			m_iTmpBufLen = 0;
			m_bFlagPmbFind = false;
			m_bFlagWorkRetry = false;
			m_iBitWrite = 0;
			m_iNumEndBits = 0;
			m_ucEndByte = 0;
			m_iIndex = 0;
			m_iPrevWritePos = 0;
			m_iAlradyMiss = 0;
			m_iPrPos = 0;
			m_iCurPmb = 0;
			m_iErrors = 0;
			m_flagInvNow = false;
		}
				
		
		return ModuleError::Ok;
	}

	// Функция обработки данных
	//	inDataList - список буферов входных интерфейсов
	//	outDataList	- список буферов выходных интерфейсов
	int Work(BDataList& inDataList, BDataList& outDataList) 
	{ 
		if (inDataList.size() == 0) 
			return ModuleError::InvalidParameters;
	 
		// Получаем реальную длину входного блока данных
		int iLen = inDataList.at(0)->RealLength();

		if (iLen > 0)
		{
			m_iPrPos += iLen;
			int iStartFindPos = 0;
			// Получаем указатели на входной и выходной блоки данных
			unsigned char *pInData = inDataList.at(0)->Data();
			unsigned char *pOutData = outDataList.at(0)->Data();
			if(m_iFlagSyncOn == 0)
			{
				if(m_iTmpBufLen != 0)
				{
					unsigned char* pucInBuf = new unsigned char[iLen + m_iTmpBufLen];
					memcpy(pucInBuf,m_pucTmpBuf,m_iTmpBufLen);
					memcpy(pucInBuf+m_iTmpBufLen, pInData, iLen);
					memset(pOutData,0,iLen*2);
					//выдаем недописанные биты на выход
					pOutData[0] = m_ucEndByte>>(8-m_iNumEndBits);
					m_iBitWrite = m_iNumEndBits; //позиция в битах, с которой нужно писать
					//даем целое количество байт на выход
					//подаем целое число байт на выход
					int iBitWrite = (iLen+m_iTmpBufLen)*8;//столько бит надо записать
					int iBitToOut = iBitWrite + m_iBitWrite; //столько бит пойдет на выход
					//вычисляем сколько бит можно дописать
					int iBitWriteToOut = iBitWrite - iBitToOut%8;
					int iNumByte = (iBitWriteToOut+m_iBitWrite)/8; //столько байт надо дать на выход
					m_ibufsize = iNumByte;
					//даем 
					m_cFindPmb.cutBits(pucInBuf,pOutData,0,m_iBitWrite,iBitWriteToOut);
					//сохраняем недописанный байт
					m_iNumEndBits = iBitToOut%8;
					if(m_iNumEndBits != 0 )
					{	//сохраняем эти биты
						m_ucEndByte = pucInBuf[iLen+m_iTmpBufLen - 1];
					}
					else
						m_ucEndByte = 0;
					m_iTmpBufLen = 0;
					if (pucInBuf)
						delete[] pucInBuf;
					if (outDataList.at(0)->SetRealLength(iNumByte)>0) 
						return ModuleError::BufLengsError; // Ошибка длины выходного буфера
				}
				else
				{
					memset(pOutData,0,iLen*2);
					//выдаем недописанные биты на выход
					pOutData[0] = m_ucEndByte>>(8-m_iNumEndBits);
					m_iBitWrite = m_iNumEndBits; //позиция в битах, с которой нужно писать
					//подаем целое число байт на выход
					int iBitWrite = iLen*8;//столько бит надо записать
					int iBitToOut = iBitWrite + m_iBitWrite; //столько бит пойдет на выход
					//вычисляем сколько бит можно дописать
					int iBitWriteToOut = iBitWrite - iBitToOut%8;
					int iNumByte = (iBitWriteToOut+m_iBitWrite)/8; //столько байт надо дать на выход
					m_ibufsize = iNumByte;
					//даем 
					m_cFindPmb.cutBits(pInData,pOutData,0,m_iBitWrite,iBitWriteToOut);

					//сохраняем недописанный байт
					m_iNumEndBits = iBitToOut%8;
					if(m_iNumEndBits != 0 )
					{	//сохраняем эти биты
						m_ucEndByte = pInData[iLen-1];
					}
					else
						m_ucEndByte = 0;
					if (outDataList.at(0)->SetRealLength(iNumByte)>0) 
						return ModuleError::BufLengsError; // Ошибка длины выходного буфера
					
				}
				if (m_flagInvNow) {
					InvBuf(pOutData, outDataList.at(0)->RealLength());
				}
				return ModuleError::Ok;
			}
			//формируем входной буффер с хвостом
			unsigned char* pucInBuf = new unsigned char[iLen + m_iTmpBufLen];
			memcpy(pucInBuf,m_pucTmpBuf,m_iTmpBufLen);
			memcpy(pucInBuf+m_iTmpBufLen, pInData, iLen);
			memset(pOutData,0,iLen*2);
			//выдаем недописанные биты на выход
			if (m_flagInvNow) {
				// Инвертируем недописанные биты с использованием матрицы RevarseByte
				uchar ucInv = ReverseByte[m_ucEndByte];
				pOutData[0] = ucInv >> (8 - m_iNumEndBits);
			}
			else
				pOutData[0] = m_ucEndByte >> (8 - m_iNumEndBits);
			m_iBitWrite = m_iNumEndBits; //позиция в битах, с которой нужно писать
			//ищем преамбулу
			if(!m_bFlagWorkRetry)
			{
				//вычисляем байт с которого надо начинать искать
				iStartFindPos = (m_iBitMiss - m_iAlradyMiss)/8;
				//если до этого еще не было преамбулы
				if(!m_bFlagPmbFind)
				{
					iStartFindPos = 0;
				}
				//проверяем хватило ли нам чтобы пропустить необходимое количество
				if(iStartFindPos >=iLen + m_iTmpBufLen - m_iPreamblLen)
				{
					//не хватило
					//говорим что в этом блоке преамбулы нет
					m_iIndex = -1;
				}
				else
				{
					//хватило
					for (int i = 0; i < (int)m_vucPreambul.size(); i++)
					{
						m_iIndex = m_cFindPmb.findePmbTableErrors(pucInBuf + iStartFindPos, (iLen + m_iTmpBufLen - iStartFindPos) * 8, (int)m_vucPreambul[i].size() * 8, m_vucPreambul[i].data(),m_vucPmbMask[i].data(), m_iError, m_iErrors);
						if (m_iIndex != -1)
						{
							m_iCurPmb = i;
							m_flagInvNow = false;
							if (m_uiFixSkkInv)
								if (m_iCurPmb >= m_vucPreambul.size() / 2)
									m_flagInvNow = true;
							break;
						}
					}
					if(m_iIndex != -1)
					{
						m_iPrPos = m_iPrPos - (iLen - m_iIndex);
						m_iIndex += iStartFindPos*8;
					}
				}
			}
			if(m_iIndex != -1)
			{
				//рассчитываем позицию с которой будем давать на выход
				m_iPrevWritePos = m_iIndex - m_iPrevBits;
				//проверяем, что столько есть
				if(m_iPrevWritePos < 0)
				{
					//столько нету, выдаем с самого начала
					m_iPrevWritePos = 0;
				}
				//преамбула найдена
				//проверяем была ли до этого преамбула
				if(m_bFlagPmbFind)
				{
					//преамбула до этого уже была
					//проверяем что мы попали сюда не из WorkRetry
					if(!m_bFlagWorkRetry)
					{
						//мы здесь не из WorkRetry
						//записываем, что недописанных бит у нас нет
						m_iNumEndBits = 0;
						m_ucEndByte = 0;
						//даем на выход все до новой преамбулы
						m_cFindPmb.cutBits(pucInBuf,pOutData,0,m_iBitWrite,m_iPrevWritePos);
						int iOutLen = 0;
						if((m_iPrevWritePos + m_iBitWrite)%8 == 0)
						{
							iOutLen = (m_iPrevWritePos + m_iBitWrite)/8;
						}
						else
						{
							iOutLen = (m_iPrevWritePos + m_iBitWrite)/8 +1;
						}
						if (outDataList.at(0)->SetRealLength(iOutLen)>0) 
							return ModuleError::BufLengsError; // Ошибка длины выходного буфера
						m_bFlagWorkRetry = true;
						if (m_flagInvNow)
							InvBuf(pOutData, outDataList.at(0)->RealLength());
						
						return ModuleError::WorkRetry;
					}
					else
					{
						//мы здесь из WorkRetry
						//отправляем сообщение о нахождении преамбулы
						m_bFlagPmbFind = true;
						if (m_iFlagTimeCnt == 1)
						{
							m_info.SendMsg(TParam::Msg_PreamblFind, cast_str(m_iPrPos));
							double dBER = (double)m_iErrors / (double)(m_vucPreambul.at(m_iCurPmb).size() * 8);
							m_info.SendMsg(TParam::Msg_BER, cast_str(dBER));
							if (m_vucPreambul.size() > 1)
							{
								m_info.SendMsg(TParam::Msg_PreamblFindNum, cast_str(m_iCurPmb));
							}
							m_iPrPos = (m_iPrPos / iLen) * (iLen + 1);
						}
						else
						{
							m_info.SendMsg(TParam::Msg_PreamblFind);
							double dBER = (double)m_iErrors / (double)(m_vucPreambul.at(m_iCurPmb).size() * 8);
							m_info.SendMsg(TParam::Msg_BER, cast_str(dBER));
							if (m_vucPreambul.size() > 1)
							{
								m_info.SendMsg(TParam::Msg_PreamblFindNum, cast_str(m_iCurPmb));
							}
						}

						//подаем целое число байт после преамбулы на выход
						int iBitWrite = (iLen+m_iTmpBufLen)*8 - m_iPrevWritePos - m_iPreamblLen*8 - m_iPrevBits;//столько бит надо записать
						int iBitToOut = iBitWrite + m_iBitWrite; //столько бит пойдет на выход
						//вычисляем сколько бит можно дописать
						int iBitWriteToOut = iBitWrite - iBitToOut%8;
						int iNumByte = (iBitWriteToOut+m_iBitWrite)/8; //столько байт надо дать на выход
						//даем 
						m_cFindPmb.cutBits(pucInBuf,pOutData,m_iPrevWritePos,m_iBitWrite,iBitWriteToOut);
						//записываем сколько бит уже пропущено после преамбулы
						if(iNumByte*8 >= m_iBitMiss)
						{
							//если уже хватает для пропуска
							m_iAlradyMiss = m_iBitMiss;
						}
						else
						{
							m_iAlradyMiss = iNumByte*8;
						}
						// Записываем реальную длину выходного блока данных
						if (outDataList.at(0)->SetRealLength(iNumByte)>0) 
							return ModuleError::BufLengsError; // Ошибка длины выходного буфера
						//сохраняем недописанный байт
						m_iNumEndBits = iBitToOut%8;
						if(m_iNumEndBits != 0 )
						{	//сохраняем эти биты
							m_ucEndByte = pucInBuf[iLen - 1];
						}
						else
							m_ucEndByte = 0;
						//копируем хвост для обнаружения преамбулы на стыке
						memcpy(m_pucTmpBuf, pInData+iLen-m_iPreamblLen - m_iPrevBits/8, m_iPreamblLen + m_iPrevBits/8);
						m_iTmpBufLen = m_iPreamblLen + m_iPrevBits/8;
					}
				}
				else
				{
					//преамбулы до этого еще не было
					//отправляем сообщение о нахождении преамбулы
					m_bFlagPmbFind = true;
					if (m_iFlagTimeCnt == 1)
					{
						double dBER = (double)m_iErrors / (double)(m_vucPreambul.at(m_iCurPmb).size() * 8);
						m_info.SendMsg(TParam::Msg_BER, cast_str(dBER));
						m_info.SendMsg(TParam::Msg_PreamblFind, cast_str(m_iPrPos));
						
						
						if (m_vucPreambul.size() > 1)
						{
							m_info.SendMsg(TParam::Msg_PreamblFindNum, cast_str(m_iCurPmb));
						}
						m_iPrPos = (m_iPrPos / iLen) * (iLen + 1);
					}
					else
					{
						m_info.SendMsg(TParam::Msg_PreamblFind);
						double dBER = (double)m_iErrors / (double)(m_vucPreambul.at(m_iCurPmb).size() * 8);
						m_info.SendMsg(TParam::Msg_BER, cast_str(dBER));
						if (m_vucPreambul.size() > 1)
						{
							m_info.SendMsg(TParam::Msg_PreamblFindNum, cast_str(m_iCurPmb));
						}
					}
					
					//подаем целое число байт после преамбулы на выход
					int iBitWrite = (iLen+m_iTmpBufLen)*8 - m_iPrevWritePos - m_iPreamblLen*8 - m_iPrevBits;//столько бит надо записать
					int iBitToOut = iBitWrite + m_iBitWrite; //столько бит пойдет на выход
					//вычисляем сколько бит можно дописать
					int iBitWriteToOut = iBitWrite - iBitToOut%8;
					int iNumByte = (iBitWriteToOut+m_iBitWrite)/8; //столько байт надо дать на выход
					m_ibufsize = iNumByte;
					//даем
					m_cFindPmb.cutBits(pucInBuf,pOutData,m_iPrevWritePos,m_iBitWrite,iBitWriteToOut);
					//записываем сколько бит уже пропущено после преамбулы
					if(iNumByte*8 >= m_iBitMiss)
					{
						//если уже хватает для пропуска
						m_iAlradyMiss = m_iBitMiss;
					}
					else
					{
						m_iAlradyMiss = iNumByte*8;
					}
					// Записываем реальную длину выходного блока данных
					if (outDataList.at(0)->SetRealLength(iNumByte)>0) 
						return ModuleError::BufLengsError; // Ошибка длины выходного буфера
					//сохраняем недописанный байт
					m_iNumEndBits = iBitToOut%8;
					if(m_iNumEndBits != 0)
					{	//сохраняем эти биты
						m_ucEndByte = pucInBuf[iLen-1];
					}
					else
						m_ucEndByte = 0;
					//копируем хвост для обнаружения преамбулы на стыке
					memcpy(m_pucTmpBuf,pInData+iLen-m_iPreamblLen-m_iPrevBits/8,m_iPreamblLen+m_iPrevBits/8);
					m_iTmpBufLen = m_iPreamblLen + m_iPrevBits/8;
				}

			}
			else
			{
				//преамбула не найдена
				//проверяем была ли до этого преамбула
				if(m_bFlagPmbFind)
				{
					//преамбула была
					//подаем целое число байт после преамбулы на выход
					int iBitWrite = (iLen+m_iTmpBufLen)*8 - m_iPreamblLen*8 - m_iPrevBits;//столько бит надо записать
					int iBitToOut = iBitWrite + m_iBitWrite; //столько бит пойдет на выход
					//вычисляем сколько бит можно дописать
					int iBitWriteToOut = iBitWrite - iBitToOut%8;
					int iNumByte = (iBitWriteToOut+m_iBitWrite)/8; //столько байт надо дать на выход
					m_ibufsize = iNumByte;
					//даем
					m_cFindPmb.cutBits(pucInBuf,pOutData,0,m_iBitWrite,iBitWriteToOut);
					//записываем сколько бит уже пропущено после преамбулы
					if(iNumByte*8 + m_iAlradyMiss>= m_iBitMiss)
					{
						//если уже хватает для пропуска
						m_iAlradyMiss = m_iBitMiss;
					}
					else
					{
						m_iAlradyMiss += iNumByte*8;
					}

					// Записываем реальную длину выходного блока данных
					if (outDataList.at(0)->SetRealLength(iNumByte)>0) 
						return ModuleError::BufLengsError; // Ошибка длины выходного буфера
					//сохраняем недописанный байт
					m_iNumEndBits = iBitToOut%8;
					if(m_iNumEndBits != 0)
					{	//сохраняем эти биты
						m_ucEndByte = pucInBuf[iLen-1];
					}
					else
						m_ucEndByte = 0;
					//копируем хвост для обнаружения преамбулы на стыке
					memcpy(m_pucTmpBuf,pInData+iLen-m_iPreamblLen-m_iPrevBits/8,m_iPreamblLen+m_iPrevBits/8);
					m_iTmpBufLen = m_iPreamblLen + m_iPrevBits/8;
				}
				else
				{
					//преамбулы не было
					//записываем, что недописанных бит у нас нет
					
					if (m_iFlagSetDataToOut == 1)
					{
						memset(pOutData, 0, iLen);
						memcpy(pOutData, pInData, iLen);
						outDataList.at(0)->SetRealLength(iLen);
					}
					m_iNumEndBits = 0;
					m_ucEndByte = 0;
					//копируем хвост для обнаружения преамбулы на стыке
					memcpy(m_pucTmpBuf,pInData+iLen-m_iPreamblLen-m_iPrevBits/8,m_iPreamblLen+m_iPrevBits/8);
					m_iTmpBufLen = m_iPreamblLen+m_iPrevBits/8;
				}
			}
			if (m_flagInvNow)
				InvBuf(pOutData, outDataList.at(0)->RealLength());
			
			if (pucInBuf)
				delete[] pucInBuf;
		}
		else 
			return ModuleError::NoDataForProcess;
		m_bFlagWorkRetry = false;
		return ModuleError::Ok;
	}

	// Функция динамического изменения параметров модуля.
	//  pParam - параметр
	int Apply(const TModuleParam* pParam)
	{ 
		if (pParam->name == m_info.GetParamName(TParam::Preambl)) 
			m_sPreambl = pParam->data;	
		if (pParam->name == m_info.GetParamName(TParam::Error)) 
			m_iError = cast_int(pParam->data);
		if (pParam->name == m_info.GetParamName(TParam::PrevBits)) 
			m_iPrevBits = cast_int(pParam->data);
		if (pParam->name == m_info.GetParamName(TParam::ByteMiss)) 
			m_iBitMiss = cast_int(pParam->data);
		if (pParam->name == m_info.GetParamName(TParam::FlagSyncOn)) 
			m_iFlagSyncOn = cast_int(pParam->data);
		if (pParam->name == m_info.GetParamName(TParam::FlagTimeCnt))
			m_iFlagTimeCnt = cast_int(pParam->data);
		if (pParam->name == m_info.GetParamName(TParam::FixSkkInv))
			m_uiFixSkkInv = cast_uint(pParam->data);
			
		return ModuleError::Ok;
	}

	// Функция приостановки процесса обработки
	int Pause() 
	{ 
		return ModuleError::Ok;
	}

	// Функция завершения процесса обработки
	int Stop()
	{
		return ModuleError::Ok;
	}

	// Функция возврата текущих параметров модуля
	TModuleContext* Parameters() 
	{
		return &m_info;
	}

	// Статическая функция возвращает информацию о модуле
	static TModuleDescr* Description()
	{	
		return &g_description; 
	}
	
	bool HexStringToBytes(string_type& sValue, uchar*& pOut, int& nOutLen)
	{
		if (pOut)
			delete[] pOut;

		if ((sValue.size() % 2) == 1)
		{
			sValue = STR("0") + sValue;
		}
		if ((sValue.size() % 2) == 0)
		{
			std::string sHex = str_utils::aFromLocal(sValue);

			if (sHex[0] == '0' && sHex[1] == 'x')
				sHex = sHex.substr(2, sHex.size() - 2);

			nOutLen = static_cast<int>(sHex.size()) / 2;

			if (sHex.size() >= 2)
			{
				pOut = new uchar[nOutLen];

				for (unsigned int i = 0, j = 0; i < sHex.size(); i += 2, j++)
				{
					int nByte = 0;
					std::string sByte = sHex.substr(i, 2);
					std::basic_istringstream<char> sis(sByte);
					sis >> std::hex >> nByte;
					pOut[j] = static_cast<uchar>(nByte);
				}

				return true;
			}

		}


		return false;
	}

	bool HexStringToBytes(string_type& sValue, std::vector<uchar>& vOut, std::vector<uchar>& vOutMask )
	{
		if ((sValue.size() % 2) == 1)
		{
			sValue = STR("0") + sValue;
		}

		if ((sValue.size() % 2) == 0)
		{
			std::string sHex = str_utils::aFromLocal(sValue);

			if (sHex[0] == '0' && sHex[1] == 'x')
				sHex = sHex.substr(2, sHex.size() - 2);

			int nOutLen = static_cast<int>(sHex.size())/2;

			if (sHex.size() >= 2)
			{
				vOut.resize(nOutLen);
				vOutMask.resize(nOutLen);

				for (unsigned int i = 0, j = 0; i < sHex.size(); i += 2, j++)
				{
					int nByte = 0;
					std::string sByte = sHex.substr(i, 2);
					if (sByte[0] != '*')
					{
						if (sByte[1] != '*')
						{
							std::basic_istringstream<char> sis(sByte);
							sis >> std::hex >> nByte;
							vOutMask[j] = 0;
							vOut[j] = static_cast<uchar>(nByte);
						}
						else
						{
							vOut.resize(vOut.size()-1);
							vOutMask.resize(vOut.size() - 1);
							j--;
						}

					}
					else
					{
						if (sByte[1] == '*')
						{
							vOut[j] = 0;
							vOutMask[j] = 1;
						}
						else
						{
							vOut.resize(vOut.size()-1);
							vOutMask.resize(vOut.size() - 1);
							j--;
						}
					}
				}

				return true;
			}
		}

		return false;
	}

	void InvBuf(unsigned char* outData, int outLength)
	{
		for (int i = 0; i < outLength; ++i) {
			outData[i] = ReverseByte[outData[i]];
		}
	}

};

DECLARE_AS_MODULE(CeSyncMultyPmb);

#endif // eSyncMultyPmb_h__
