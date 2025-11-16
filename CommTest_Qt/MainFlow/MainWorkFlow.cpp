#include "MainWorkFlow.h"
#include "CommTest_Qt.h"


//初始化静态实例
MainWorkFlow* MainWorkFlow::s_pInstance = nullptr;
QMutex MainWorkFlow::s_mutex;

MainWorkFlow::MainWorkFlow(QObject* pParent /*= nullptr*/)
    : QObject(pParent), m_RegisterVal(REGISTER_VAL_NUM)
{
	m_pComm = nullptr;
	m_pCommInfo = nullptr;

	m_bValidComm = false;
	m_pComProBase = nullptr;
	
	for (int i = 0 ; i < REGISTER_VAL_NUM;i++)
	{
		m_RegisterVal[i].store(0, std::memory_order_relaxed);
	}

	m_bDataChanged = false;

	//m_pLuaScript = nullptr;
    m_vpLuaScript.resize(LUA_SCRIPT_NUM);
    m_vLuaMutex.resize(LUA_SCRIPT_NUM);
    for (int i = 0; i < LUA_SCRIPT_NUM; ++i)
    {
        m_vpLuaScript[i] = std::unique_ptr<LuaScript>(LuaScript::InitialLuaScript());
        ConnectLuaSignalSlot(m_vpLuaScript[i]);
        m_vLuaMutex[i] = std::make_unique<QMutex>();
    }

    m_luaThreadPool = new QThreadPool(this);
    m_luaThreadPool->setMaxThreadCount(QThread::idealThreadCount());


	

	
}

// 析构函数：确保所有资源正确释放
MainWorkFlow::~MainWorkFlow()
{
	// 关闭通信
	if (m_pComm != nullptr)
	{
		m_pComm->Close();
		delete m_pComm;
		m_pComm = nullptr;
	}
	
	// 释放协议实例
	if (m_pComProBase != nullptr)
	{
		delete m_pComProBase;
		m_pComProBase = nullptr;
	}
	
	// m_pCommInfo 会自动释放（unique_ptr）
	// m_vpLuaScript 会自动释放（vector的unique_ptr）
}

