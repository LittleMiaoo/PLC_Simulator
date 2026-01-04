#include "MainWorkFlow.h"
#include "CommTest_Qt.h"
#include "Comm/Socket/CommSocket.h"

// ScriptRunner 实现 - 用于 ScriptEditor 的异步脚本执行
class ScriptRunnerImpl : public IScriptRunner
{
public:
    ScriptRunnerImpl(MainWorkFlow* workflow, int luaIndex)
        : m_pWorkFlow(workflow), m_nLuaIndex(luaIndex) {}

    void RunScriptAsync(const QString& scriptContent,
                        std::function<void(bool success, const QString& errorMsg)> onFinished) override
    {
        if (!m_pWorkFlow) {
            if (onFinished) onFinished(false, "Workflow not available");
            return;
        }

        LuaScript* pLuaScript = m_pWorkFlow->GetLuaScript(m_nLuaIndex);
        if (!pLuaScript) {
            if (onFinished) onFinished(false, "LuaScript instance not found");
            return;
        }

        // 创建异步任务
        class EditorScriptTask : public QRunnable {
        public:
            LuaScript* lua;
            QString content;
            std::function<void(bool, const QString&)> callback;
            QMutex* mutex;

            EditorScriptTask(LuaScript* l, const QString& c,
                           std::function<void(bool, const QString&)> cb, QMutex* m)
                : lua(l), content(c), callback(std::move(cb)), mutex(m) {}

            void run() override {
                QMutexLocker locker(mutex);
                QString err;
                bool ok = lua->RunLuaScriptWithEditor(content, err);
                if (callback) {
                    callback(ok, err);
                }
            }
        };

        EditorScriptTask* task = new EditorScriptTask(
            pLuaScript, scriptContent, std::move(onFinished),
            m_pWorkFlow->m_vLuaMutex[m_nLuaIndex].get());
        task->setAutoDelete(true);
        m_pWorkFlow->m_luaThreadPool->start(task);
    }

private:
    MainWorkFlow* m_pWorkFlow;
    int m_nLuaIndex;
};

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

    m_strSendObjInfo = "";
    m_strSendData.clear();
    m_strRecObjInfo = "";
    m_strRecData.clear();
	
	for (int i = 0 ; i < REGISTER_VAL_NUM;i++)
	{
		m_RegisterVal[i].store(0, std::memory_order_relaxed);
	}

	m_bDataChanged = false;

	//m_pLuaScript = nullptr;
    m_vpLuaScript.resize(LUA_SCRIPT_NUM);
    m_vLuaMutex.resize(LUA_SCRIPT_NUM);
    struct RegisterProvider : public LuaScript::IDataProvider {
        MainWorkFlow* self;
        explicit RegisterProvider(MainWorkFlow* s) : self(s) {}
        int16_t GetInt16(int index) override { return self->GetRegisterVal(index); }
        int32_t GetInt32(int index) override {
            DataTypeConvert dt;
            dt.u_Int16[0] = self->GetRegisterVal(index);
            dt.u_Int16[1] = self->GetRegisterVal(index + 1);
            return dt.u_Int32[0];
        }
        float GetFloat(int index) override {
            DataTypeConvert dt;
            dt.u_Int16[0] = self->GetRegisterVal(index);
            dt.u_Int16[1] = self->GetRegisterVal(index + 1);
            return dt.u_float[0];
        }
        double GetDouble(int index) override {
            DataTypeConvert dt;
            dt.u_Int16[0] = self->GetRegisterVal(index);
            dt.u_Int16[1] = self->GetRegisterVal(index + 1);
            dt.u_Int16[2] = self->GetRegisterVal(index + 2);
            dt.u_Int16[3] = self->GetRegisterVal(index + 3);
            return dt.u_double;
        }
        QString GetString(int index) override {
            DataTypeConvert dt;
            dt.u_Int16[0] = self->GetRegisterVal(index);
            return QString("%1%2").arg(QChar(dt.u_chars[0])).arg(QChar(dt.u_chars[1]));
        }
        void SetInt16(int index, int16_t value) override {
            self->SetRegisterVal(index, value);
            QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
        }
        void SetInt32(int index, int32_t value) override {
            DataTypeConvert dt;
            dt.u_Int32[0] = value;
            self->SetRegisterVal(index, dt.u_Int16[0]);
            self->SetRegisterVal(index + 1, dt.u_Int16[1]);
            QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
        }
        void SetFloat(int index, float value) override {
            DataTypeConvert dt;
            dt.u_float[0] = value;
            self->SetRegisterVal(index, dt.u_Int16[0]);
            self->SetRegisterVal(index + 1, dt.u_Int16[1]);
            QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
        }
        void SetDouble(int index, double value) override {
            DataTypeConvert dt;
            dt.u_double = value;
            self->SetRegisterVal(index, dt.u_Int16[0]);
            self->SetRegisterVal(index + 1, dt.u_Int16[1]);
            self->SetRegisterVal(index + 2, dt.u_Int16[2]);
            self->SetRegisterVal(index + 3, dt.u_Int16[3]);
            QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
        }
        void SetString(int index, const QString& value) override {
            DataTypeConvert dt;
            for (int i = 0; i < value.length() && i < 2; ++i) {
                dt.u_chars[i] = value[i].toLatin1();
            }
            self->SetRegisterVal(index, dt.u_Int16[0]);
            QMetaObject::invokeMethod(self, "RegisterDataUpdate", Qt::QueuedConnection);
        }

        void MovePlatformAbsInt32(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
			int X, Y, Angle;
			X = GetInt32(Xaddr);
			Y = GetInt32(Yaddr);
			Angle = GetInt32(Angleaddr);
            self->m_pController->MovePlatformAbsInt32(X, Y, Angle);
        }
        void MovePlatformAbsFloat(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
			double X, Y, Angle;
			X = GetFloat(Xaddr);
			Y = GetFloat(Yaddr);
			Angle = GetFloat(Angleaddr);
            self->m_pController->MovePlatformAbsFloat(X, Y, Angle);
        }
        void MovePlatformRelativeInt32(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
            int X, Y, Angle;
            X = GetInt32(Xaddr);
            Y = GetInt32(Yaddr);
            Angle = GetInt32(Angleaddr);
            self->m_pController->MovePlatformRelativeInt32(X, Y, Angle);
        }
        void MovePlatformRelativeFloat(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
			double X, Y, Angle;
			X = GetFloat(Xaddr);
			Y = GetFloat(Yaddr);
			Angle = GetFloat(Angleaddr);
            self->m_pController->MovePlatformRelativeFloat(X, Y, Angle);
        }
        void WriteCurrentPosInt32(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
            int X, Y, Angle;
            self->m_pController->GetCurrentPosInt32(X, Y, Angle);
            SetInt32(Xaddr, X);
            SetInt32(Yaddr, Y);
            SetInt32(Angleaddr, Angle);
        }
        void WriteCurrentPosFloat(int Xaddr, int Yaddr, int Angleaddr) override {
            if (!self->m_pController) return;
            double dX, dY, dAngle;
            self->m_pController->GetCurrentPosFloat(dX, dY, dAngle);
            SetFloat(Xaddr, dX);
            SetFloat(Yaddr, dY);
            SetFloat(Angleaddr, dAngle);
        }
    };
    m_dataProvider = std::make_unique<RegisterProvider>(this);
    m_luaThreadPool = new QThreadPool(this);
    m_luaThreadPool->setMaxThreadCount(QThread::idealThreadCount());

    m_vScriptRunners.resize(LUA_SCRIPT_NUM);
    for (int i = 0; i < LUA_SCRIPT_NUM; ++i)
    {
        m_vpLuaScript[i] = std::unique_ptr<LuaScript>(LuaScript::InitialLuaScript());
        m_vpLuaScript[i]->SetDataProvider(m_dataProvider.get());
        ConnectLuaSignalSlot(m_vpLuaScript[i]);
        m_vLuaMutex[i] = std::make_unique<QMutex>();
        m_vScriptRunners[i] = std::make_unique<ScriptRunnerImpl>(this, i);
    }
}

