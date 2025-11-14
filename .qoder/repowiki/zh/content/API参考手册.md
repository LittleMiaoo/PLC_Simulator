# API参考手册

<cite>
**本文档引用的文件**   
- [MainWorkFlow.h](file://CommTest_Qt/MainFlow/MainWorkFlow.h)
- [MainWorkFlow.cpp](file://CommTest_Qt/MainFlow/MainWorkFlow.cpp)
- [CommBase.h](file://CommTest_Qt/Comm/CommBase.h)
- [CommProtocolBase.h](file://CommTest_Qt/Comm/Protocol/CommProtocolBase.h)
- [LuaScript.h](file://CommTest_Qt/LuaScript/LuaScript.h)
- [LuaScript.cpp](file://CommTest_Qt/LuaScript/LuaScript.cpp)
</cite>

## 目录
1. [MainWorkFlow类API](#mainworkflow类api)
2. [CommBase通信信息结构体](#commbase通信信息结构体)
3. [CommProtocolBase协议处理接口](#commprotocolbase协议处理接口)
4. [LuaScript脚本API](#luascript脚本api)
5. [Qt信号与槽函数](#qt信号与槽函数)

## MainWorkFlow类API

`MainWorkFlow`类是系统的核心工作流管理类，采用单例模式设计，负责管理通信、寄存器和Lua脚本等核心功能。

### 单例管理
- **InitialWorkFlow**  
  创建并获取`MainWorkFlow`单例实例。  
  **参数**: `QObject* pParent` - 父对象指针（可选）  
  **返回值**: `MainWorkFlow*` - 单例实例指针  
  **线程安全性**: 通过`QMutex`互斥锁保证线程安全  
  **使用示例**:  
  ```cpp
  MainWorkFlow* pWorkFlow = MainWorkFlow::InitialWorkFlow(this);
  ```

- **ReleaseWorkFlow**  
  释放`MainWorkFlow`单例实例，清理所有资源。  
  **参数**: 无  
  **返回值**: `void`  
  **注意**: 必须在程序退出前调用以确保资源正确释放

### 通信管理
- **SetCommInfo**  
  设置通信信息。  
  **参数**: `CommBase::CommInfoBase* commInfo` - 通信信息实例指针  
  **返回值**: `bool` - 设置成功返回`true`，否则返回`false`  
  **注意**: `commInfo`内存由`MainWorkFlow`管理

- **GetCommInfo**  
  获取当前通信信息。  
  **参数**: 无  
  **返回值**: `CommBase::CommInfoBase*` - 通信信息实例指针，若未设置则返回`nullptr`

- **OpenComm**  
  打开通信连接。  
  **参数**: 无  
  **返回值**: `bool` - 打开成功返回`true`，否则返回`false`  
  **实现**: 根据`CommInfoBase`中的通信类型创建相应的通信实例（如`CommSocket`）

- **CloseComm**  
  关闭通信连接。  
  **参数**: 无  
  **返回值**: `bool` - 关闭成功返回`true`，否则返回`false`

- **IsCommOpen**  
  检查通信是否已打开。  
  **参数**: 无  
  **返回值**: `bool` - 已打开返回`true`，否则返回`false`

### 通信协议管理
- **CreateCommProtocol**  
  创建指定类型的通信协议实例。  
  **参数**: `ProtocolType ProType` - 协议类型枚举值  
  **返回值**: `bool` - 创建成功返回`true`，否则返回`false`  
  **支持的协议类型**:  
  - `eProRegMitsubishiQBinary`: 三菱MC协议二进制通信
  - `eProRegKeyencePCLink`: 基恩士PC-LINK上位链路协议

### 寄存器管理
- **GetRegisterNum**  
  获取寄存器总数。  
  **参数**: 无  
  **返回值**: `long` - 寄存器数量（定义为`REGISTER_VAL_NUM`，值为100000）

- **GetRegisterVal**  
  获取指定地址的寄存器值。  
  **参数**: `int Addr` - 寄存器地址  
  **返回值**: `int16_t` - 寄存器值，若地址越界则返回0  
  **线程安全性**: 使用`std::atomic_int16_t`保证原子操作

- **SetRegisterVal**  
  设置指定地址的寄存器值。  
  **参数**: 
    - `int Addr` - 寄存器地址
    - `const int16_t& nsetVal` - 要设置的值
  **返回值**: `bool` - 设置成功返回`true`，地址越界返回`false`  
  **线程安全性**: 使用`std::atomic_int16_t`保证原子操作

- **ResetAllRegisters**  
  重置所有寄存器值。  
  **参数**: `int16_t nsetVal` - 要设置的初始值  
  **返回值**: `bool` - 总是返回`true`  
  **触发信号**: `RegisterDataUpdate()`，通知GUI更新显示

### Lua脚本管理
- **RunLuaScript**  
  执行指定的Lua脚本文件。  
  **参数**: 
    - `int nLuaIndex` - Lua脚本索引（0-5）
    - `const QString& strLuaFile` - Lua脚本文件路径
  **返回值**: `bool` - 执行成功返回`true`，索引无效或执行失败返回`false`

- **GetLuaScript**  
  获取指定索引的Lua脚本实例。  
  **参数**: `int nIndex` - Lua脚本索引（0-5）  
  **返回值**: `LuaScript*` - Lua脚本实例指针，若索引无效则返回`nullptr`

**Section sources**
- [MainWorkFlow.h](file://CommTest_Qt/MainFlow/MainWorkFlow.h#L30-L112)
- [MainWorkFlow.cpp](file://CommTest_Qt/MainFlow/MainWorkFlow.cpp#L9-L544)

## CommBase通信信息结构体

`CommBase`是所有通信类的基类，定义了通信的基本接口和信息结构。

### 通信类型枚举
```cpp
enum class CommType
{
    eCommUnknown = -1,  // 未知类型
    eSerial,            // 串口
    eSocket,            // 网络
};
```

### 通信状态枚举
```cpp
enum class CommStatus
{
    eCommSucceed = 0,   // 成功
    eCommErr = 1,       // 通信错误
    eCommUnConnect,     // 通信未连接
    eCommStop,          // 通信停止
    eCommTimeOut,       // 通信超时
};
```

### CommInfoBase结构体
`CommInfoBase`是通信信息的基类，包含通信的基本配置信息。

**字段说明**:
- `m_strCommStop`: 通信终止符，用于标识通信数据包的结束
- `m_strCmdStop`: 命令终止符，用于标识命令的结束

**虚函数**:
- `virtual CommType GetCommType() = 0;`  
  获取通信类型，必须在派生类中实现

**示例派生类**: `SocketCommInfo`（位于`CommSocket.h`）扩展了`CommInfoBase`，增加了网络通信特有的配置：
- `m_SocketType`: 网络类型（服务端或客户端）
- `m_strSocketIPAddress`: IP地址
- `m_nSocketPort`: 端口号
- `m_nSocketListenNum`: 监听数

**Section sources**
- [CommBase.h](file://CommTest_Qt/Comm/CommBase.h#L1-L66)

## CommProtocolBase协议处理接口

`CommProtocolBase`是所有通信协议的抽象基类，定义了协议处理的标准接口。

### 协议类型枚举
```cpp
enum class ProtocolType
{
    eProUnknown = -1,                                       // 未知的通信协议
    eProCmdFast = 0,                                        // 无协议
    eProRegMitsubishiQAscii = 10,                           // 三菱MC 3E帧ASCII通信协议
    eProRegMitsubishiQBinary = 11,                          // 三菱MC 3E帧二进制通信协议
    eProRegKeyencePCLink = 20,                              // 基恩士KV系列上位链路协议
    eProRegKeyenceWithMitsubishiQAscii = 21,                // 基恩士使用三菱Q系列PLC的ASCII协议
    eProRegKeyenceWithMitsubishiQBinary = 22,               // 基恩士使用三菱Q系列PLC的二进制协议
};
```

### 指令类型枚举
```cpp
enum class CmdType
{
    eCmdUnkown = -1,    // 未定义
    eCmdWriteReg = 0,   // 写寄存器
    eCmdReadReg = 1,    // 读寄存器
};
```

### 数据类型枚举
```cpp
enum class RegisterDataType
{
    eDataTypeUnkown = -1,
    eDataTypeChar8,
    eDataTypeInt16,
    eDataTypeInt32,
    eDataTypeFloat,
    eDataTypeDouble,
};
```

### 核心接口方法
- **AnalyzeCmdInfo**  
  分析接收到的指令信息，确定指令类型。  
  **参数**: 
    - `QByteArray strInfo` - 接收到的指令数据
    - `CmdType& cCmdType` - 输出参数，用于返回指令类型
  **返回值**: `bool` - 解析成功返回`true`，否则返回`false`

- **AnalyzeReadReg**  
  解析读寄存器指令。  
  **参数**: 
    - `QByteArray strInfo` - 接收到的指令数据
    - `long& nRegAddr` - 输出参数，返回寄存器起始地址
    - `int& nWriteNum` - 输出参数，返回要读取的寄存器数量
  **返回值**: `bool` - 解析成功返回`true`，否则返回`false`

- **PackReportReadRegInfo**  
  打包回复读寄存器指令的信息。  
  **参数**: 
    - `QByteArray& strInfo` - 输出参数，用于返回打包后的回复数据
    - `long nRegAddr` - 寄存器起始地址
    - `int nWriteNum` - 要读取的寄存器数量
    - `const std::vector<int16_t>& vWriteData` - 要返回的寄存器数据
  **返回值**: `bool` - 打包成功返回`true`，否则返回`false`

- **AnalyzeWriteReg**  
  解析写寄存器指令。  
  **参数**: 
    - `QByteArray strInfo` - 接收到的指令数据
    - `long& nRegAddr` - 输出参数，返回寄存器起始地址
    - `int& nWriteNum` - 输出参数，返回要写入的寄存器数量
    - `std::vector<int16_t>& vWriteData` - 输出参数，返回要写入的数据
  **返回值**: `bool` - 解析成功返回`true`，否则返回`false`

- **PackReportWriteRegInfo**  
  打包回复写寄存器指令的信息。  
  **参数**: `QByteArray& strInfo` - 输出参数，用于返回打包后的回复数据  
  **返回值**: `bool` - 打包成功返回`true`，否则返回`false`

- **CmdInfoProcessing**  
  处理通信接收到的数据，纯虚函数，必须在派生类中实现。  
  **参数**: 
    - `const QByteArray& strInfo` - 接收到的数据
    - `ProcessType Curtype` - 处理类型（接收或发送）
    - `QByteArray& strOut` - 输出参数，用于返回处理后的数据
  **返回值**: `bool` - 处理成功返回`true`，否则返回`false`

**继承实现要求**:
1. 必须重写`CmdInfoProcessing`虚函数，实现具体的协议处理逻辑
2. 必须重写所有`Analyze*`和`Pack*`系列的纯虚函数
3. 可以使用`m_mCmdInfoType`成员变量（`std::map<QByteArray, CmdType>`）来映射协议命令字符串与指令类型

**Section sources**
- [CommProtocolBase.h](file://CommTest_Qt/Comm/Protocol/CommProtocolBase.h#L1-L107)

## LuaScript脚本API

`LuaScript`类封装了Lua脚本引擎，提供了C++与Lua脚本交互的接口。

### 脚本生命周期管理
- **InitialLuaScript**  
  创建并初始化Lua脚本实例。  
  **参数**: `QObject* pParent` - 父对象指针（可选）  
  **返回值**: `LuaScript*` - Lua脚本实例指针

- **RunLuaScript**  
  执行指定路径的Lua脚本文件。  
  **参数**: 
    - `const QString& strLuaFile` - Lua脚本文件路径
    - `QString& errorMsg` - 输出参数，用于返回错误信息
  **返回值**: `bool` - 执行成功返回`true`，否则返回`false`

- **RunLuaScriptWithEditor**  
  执行指定内容的Lua脚本（通常用于编辑器实时执行）。  
  **参数**: 
    - `const QString& strLuaContent` - Lua脚本内容
    - `QString& errorMsg` - 输出参数，用于返回错误信息
  **返回值**: `bool` - 执行成功返回`true`，否则返回`false`

- **CheckLuaScript**  
  检查Lua脚本语法是否正确。  
  **参数**: 
    - `const QString& strLuaFile` - Lua脚本内容
    - `QString& strErrorInfo` - 输出参数，用于返回错误信息
  **返回值**: `bool` - 语法正确返回`true`，否则返回`false`

### 变量交互API
Lua脚本通过注册的全局函数与C++交互，这些函数在`RegisterLuaFunc`中注册。

#### 写入寄存器数据（Set系列）
- **SetInt16("D100", 123)**  
  将16位整数写入指定地址的寄存器。
- **SetInt32("D100", 123)**  
  将32位整数写入指定地址的寄存器（占用两个16位寄存器）。
- **SetFloat("D100", 123.45)**  
  将浮点数写入指定地址的寄存器（占用两个16位寄存器）。
- **SetDouble("D100", 123.45)**  
  将双精度浮点数写入指定地址的寄存器（占用四个16位寄存器）。
- **SetString("D100", "AB")**  
  将字符串写入指定地址的寄存器（最多2个字符，存储为16位整数）。

#### 读取寄存器数据（Get系列）
- **GetInt16("D100")**  
  从指定地址的寄存器读取16位整数。
- **GetInt32("D100")**  
  从指定地址开始的寄存器读取32位整数。
- **GetFloat("D100")**  
  从指定地址开始的寄存器读取浮点数。
- **GetDouble("D100")**  
  从指定地址开始的寄存器读取双精度浮点数。
- **GetString("D100")**  
  从指定地址的寄存器读取字符串（最多2个字符）。

#### 平台控制函数
- **MoveAbsInt32("D100", "D102", "D104")**  
  绝对移动，参数为X、Y、角度的寄存器地址。
- **MoveAbsFloat("D100", "D102", "D104")**  
  绝对移动（浮点数），参数为X、Y、角度的寄存器地址。
- **MoveRelativeInt32("D100", "D102", "D104")**  
  相对移动，参数为X、Y、角度的寄存器地址。
- **MoveRelativeFloat("D100", "D102", "D104")**  
  相对移动（浮点数），参数为X、Y、角度的寄存器地址。

#### 循环状态检查
- **IsLoopValid()**  
  检查循环是否有效，用于控制脚本中的循环执行。

**地址格式**: 所有寄存器地址必须以`D`开头，后跟数字（如`D100`），范围为0-100000。

**Section sources**
- [LuaScript.h](file://CommTest_Qt/LuaScript/LuaScript.h#L7-L140)
- [LuaScript.cpp](file://CommTest_Qt/LuaScript/LuaScript.cpp#L1-L764)

## Qt信号与槽函数

### MainWorkFlow信号
- **commLogRecord(QString strLogInfo)**  
  **触发条件**: 当有通信日志需要记录时  
  **参数**: `strLogInfo` - 日志信息字符串  
  **用途**: 用于将通信日志转发到GUI界面显示

- **dataReceived(QString objectInfo, QByteArray recData)**  
  **触发条件**: 当接收到通信数据时  
  **参数**: 
    - `objectInfo` - 对象信息（如"接收到: "）
    - `recData` - 接收到的原始数据
  **用途**: 通知上层应用有新数据到达，并触发`WorkProcess`进行处理

- **dataSend(QString objectInfo, QByteArray recData)**  
  **触发条件**: 当发送通信数据时  
  **参数**: 
    - `objectInfo` - 对象信息（如"发送: "）
    - `recData` - 发送的原始数据
  **用途**: 用于记录发送的数据日志

- **RegisterDataUpdate()**  
  **触发条件**: 当寄存器数据发生改变时（通过`SetRegisterVal`或Lua脚本）  
  **参数**: 无  
  **用途**: 通知GUI界面更新寄存器表格显示

### LuaScript信号
- **SetRegisterValInt16(int nIndex, int16_t nValue)**  
  **触发条件**: 当Lua脚本调用`SetInt16`函数时  
  **参数**: 
    - `nIndex` - 寄存器地址
    - `nValue` - 要设置的16位整数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::SetRegisterVal`

- **SetRegisterValInt32(int nIndex, int32_t nValue)**  
  **触发条件**: 当Lua脚本调用`SetInt32`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `nValue` - 要设置的32位整数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::SetRegisterVal`，自动拆分为两个16位寄存器

- **SetRegisterValFloat(int nIndex, float fValue)**  
  **触发条件**: 当Lua脚本调用`SetFloat`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `fValue` - 要设置的浮点数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::SetRegisterVal`，自动拆分为两个16位寄存器

- **SetRegisterValDouble(int nIndex, double dValue)**  
  **触发条件**: 当Lua脚本调用`SetDouble`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `dValue` - 要设置的双精度浮点数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::SetRegisterVal`，自动拆分为四个16位寄存器

- **SetRegisterValString(int nIndex, QString strValue)**  
  **触发条件**: 当Lua脚本调用`SetString`函数时  
  **参数**: 
    - `nIndex` - 寄存器地址
    - `strValue` - 要设置的字符串（最多2个字符）
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::SetRegisterVal`，自动转换为16位整数

- **GetRegisterValInt16(int nIndex, int16_t& nValue)**  
  **触发条件**: 当Lua脚本调用`GetInt16`函数时  
  **参数**: 
    - `nIndex` - 寄存器地址
    - `nValue` - 输出参数，用于返回读取到的16位整数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::GetRegisterVal`

- **GetRegisterValInt32(int nIndex, int32_t& nValue)**  
  **触发条件**: 当Lua脚本调用`GetInt32`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `nValue` - 输出参数，用于返回读取到的32位整数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::GetRegisterVal`，自动组合两个16位寄存器

- **GetRegisterValFloat(int nIndex, float& fValue)**  
  **触发条件**: 当Lua脚本调用`GetFloat`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `fValue` - 输出参数，用于返回读取到的浮点数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::GetRegisterVal`，自动组合两个16位寄存器

- **GetRegisterValDouble(int nIndex, double& dValue)**  
  **触发条件**: 当Lua脚本调用`GetDouble`函数时  
  **参数**: 
    - `nIndex` - 寄存器起始地址
    - `dValue` - 输出参数，用于返回读取到的双精度浮点数值
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::GetRegisterVal`，自动组合四个16位寄存器

- **GetRegisterValString(int nIndex, QString& strValue)**  
  **触发条件**: 当Lua脚本调用`GetString`函数时  
  **参数**: 
    - `nIndex` - 寄存器地址
    - `strValue` - 输出参数，用于返回读取到的字符串
  **连接**: 在`MainWorkFlow::ConnectLuaSignalSlot`中连接到`MainWorkFlow::GetRegisterVal`，自动从16位整数转换

- **MovePlatformAbsInt32(int32_t nX, int32_t nY, int32_t nAngle)**  
  **触发条件**: 当Lua脚本调用`MoveAbsInt32`函数时  
  **参数**: `nX`, `nY`, `nAngle` - 绝对移动的目标坐标和角度  
  **连接**: 在`CommTest_Qt::InitialLuaScript`中连接到`CommTest_Qt::OnMovePlatformAbsInt32`槽函数

- **MovePlatformAbsFloat(double dX, double dY, double dAngle)**  
  **触发条件**: 当Lua脚本调用`MoveAbsFloat`函数时  
  **参数**: `dX`, `dY`, `dAngle` - 绝对移动的目标坐标和角度（浮点数）  
  **连接**: 在`CommTest_Qt::InitialLuaScript`中连接到`CommTest_Qt::OnMovePlatformAbsFloat`槽函数

- **MovePlatformRelativeInt32(int32_t nX, int32_t nY, int32_t nAngle)**  
  **触发条件**: 当Lua脚本调用`MoveRelativeInt32`函数时  
  **参数**: `nX`, `nY`, `nAngle` - 相对移动的偏移量和角度  
  **连接**: 在`CommTest_Qt::InitialLuaScript`中连接到`CommTest_Qt::OnMovePlatformRelativeInt32`槽函数

- **MovePlatformRelativeFloat(double dX, double dY, double dAngle)**  
  **触发条件**: 当Lua脚本调用`MoveRelativeFloat`函数时  
  **参数**: `dX`, `dY`, `dAngle` - 相对移动的偏移量和角度（浮点数）  
  **连接**: 在`CommTest_Qt::InitialLuaScript`中连接到`CommTest_Qt::OnMovePlatformRelativeFloat`槽函数

**Section sources**
- [MainWorkFlow.h](file://CommTest_Qt/MainFlow/MainWorkFlow.h#L104-L112)
- [LuaScript.h](file://CommTest_Qt/LuaScript/LuaScript.h#L120-L138)
- [MainWorkFlow.cpp](file://CommTest_Qt/MainFlow/MainWorkFlow.cpp#L58-L151)
- [CommTest_Qt.cpp](file://CommTest_Qt/Gui/CommTest_Qt.cpp#L414-L593)