void MainWorkFlow::ConnectLuaSignalSlot(std::unique_ptr<LuaScript> &pLuaScript)
{
		//连換lua实例的SetRegisterValInt16
	connect(pLuaScript.get(), &LuaScript::SetRegisterValInt16, this, [this](int Addr, int16_t nVal) {
		SetRegisterVal(Addr, nVal);
		emit RegisterDataUpdate();
	});

	//连換lua实例的SetRegisterValInt32
	connect(pLuaScript.get(), &LuaScript::SetRegisterValInt32, this, [this](int Addr, int32_t nVal) {

		DataTypeConvert dataConvert;
		dataConvert.u_Int32[0] = nVal;
		SetRegisterVal(Addr, dataConvert.u_Int16[0]);
		SetRegisterVal(Addr+1, dataConvert.u_Int16[1]);
		emit RegisterDataUpdate();
	});

	//连換lua实例的SetRegisterValFloat
	connect(pLuaScript.get(), &LuaScript::SetRegisterValFloat, this, [this](int Addr, float nVal) {

		DataTypeConvert dataConvert;
		dataConvert.u_float[0] = nVal;
		SetRegisterVal(Addr, dataConvert.u_Int16[0]);
		SetRegisterVal(Addr+1, dataConvert.u_Int16[1]);

		emit RegisterDataUpdate();
	});

	//连換lua实例的SetRegisterValDouble
	connect(pLuaScript.get(), &LuaScript::SetRegisterValDouble, this, [this](int Addr, double nVal) {
		
		DataTypeConvert dataConvert;
		dataConvert.u_double = nVal;
		SetRegisterVal(Addr, dataConvert.u_Int16[0]);
		SetRegisterVal(Addr+1, dataConvert.u_Int16[1]);
		SetRegisterVal(Addr+2, dataConvert.u_Int16[2]);
		SetRegisterVal(Addr+3, dataConvert.u_Int16[3]);

		emit RegisterDataUpdate();
	});

	//连換lua实例的SetRegisterValString
	connect(pLuaScript.get(), &LuaScript::SetRegisterValString, this, [this](int Addr, QString strVal) {
		
		DataTypeConvert dataConvert;
		for (int i = 0; i < strVal.length() && i < 2; i++)
		{
			dataConvert.u_chars[i] = strVal[i].toLatin1();
		}
		SetRegisterVal(Addr, dataConvert.u_Int16[0]);

		emit RegisterDataUpdate();
	});

	// ===== 连接Get系列信号 =====
	//连換lua实例的GetRegisterValInt16
	connect(pLuaScript.get(), &LuaScript::GetRegisterValInt16, this, [this](int Addr, int16_t& nVal) {
		nVal = GetRegisterVal(Addr);
	});

	//连換lua实例的GetRegisterValInt32
	connect(pLuaScript.get(), &LuaScript::GetRegisterValInt32, this, [this](int Addr, int32_t& nVal) {
		DataTypeConvert dataConvert;
		dataConvert.u_Int16[0] = GetRegisterVal(Addr);
		dataConvert.u_Int16[1] = GetRegisterVal(Addr + 1);
		nVal = dataConvert.u_Int32[0];
	});

	//连換lua实例的GetRegisterValFloat
	connect(pLuaScript.get(), &LuaScript::GetRegisterValFloat, this, [this](int Addr, float& fVal) {
		DataTypeConvert dataConvert;
		dataConvert.u_Int16[0] = GetRegisterVal(Addr);
		dataConvert.u_Int16[1] = GetRegisterVal(Addr + 1);
		fVal = dataConvert.u_float[0];
	});

	//连換lua实例的GetRegisterValDouble
	connect(pLuaScript.get(), &LuaScript::GetRegisterValDouble, this, [this](int Addr, double& dVal) {
		DataTypeConvert dataConvert;
		dataConvert.u_Int16[0] = GetRegisterVal(Addr);
		dataConvert.u_Int16[1] = GetRegisterVal(Addr + 1);
		dataConvert.u_Int16[2] = GetRegisterVal(Addr + 2);
		dataConvert.u_Int16[3] = GetRegisterVal(Addr + 3);
		dVal = dataConvert.u_double;
	});

	//连換lua实例的GetRegisterValString
	connect(pLuaScript.get(), &LuaScript::GetRegisterValString, this, [this](int Addr, QString& strVal) {
		DataTypeConvert dataConvert;
		dataConvert.u_Int16[0] = GetRegisterVal(Addr);
		strVal = QString("%1%2").arg(QChar(dataConvert.u_chars[0])).arg(QChar(dataConvert.u_chars[1]));
	});
}

//初始化静态实例
MainWorkFlow* MainWorkFlow::InitialWorkFlow(QObject* pParent /*= nullptr*/)
{
	QMutexLocker locker(&s_mutex);

	//不存在则创建
	if (s_pInstance == nullptr) 
	{
		s_pInstance = new MainWorkFlow(pParent);
	}

	return s_pInstance;
	
}

