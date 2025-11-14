#ifndef COMM_PROTOCOL_BASE_H
#define COMM_PROTOCOL_BASE_H
#include <QObject>
//协议枚举
enum class ProtocolType
{
	eProUnknown = -1,										// 未知的通信协议
	eProCmdFast = 0,										// 无协议

	eProRegMitsubishiQAscii = 10,							// 三菱MC 3E帧ASCII通信协议
	eProRegMitsubishiQBinary = 11,							// 三菱MC 3E帧二进制通信协议

	eProRegKeyencePCLink = 20,								// 基恩士KV系列上位链路协议
	eProRegKeyenceWithMitsubishiQAscii = 21,				// 基恩士KV系列使用三菱Q系列PLC的寄存器网口MC（3E）ASCII协议   (QnA兼容3E)
	eProRegKeyenceWithMitsubishiQBinary = 22,				// 基恩士KV系列使用三菱Q系列PLC的寄存器网口MC（3E）二进制协议  (QnA兼容3E)
};

enum class CmdType //指令类型
{
	eCmdUnkown = -1,		//未定义
	eCmdWriteReg = 0,		//写寄存器
	eCmdReadReg = 1,		//读寄存器

};

//数据类型枚举类
enum class RegisterDataType
{
	eDataTypeUnkown = -1,
	eDataTypeChar8,
	eDataTypeInt16,
	eDataTypeInt32,
	eDataTypeFloat,
	eDataTypeDouble,
};

typedef union tagDataTypeConvert //20250606	wm	数据转换的联合体,直接将相同内存中的数据按照需求转换成不同的数据格式
{
	uint8_t  u_chars[8];	//字符型，每个char对应8bit
	int16_t	 u_Int16[4];	//16位整型，分别对应低高16位
	int32_t  u_Int32[2];	//32位整型
	float	 u_float[2];	//浮点型，32位
	double   u_double;		//浮点型，64位

	tagDataTypeConvert()
	{
		u_double = 0;
	}

}DataTypeConvert;

enum class ProcessType
{
	eProcessUnDef = -1,
	eProcessRece,
	eProcessSend,
};

class CommProtocolBase : public QObject
{
	Q_OBJECT
public:
	explicit CommProtocolBase(QObject* pParent = nullptr) : QObject(pParent) {}

	CommProtocolBase(const CommProtocolBase& other) = delete;
	CommProtocolBase& operator= (const CommProtocolBase& other) = delete;
	virtual ~CommProtocolBase() = default;

public:

// 	//template <typename T>
// 	virtual bool PackWriteRegInfo(QByteArray& strInfo, long nRegAddr, int nWriteNum, std::vector<uint16_t> vWriteData) = 0;	//打包写寄存器信息字符串
// 	virtual bool AnalyzeAswWriteReg(QByteArray strAsw) = 0;	//解析写寄存器回复信息(是否写成功)
// 
// 	virtual bool PackReadRegInfo(QByteArray& strInfo, long nRegAddr, int nReadNum, bool bDWORD) = 0;	//打包读寄存器信息字符串
// 	//template <typename T>
// 	virtual bool AnalyzeAswReadReg(QByteArray strAsw, int nReadNum, std::vector<uint16_t>& vReceiveData) = 0; //解析读寄存器回复信息(解析寄存器中的详细数值)

	

	virtual bool AnalyzeCmdInfo(QByteArray strInfo, CmdType& cCmdType) = 0;

	//20251101	wm	解析读寄存器指令
	virtual bool AnalyzeReadReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum) = 0;

	//20251101	wm	打包回复读寄存器指令信息
	virtual bool PackReportReadRegInfo(QByteArray& strInfo, long nRegAddr, int nWriteNum, const std::vector<int16_t>& vWriteData) = 0;

	//20251101	wm	解析写寄存器指令
	virtual bool AnalyzeWriteReg(QByteArray strInfo, long& nRegAddr, int& nWriteNum, std::vector<int16_t>& vWriteData) = 0;

	//20251101	wm	打包回复写寄存器指令信息
	virtual bool PackReportWriteRegInfo(QByteArray& strInfo) = 0;

protected:
	//通信接收到的数据处理


	virtual bool CmdInfoProcessing(const QByteArray& strInfo, ProcessType Curtype, QByteArray& strOut) = 0;

	std::map<QByteArray, CmdType> m_mCmdInfoType; //20251101	wm	各种协议的字符串对应的指令
};



#endif	//COMM_PROTOCOL_BASE_H
