#include "CommProMitsubishiQBinary.h"

CommProMitsubishiQBinary::CommProMitsubishiQBinary(QObject* pParent)
	: CommProtocolBase(pParent) 
{
	m_mCmdInfoType.insert(std::make_pair(QByteArray("01040000"), CmdType::eCmdReadReg));
	m_mCmdInfoType.insert(std::make_pair(QByteArray("01140000"), CmdType::eCmdWriteReg));
}

bool CommProMitsubishiQBinary::AnalyzeCmdInfo(QByteArray strInfo, CmdType& cCmdType)
{

	

	long nRegAddr = 0;
	int nRegNum = 0;

	QByteArray strCurCmdInfo = ("");

	if (!CheckCmdInfoValid(strInfo, nRegAddr, nRegNum, strCurCmdInfo)) return false;

	cCmdType = CmdType::eCmdUnkown;

	if (m_mCmdInfoType.find(strCurCmdInfo) == m_mCmdInfoType.end()) return false;

	cCmdType = m_mCmdInfoType[strCurCmdInfo];
	return true;
}

bool CommProMitsubishiQBinary::AnalyzeReadReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum)
{
	QByteArray tmp;
	if (!CheckCmdInfoValid(strInfo, nRegAddr, nWriteNum, tmp))
	{
		return false;
	}
	return true;
}

bool CommProMitsubishiQBinary::PackReportReadRegInfo(QByteArray& strInfo, long nRegAddr, int nWriteNum, const std::vector<int16_t>& vWriteData)
{
	QByteArray strHead1 = ("D00000FFFF0300");
	QByteArray strDataLen;
	QByteArray strEnd = ("0000");

	//strDataLen = QByteArray::number((nWriteNum + 1) * 2, 16);
	strDataLen = QString("%1").arg((nWriteNum + 1) * 2, 4, 16, QChar('0')).toUpper().toLatin1();

	strDataLen = strDataLen.mid(2, 2) + strDataLen.mid(0, 2);

	QByteArray strRegData;
	strRegData = ("");

	for (int i = 0; i < nWriteNum; i++)
	{
		QByteArray strCurData;
		//strCurData = QByteArray::number(vWriteData.at(i), 16);
		strCurData = QString("%1").arg(vWriteData.at(i), 4, 16, QChar('0')).toUpper().toLatin1();

		QByteArray strOut = strCurData.mid(2, 2) + strCurData.mid(0, 2);
		strRegData = strRegData + strOut;
	}

	strInfo = strHead1 + strDataLen + strEnd + strRegData;

	QByteArray strOut;

	CmdInfoProcessing(strInfo, ProcessType::eProcessSend, strOut);

	strInfo = strOut;

	return true;
}

bool CommProMitsubishiQBinary::AnalyzeWriteReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum, std::vector<int16_t>& vWriteData)
{
	QByteArray tmp;
	if (!CheckCmdInfoValid(strInfo, nRegAddr, nWriteNum, tmp))
	{
		return false;
	}

	vWriteData.resize(nWriteNum);

	for (int i = 0; i < nWriteNum; i++)
	{
		QByteArray strTemp = strInfo.mid(i * 4, 4);

		QByteArray str1 = strTemp.mid(2, 2) + strTemp.mid(0, 2);

		bool bOk = false;
		int16_t d = str1.toInt(&bOk, 16);

		vWriteData.at(i) = d & 0xFFFF;
	}

	return true;
}

bool CommProMitsubishiQBinary::PackReportWriteRegInfo(QByteArray& strInfo)
{
	QByteArray strHead1 = ("D00000FFFF0300");
	QByteArray strDataLen = ("0200");
	QByteArray strEnd = ("0000");

	strInfo = strHead1 + strDataLen + strEnd;

	QByteArray strOut;

	CmdInfoProcessing(strInfo, ProcessType::eProcessSend, strOut);

	strInfo = strOut;

	return true;
}

bool CommProMitsubishiQBinary::CheckCmdInfoValid(QByteArray& strInfo, long& nRegAddr, int& nRegNum, QByteArray& strCmdInfo)
{
	QByteArray strOut;
	if (!CmdInfoProcessing(strInfo, ProcessType::eProcessRece, strOut))
		return false;

	strInfo = strOut;

	QByteArray strHead1 = ("500000FFFF0300");	//指令头，包含
	QByteArray strDataLen;						//
	QByteArray strTime = ("1000");
	QByteArray strCmdWrite = ("01140000");
	QByteArray strCmdRead = ("01040000");
	QByteArray strAdrTpye = ("A8");


	//先判断指令头是否正常
	if (strHead1.compare(strInfo.mid(0, strHead1.length())) != 0)
	{
		return false;
	}

	//解析其中的数据长度
	strDataLen = strInfo.mid(strHead1.length(), 4);

	QByteArray strLen = strDataLen.mid(2, 2) + strDataLen.mid(0, 2);

	bool bOk = false;
	int nDataLen = strLen.toInt(&bOk, 16);/*_tcstoul(strLen, NULL, 16);*/

	//判断后续数据长度和指令信息中的数据长度是否符合
	QByteArray strCmdInfoAfterDataLen = strInfo.mid(strHead1.length() + 4);

	int nAllDataLen = strCmdInfoAfterDataLen.length() / 2;
	if (nDataLen != nAllDataLen)
	{
		return false;
	}

	//指令检查，暂时不支持其他指令
	if (strCmdWrite.compare(strCmdInfoAfterDataLen.mid(4, 8)) != 0 && strCmdRead.compare(strCmdInfoAfterDataLen.mid(4, 8)) != 0)
	{
		return false;
	}

	strCmdInfo = strCmdInfoAfterDataLen.mid(4, 8);

	QByteArray strAdr = strCmdInfoAfterDataLen.mid(12, 6);
	QByteArray strRegAdr = strAdr.mid(4, 2) + strAdr.mid(2, 2) + strAdr.mid(0, 2);
	nRegAddr = strRegAdr.toInt(&bOk, 16);/*_tcstoul(strRegAdr, NULL, 16);*/

	QByteArray strNum = strCmdInfoAfterDataLen.mid(20, 4);
	QByteArray strRegNum = strNum.mid(2, 2) + strNum.mid(0, 2);
	nRegNum = strRegNum.toInt(&bOk, 16);/*_tcstoul(strRegNum, NULL, 16);*/

	strInfo = strCmdInfoAfterDataLen.mid(24);
	return true;
}

bool CommProMitsubishiQBinary::CmdInfoProcessing(const QByteArray& strInfo, ProcessType Curtype, QByteArray& strOut)
{
	strOut.clear();

	if (Curtype == ProcessType::eProcessRece)
	{
		strOut = strInfo.toHex().toUpper();
	}
	else if (Curtype == ProcessType::eProcessSend)
	{
		strOut = QByteArray::fromHex(strInfo);
	}

	return true;
}
