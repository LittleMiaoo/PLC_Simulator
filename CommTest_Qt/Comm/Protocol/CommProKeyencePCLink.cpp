#include "CommProKeyencePCLink.h"

CommProKeyencePCLink::CommProKeyencePCLink(QObject* pParent /*= nullptr*/)
	: CommProtocolBase(pParent)
{
	m_mCmdInfoType.insert(std::make_pair(("RDS"), CmdType::eCmdReadReg));
	m_mCmdInfoType.insert(std::make_pair(("WRS"), CmdType::eCmdWriteReg));
}

bool CommProKeyencePCLink::CmdInfoProcessing(const QByteArray& strInfo, ProcessType Curtype, QByteArray& strOut)
{
	//基恩士无需处理,直接使用即可
	return true;
}

bool CommProKeyencePCLink::AnalyzeCmdInfo(QByteArray strInfo, CmdType& cCmdType)
{
	QByteArray strCurCmdInfo = strInfo.mid(0, 3);
	cCmdType = CmdType::eCmdUnkown;

	if (m_mCmdInfoType.find(strCurCmdInfo) == m_mCmdInfoType.end()) return false;

	cCmdType = m_mCmdInfoType[strCurCmdInfo];
	return true;
}

bool CommProKeyencePCLink::AnalyzeReadReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum)
{
	QByteArray strCmdRead = ("RDS");
	QByteArray strSpace = (" ");

	QByteArray RegType = ("DM");
	QByteArray strDataFormat = (".H");

	int nLenRegAddr = 5;
	int nLenBeforeRegAddr = strCmdRead.length() + strSpace.length() + RegType.length();
	int nLenBeforeRegNum = nLenBeforeRegAddr + 5 + strDataFormat.length() + strSpace.length();

	if (strCmdRead.compare(strInfo.mid(0, 3)) != 0)	//先解析指令头是否为读指令
	{
		return false;
	}

	if (strDataFormat.compare(strInfo.mid(nLenBeforeRegAddr + 5, 2)) != 0)	//判断读写数据格式
	{
		return false;
	}

	QByteArray strRegAddr = strInfo.mid(nLenBeforeRegAddr, 5);
	QByteArray strRegNum = strInfo.mid(nLenBeforeRegNum, 4);

	bool bOk = false;
	nRegAddr = strRegAddr.toInt(&bOk);
	nWriteNum = strRegNum.toInt(&bOk);

	return true;
}

bool CommProKeyencePCLink::PackReportReadRegInfo(QByteArray& strInfo, long nRegAddr, int nWriteNum, const std::vector<int16_t>& vWriteData)
{
	QByteArray strReadData = ("");
	QByteArray strSpace = (" ");
	for (int i = 0; i < nWriteNum; i++)
	{
		int16_t nTemp = vWriteData.at(i) & 0xFFFF;

		QByteArray strTemp = QString("%1").arg(nTemp, 4, 16, QChar('0')).toUpper().toLatin1();

		strReadData = strReadData + strTemp.mid(0, 4) + strSpace;
	}

	int nLenStrReadData = strReadData.length();
	if (strReadData.at(nLenStrReadData - 1) == *strSpace.data())
	{
		strInfo = strReadData.mid(0, nLenStrReadData - 1);
	}
	else
	{
		strInfo = strReadData;
	}


	return true;
}

bool CommProKeyencePCLink::AnalyzeWriteReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum, std::vector<int16_t>& vWriteData)
{
	QByteArray strCmdWrite = ("WRS");
	QByteArray strSpace = (" ");
	QByteArray RegType = ("DM");
	QByteArray strDataFormat = (".H");

	int nLenRegAddr = 5;

	int nLenBeforeRegAddr = strCmdWrite.length() + strSpace.length() + RegType.length();
	int nLenBeforeRegNum = nLenBeforeRegAddr + 5 + strDataFormat.length() + strSpace.length();


	if (strCmdWrite.compare(strInfo.mid(0, 3)) != 0)	//先解析指令头是否为写指令
	{
		return false;
	}

	if (strDataFormat.compare(strInfo.mid(nLenBeforeRegAddr + 5, 2)) != 0)	//判断读写数据格式
	{
		return false;
	}

	QByteArray strRegAddr = strInfo.mid(nLenBeforeRegAddr, 5);
	QByteArray strRegNum = strInfo.mid(nLenBeforeRegNum, 4);

	bool bOk = false;
	nRegAddr = strRegAddr.toInt(&bOk);
	nWriteNum = strRegNum.toInt(&bOk);

	vWriteData.clear();
	vWriteData.resize(nWriteNum);
	for (int i = 0; i < nWriteNum; i++)
	{
		QByteArray strTemp = strInfo.mid(nLenBeforeRegNum + 5 + i * 5, 4);

		int16_t d = strTemp.toInt(&bOk,16);
		int16_t tmpInt16 = d & 0xFFFF;

		vWriteData.at(i) = d;
	}

	return true;
}

bool CommProKeyencePCLink::PackReportWriteRegInfo(QByteArray& strInfo)
{
	strInfo = ("OK");

	return true;
}