// 析构函数：确保所有资源正确释放
MainWorkFlow::~MainWorkFlow()
{
	// 等待所有线程池任务完成，避免访问已释放的LuaScript
	if (m_luaThreadPool != nullptr)
	{
		m_luaThreadPool->waitForDone();
	}

	// 显式清理脚本执行器（在LuaScript之前）
	m_vScriptRunners.clear();

	// 显式清理LuaScript实例
	m_vpLuaScript.clear();

	// 释放全局编译检查状态机
	LuaScript::ReleaseCompileLuaState();

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
}

void MainWorkFlow::ConnectLuaSignalSlot(std::unique_ptr<LuaScript> &pLuaScript)
{





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

bool MainWorkFlow::ConfigureComm(const CommConfig& cfg)
{
    if (cfg.type == CommBase::CommType::eSocket)
    {
        auto info = std::make_unique<CommSocket::SocketCommInfo>();
        int socketType = cfg.params.value("socketType", 0).toInt();
        info->m_SocketType = socketType == 0 ? CommSocket::SocketType::eSTServer : CommSocket::SocketType::eSTClient;
        info->m_strSocketIPAddress = cfg.params.value("ip", "0.0.0.0").toString();
        info->m_nSocketPort = static_cast<uint16_t>(cfg.params.value("port", 2000).toUInt());
        info->m_nSocketListenNum = cfg.params.value("listenNum", 10).toInt();
        m_ownedCommInfo = std::move(info);
        return SetCommInfo(m_ownedCommInfo.get());
    }
    // TODO: 支持串口等其他通信方式
    return false;
}

void MainWorkFlow::SetRequestProcessor(std::function<bool(const QByteArray&, QByteArray&)> fn)
{
    if (m_pComm) {
        m_pComm->SetRequestProcessor(std::move(fn));
    }
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

                if (this->m_strRecObjInfo != objectInfo || this->m_strRecData != strData)
                {
                    this->m_strRecObjInfo = objectInfo;
                    this->m_strRecData = strData;

                    emit dataReceived(objectInfo,strData);
                }
                
				
			});

			connect(m_pComm, &CommBase::dataSend, this, [this](QString objectInfo, QByteArray strData) {

                if( this->m_strSendObjInfo != objectInfo || this->m_strSendData != strData)
                {
                    this->m_strSendObjInfo = objectInfo;
                    this->m_strSendData = strData;
                    emit dataSend(objectInfo, strData);
                }
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

IScriptRunner* MainWorkFlow::GetScriptRunner(int nIndex)
{
    if (nIndex < 0 || nIndex >= static_cast<int>(m_vScriptRunners.size())) return nullptr;
    return m_vScriptRunners[nIndex].get();
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