//释放单例实例
void MainWorkFlow::ReleaseWorkFlow()
{
	QMutexLocker locker(&s_mutex);
	
	if (s_pInstance != nullptr)
	{
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

bool MainWorkFlow::SetCommInfo(CommBase::CommInfoBase* commInfo)
{
	if (commInfo == nullptr) return false;

	// 使用unique_ptr管理内存
	m_pCommInfo = commInfo;
	return true;
}

CommBase::CommInfoBase* MainWorkFlow::GetCommInfo()
{
	if (nullptr == m_pCommInfo) return nullptr;

	return m_pCommInfo;
}

bool MainWorkFlow::OpenComm()
{
	if (m_pCommInfo->GetCommType() == CommBase::CommType::eSocket)
	{
		//删除原本的通信实例
		if (m_pComm != nullptr)
		{
			m_pComm->Close();
			delete m_pComm;
			m_pComm = nullptr;
		}

		//通信终止符
		if (("CR") == m_pCommInfo->m_strCommStop)
		{
			QString strCR = QString(char(0x0D));
			QString strCommStop = strCR;
			m_pCommInfo->m_strCommStop = strCommStop;
		}
		else if (("CRLF") == m_pCommInfo->m_strCommStop)
		{
			QString strCR = QString(char(0x0D));
			QString strLF = QString(char(0x0A));
			QString strCommStop = strCR + strLF;
			m_pCommInfo->m_strCommStop = strCommStop;
		}

		m_pComm = new CommSocket(this);

		//新建信号槽连接
		{
			connect(m_pComm, &CommBase::CommLogRecord, this, [this](QString strLog) {
				emit commLogRecord(strLog);
			});

			connect(m_pComm, &CommBase::dataReceived, this, [this](QString objectInfo,QByteArray strData) {
				emit dataReceived(objectInfo,strData);
			});

			connect(m_pComm, &CommBase::dataSend, this, [this](QString objectInfo, QByteArray strData) {
				emit dataSend(objectInfo, strData);
				});
		}

		m_bValidComm = m_pComm->Open(m_pCommInfo);


		return m_bValidComm;
	}
	else if (m_pCommInfo->GetCommType() == CommBase::CommType::eSerial)
	{
		return false;
	}

	return false;

}

bool MainWorkFlow::CloseComm()
{
	if (m_pComm == nullptr)
	{
		m_bValidComm = false;
		return true;
	}

	if (m_pComm->Close())
	{
		delete m_pComm;
		m_pComm = nullptr;

		m_bValidComm = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool MainWorkFlow::IsCommOpen()
{
	return m_bValidComm;
}

bool MainWorkFlow::CreateCommProtocol(ProtocolType ProType)
{
	switch (ProType)
	{
	case ProtocolType::eProRegMitsubishiQBinary:
	{
		if (m_pComProBase != nullptr)
		{
			delete m_pComProBase;
			m_pComProBase = nullptr;
		}

		m_pComProBase = new CommProMitsubishiQBinary(this);
	}
	break;
	case ProtocolType::eProRegKeyencePCLink:				
	{
		if (m_pComProBase != nullptr)
		{
			delete m_pComProBase;
			m_pComProBase = nullptr;
		}

		m_pComProBase = new CommProKeyencePCLink(this);
	}
	break;

	default:
		if (m_pComProBase != nullptr)
		{
			delete m_pComProBase;
			m_pComProBase = nullptr;
		}
		break;
	}

	if (m_pComProBase == nullptr) return false;

	return true;
}

// void MainWorkFlow::WorkProcess(QByteArray& RecInfo)
// {
// 	if (RecInfo  == "") return;
// 
// 	CmdType CurrentCmd = CmdType::eCmdUnkown;
// 
// 	//1. 先解析接收到的内容是否符合当前通信协议格式
// 	if (!WorkProcess_AnalyzeReceiveInfo(RecInfo, CurrentCmd))
// 		return;
// 
// 	QByteArray strSend;
// 
// 	int nCurAddr = 0;
// 	int nDataNum = 0;
// 
// 	//2. 解析当前指令的详细信息
// 	switch (CurrentCmd)
// 	{
// 	case CmdType::eCmdWriteReg:
// 
// 		if (!WorkProcess_WriteReg(RecInfo, strSend, nCurAddr, nDataNum))
// 		{
// 			return;
// 		}
// 		break;
// 	case CmdType::eCmdReadReg:
// 		if (!WorkProcess_ReadReg(RecInfo, strSend, nCurAddr, nDataNum))
// 		{
// 			return;
// 		}
// 		break;
// 	default:
// 		return;
// 		break;
// 	}
// 
// 	//3. 若是写入指令需要发送一个更新GUI表格显示的信号
// 	if (m_bDataChanged)
// 	{
// 		emit RegisterDataUpdate();
// 		m_bDataChanged = false;
// 	}
// 
// 
// 	//4. 回复客户端消息
// 	WorkProcess_SendCommInfo(strSend);
// 
// }

bool MainWorkFlow::ProcessRequest(const QByteArray& RecInfo, QByteArray& Reply)
{
    if (RecInfo == "") return false;

    if (m_pComProBase == nullptr) return false;

	std::unique_ptr<CommProtocolBase> pro;
   // CommProtocolBase* pro = nullptr;
    if (dynamic_cast<CommProMitsubishiQBinary*>(m_pComProBase) != nullptr)
    {
        pro = std::make_unique<CommProMitsubishiQBinary>(nullptr);
    }
    else if (dynamic_cast<CommProKeyencePCLink*>(m_pComProBase) != nullptr)
    {
		pro = std::make_unique<CommProKeyencePCLink>(nullptr);
    }
    else
    {
        return false;
    }

    CmdType CurrentCmd = CmdType::eCmdUnkown;
    if (!pro->AnalyzeCmdInfo(RecInfo, CurrentCmd))
    {
        //delete pro;
        return false;
    }

    int nCurAddr = 0;
    int nDataNum = 0;

    switch (CurrentCmd)
    {
    case CmdType::eCmdWriteReg:
    {
        long nCmdRegAddr = 0;
        int nCmdRedNum = 0;
        std::vector<int16_t> vnCmdWriteData;
        if (!pro->AnalyzeWriteReg(RecInfo, nCmdRegAddr, nCmdRedNum, vnCmdWriteData))
        {
           // delete pro;
            return false;
        }
        nCurAddr = nCmdRegAddr;
        nDataNum = nCmdRedNum;
        for (int i = 0; i < nCmdRedNum; i++)
        {
            int nPLCAddr = nCmdRegAddr + i;
            if (nPLCAddr > m_RegisterVal.size())
            {
                break;
            }
            int nPreData = m_RegisterVal.at(nPLCAddr).load();
            m_RegisterVal.at(nPLCAddr).store(vnCmdWriteData.at(i));
            if (nPreData != vnCmdWriteData.at(i))
            {
                m_bDataChanged = true;
            }
        }
        QByteArray strSend;
        if (!pro->PackReportWriteRegInfo(strSend))
        {
            //delete pro;
            return false;
        }
        Reply = strSend;
    }
    break;
    case CmdType::eCmdReadReg:
    {
        long nCmdRegAddr = 0;
        int nCmdRedNum = 0;
        if (!pro->AnalyzeReadReg(RecInfo, nCmdRegAddr, nCmdRedNum))
        {
          //  delete pro;
            return false;
        }
        nCurAddr = nCmdRegAddr;
        nDataNum = nCmdRedNum;
        std::vector<int16_t> vnCmdData;
        vnCmdData.resize(nCmdRedNum);
        for (int i = 0; i < nCmdRedNum; i++)
        {
            int nPLCAddr = nCmdRegAddr + i;
            if (nPLCAddr > m_RegisterVal.size())
            {
                break;
            }
            vnCmdData.at(i) = m_RegisterVal.at(nPLCAddr).load();
        }
        QByteArray strSend;
        if (!pro->PackReportReadRegInfo(strSend, nCmdRegAddr, nCmdRedNum, vnCmdData))
        {
          //  delete pro;
            return false;
        }
        Reply = strSend;
    }
    break;
    default:
       // delete pro;
        return false;
    }

    if (m_bDataChanged)
    {
        emit RegisterDataUpdate();
        m_bDataChanged = false;
    }

  //  delete pro;
    return true;
}

CommBase* MainWorkFlow::GetCommBase()
{
	if (m_pComm != nullptr)
	{
		return m_pComm;
	}

	return nullptr;
}

long MainWorkFlow::GetRegisterNum()
{
	return m_RegisterVal.size();
}

int16_t MainWorkFlow::GetRegisterVal(int Addr)
{
	if (Addr >= m_RegisterVal.size()) return 0;

	return m_RegisterVal.at(Addr).load();
}

bool MainWorkFlow::SetRegisterVal(int Addr, const int16_t& nsetVal)
{
	if (Addr >= m_RegisterVal.size())	return false;
	
	m_RegisterVal.at(Addr).store(nsetVal);

	return true;
}

bool MainWorkFlow::ResetAllRegisters(int16_t nsetVal)
{
	for (auto& i : m_RegisterVal)
	{
		i.store(nsetVal);
	}

	emit RegisterDataUpdate();

	return true;
}

bool MainWorkFlow::RunLuaScript(int nLuaIndex, const QString &strLuaFile)
{
    return RunLuaScriptAsync(nLuaIndex, strLuaFile);
}

bool MainWorkFlow::RunLuaScriptAsync(int nLuaIndex, const QString &strLuaFile)
{
    if (nLuaIndex < 0 || nLuaIndex >= LUA_SCRIPT_NUM) return false;
    if (m_vpLuaScript[nLuaIndex] == nullptr) return false;
    class LuaTask : public QRunnable {
    public:
        MainWorkFlow* self;
        int idx;
        QString file;
        LuaTask(MainWorkFlow* s, int i, const QString& f) : self(s), idx(i), file(f) {}
        void run() override {
            QMutexLocker locker(self->m_vLuaMutex[idx].get());
            QString err;
            bool ok = self->m_vpLuaScript[idx]->RunLuaScript(file, err);
            if (ok) {
                QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
            } else {
                QMetaObject::invokeMethod(self, "commLogRecord", Qt::QueuedConnection,
                                          Q_ARG(QString, QString("Lua执行失败:%1").arg(err)));
            }
        }
    };
    LuaTask* t = new LuaTask(this, nLuaIndex, strLuaFile);
    t->setAutoDelete(true);
    m_luaThreadPool->start(t);
    return true;
}

LuaScript* MainWorkFlow::GetLuaScript(int nIndex)
{
    if (nIndex >= m_vpLuaScript.size()) return nullptr;
	
	return m_vpLuaScript[nIndex].get();
}

// bool MainWorkFlow::WorkProcess_AnalyzeReceiveInfo(QByteArray& strRecevie,CmdType& CurCmdType)
// {
// 	if (m_pComProBase == nullptr)	return false;
// 
// 	//2. 解析数据
// 	if (!m_pComProBase->AnalyzeCmdInfo(strRecevie, CurCmdType))
// 	{
// // 		if (m_pComm != nullptr)
// // 		{
// // 			m_pComm->SendData("");
// // 		}
// 		return false;
// 	}
// 
// 	return true;
// }
// 
// bool MainWorkFlow::WorkProcess_WriteReg(const QByteArray& strRecevie, QByteArray& strSend, int& nAddress, int& nDataNum)
// {
// 	//2.20250612	wm	解析客户端信息
// 	if (m_pComProBase == nullptr)	return false;
// 
// 	long nCmdRegAddr = 0;
// 	int nCmdRedNum = 0;
// 
// 	std::vector<int16_t> vnCmdWriteData;
// 
// 	if (!m_pComProBase->AnalyzeWriteReg(strRecevie, nCmdRegAddr, nCmdRedNum, vnCmdWriteData))
// 	{
// 		return false;
// 	}
// 
// 	nAddress = nCmdRegAddr;
// 	nDataNum = nCmdRedNum;
// 	//3.20250612	wm	根据客户端信息进行相应处理，写入数据值
// 	for (int i = 0; i < nCmdRedNum; i++)
// 	{
// 		int nPLCAddr = nCmdRegAddr + i;
// 		if (nPLCAddr > m_RegisterVal.size())
// 		{
// 			break;
// 		}
// 		int nPreData = m_RegisterVal.at(nPLCAddr).load();
// 
// 		m_RegisterVal.at(nPLCAddr).store(vnCmdWriteData.at(i));
// 
// 		if (nPreData != vnCmdWriteData.at(i))
// 		{
// 			m_bDataChanged = true;
// 		}
// 	}
// 
// 
// 	//4.20250612	wm	打包回复给客户端的信息
// 	if (!m_pComProBase->PackReportWriteRegInfo(strSend))
// 	{
// 		return false;
// 	}
// 
// 	return true;
// }
// 
// bool MainWorkFlow::WorkProcess_ReadReg(const QByteArray& strRecevie, QByteArray& strSend, int& nAddress, int& nDataNum)
// {
// 	//2.20250612	wm	解析客户端信息
// 	if (m_pComProBase == nullptr)	return false;
// 
// 	long nCmdRegAddr = 0;
// 	int nCmdRedNum = 0;
// 
// 	if (!m_pComProBase->AnalyzeReadReg(strRecevie, nCmdRegAddr, nCmdRedNum))
// 	{
// 		return false;
// 	}
// 
// 	nAddress = nCmdRegAddr;
// 	nDataNum = nCmdRedNum;
// 	//3.20250612	wm	根据客户端信息进行相应处理,读取数据值
// 	std::vector<int16_t> vnCmdData;
// 	vnCmdData.resize(nCmdRedNum);
// 
// 	for (int i = 0; i < nCmdRedNum; i++)
// 	{
// 		int nPLCAddr = nCmdRegAddr + i;
// 		if (nPLCAddr > m_RegisterVal.size())
// 		{
// 			break;
// 		}
// 
// 		vnCmdData.at(i) = m_RegisterVal.at(nPLCAddr).load();
// 	}
// 
// 	//4.20250612	wm	打包回复给客户端的信息
// 	if (!m_pComProBase->PackReportReadRegInfo(strSend, nCmdRegAddr, nCmdRedNum, vnCmdData))
// 	{
// 		return false;
// 	}
// 
// 
// 	return true;
// }
// 
// bool MainWorkFlow::WorkProcess_SendCommInfo(const QByteArray& strSend)
// {
// 	if (m_pComm == nullptr)	return false;
// 
// 	if (!m_pComm->SendData(strSend)) return false;
// 
// 	return true;
// }
