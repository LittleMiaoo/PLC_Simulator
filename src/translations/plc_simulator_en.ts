<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en">
<context>
    <name>AppInfo</name>
    <message>
        <source>PLC通信模拟器</source>
        <translation>PLC Communication Simulator</translation>
    </message>
    <message>
        <source>一个基于Qt6的PLC模拟通信测试软件.
支持基恩士上位链路协议&amp;三菱Q系列MC协议二进制通信.
支持通过Lua脚本语言实现自动化.
本软件采用 MIT 许可证发布,详情查看 LICENSE 文件.</source>
        <translation>A Qt6-based PLC communication simulation and testing tool.
Supports Keyence host link protocol and Mitsubishi Q-series MC binary protocol.
Supports automation via Lua scripting.
Released under the MIT License. See LICENSE for details.</translation>
    </message>
</context>
<context>
    <name>AuxDialogs</name>
    <message>
        <source>关于 %1</source>
        <translation>About %1</translation>
    </message>
    <message>
        <source>第三方许可</source>
        <translation>Third-Party Licenses</translation>
    </message>
    <message>
        <source>无法读取第三方许可证文件。

本软件使用了以下第三方库：
1. Qt Framework (LGPL v3)
2. Lua 5.4 (MIT License)

详细信息请查看 THIRD_PARTY_LICENSES.txt 文件。</source>
        <translation>Unable to read the third-party license file.

This software uses the following third-party libraries:
1. Qt Framework (LGPL v3)
2. Lua 5.4 (MIT License)

See THIRD_PARTY_LICENSES.txt for details.</translation>
    </message>
    <message>
        <source>第三方许可证</source>
        <translation>Third-Party Licenses</translation>
    </message>
    <message>
        <source>更新日志</source>
        <translation>Changelog</translation>
    </message>
    <message>
        <source>平台参数设置</source>
        <translation>Platform Parameters</translation>
    </message>
    <message>
        <source>XY单位幂:</source>
        <translation>XY Unit Power:</translation>
    </message>
    <message>
        <source>D单位幂:</source>
        <translation>D Unit Power:</translation>
    </message>
    <message>
        <source>对象平台轴位置地址:</source>
        <translation>Object Platform Axis Address:</translation>
    </message>
    <message>
        <source>目标平台轴位置地址:</source>
        <translation>Target Platform Axis Address:</translation>
    </message>
    <message>
        <source>确定</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <source>关闭</source>
        <translation>Close</translation>
    </message>
</context>
<context>
    <name>CollapsibleGroupBox</name>
    <message>
        <source>关闭</source>
        <translation>Close</translation>
    </message>
</context>
<context>
    <name>CommBase</name>
    <message>
        <source>队列溢出，丢弃请求: %1</source>
        <translation>Queue overflow, dropping request: %1</translation>
    </message>
    <message>
        <source>请求超时，跳过: %1</source>
        <translation>Request timed out, skipping: %1</translation>
    </message>
</context>
<context>
    <name>CommSocket</name>
    <message>
        <source>[%1:%2] 服务器创建成功,开始监听客户端链接...</source>
        <translation>[%1:%2] Server created, listening for clients...</translation>
    </message>
    <message>
        <source>[%1:%2] 新客户端链接</source>
        <translation>[%1:%2] New client connected</translation>
    </message>
    <message>
        <source>[%1:%2] 客户端错误:%3</source>
        <translation>[%1:%2] Client error: %3</translation>
    </message>
    <message>
        <source>[%1]:客户端断开链接</source>
        <translation>[%1]: Client disconnected</translation>
    </message>
    <message>
        <source>服务器错误:%1</source>
        <translation>Server error: %1</translation>
    </message>
    <message>
        <source>即将断开所有客户端链接,当前链接客户端数量[%1]</source>
        <translation>Disconnecting all clients, current count [%1]</translation>
    </message>
    <message>
        <source>[%1:%2] 客户端即将断开</source>
        <translation>[%1:%2] Client disconnecting</translation>
    </message>
    <message>
        <source>[%1:%2] 即将关闭服务器...</source>
        <translation>[%1:%2] Shutting down server...</translation>
    </message>
    <message>
        <source>[%1:%2] 即将关闭网络连接...</source>
        <translation>[%1:%2] Closing network connection...</translation>
    </message>
</context>
<context>
    <name>ImagePage</name>
    <message>
        <source>图像文件 (*.bmp *.png *.jpg *.jpeg *.tiff *.tif)</source>
        <translation>Image Files (*.bmp *.png *.jpg *.jpeg *.tiff *.tif)</translation>
    </message>
    <message>
        <source>选择图像</source>
        <translation>Select Image</translation>
    </message>
    <message>
        <source>错误</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>无法加载所选图像文件。</source>
        <translation>Unable to load the selected image file.</translation>
    </message>
    <message>
        <source>警告</source>
        <translation>Warning</translation>
    </message>
    <message>
        <source>图像加载成功，但无法保存到配置目录。</source>
        <translation>Image loaded, but could not be saved to the config directory.</translation>
    </message>
</context>
<context>
    <name>LuaEngine</name>
    <message>
        <source>无法打开脚本文件: %1</source>
        <translation>Unable to open script file: %1</translation>
    </message>
    <message>
        <source>获取循环是否有效</source>
        <translation>Check whether loop execution is enabled</translation>
    </message>
    <message>
        <source>睡眠,毫秒</source>
        <translation>Sleep (milliseconds)</translation>
    </message>
</context>
<context>
    <name>LuaStaticCheck</name>
    <message>
        <source>无法创建 Lua 状态机</source>
        <translation>Unable to create Lua state</translation>
    </message>
    <message>
        <source>编译产物非函数，无法静态检查</source>
        <translation>Compiled result is not a function; static check aborted</translation>
    </message>
    <message>
        <source>未定义的函数/全局: %1</source>
        <translation>Undefined function/global: %1</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <source> - 子窗口</source>
        <translation> - Mini Window</translation>
    </message>
    <message>
        <source> - 模拟平台</source>
        <translation> - Simulation Platform</translation>
    </message>
    <message>
        <source>基恩士PC-LINK上位链路协议</source>
        <translation>Keyence PC-LINK Host Link Protocol</translation>
    </message>
    <message>
        <source>三菱MC协议二进制通信</source>
        <translation>Mitsubishi MC Binary Protocol</translation>
    </message>
    <message>
        <source>字符</source>
        <translation>Character</translation>
    </message>
    <message>
        <source>单字</source>
        <translation>Word</translation>
    </message>
    <message>
        <source>双字</source>
        <translation>Double Word</translation>
    </message>
    <message>
        <source>单精度</source>
        <translation>Float</translation>
    </message>
    <message>
        <source>双精度</source>
        <translation>Double</translation>
    </message>
    <message>
        <source>帮助(&amp;H)</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <source>关于(&amp;A)</source>
        <translation>&amp;About</translation>
    </message>
    <message>
        <source>更新日志(&amp;U)</source>
        <translation>&amp;Changelog</translation>
    </message>
    <message>
        <source>视图(&amp;V)</source>
        <translation>&amp;View</translation>
    </message>
    <message>
        <source>主题</source>
        <translation>Theme</translation>
    </message>
    <message>
        <source>语言</source>
        <translation>Language</translation>
    </message>
    <message>
        <source>浅色</source>
        <translation>Light</translation>
    </message>
    <message>
        <source>深色</source>
        <translation>Dark</translation>
    </message>
    <message>
        <source>平台(&amp;P)</source>
        <translation>&amp;Platform</translation>
    </message>
    <message>
        <source>显示平台</source>
        <translation>Show Platform</translation>
    </message>
    <message>
        <source>自动写入轴位置</source>
        <translation>Auto-Write Axis Position</translation>
    </message>
    <message>
        <source>写入格式</source>
        <translation>Write Format</translation>
    </message>
    <message>
        <source>浮点写入</source>
        <translation>Write as Float</translation>
    </message>
    <message>
        <source>双字写入</source>
        <translation>Write as Double Word</translation>
    </message>
    <message>
        <source>参数设置…</source>
        <translation>Parameters…</translation>
    </message>
    <message>
        <source>平台操作</source>
        <translation>Platform Actions</translation>
    </message>
    <message>
        <source>浮点</source>
        <translation>Float</translation>
    </message>
    <message>
        <source>打开连接失败!</source>
        <translation>Failed to open connection!</translation>
    </message>
    <message>
        <source>关闭连接失败!</source>
        <translation>Failed to close connection!</translation>
    </message>
    <message>
        <source>打开链接</source>
        <translation>Open Connection</translation>
    </message>
    <message>
        <source>关闭链接</source>
        <translation>Close Connection</translation>
    </message>
    <message>
        <source>错误: 寄存器地址超出范围</source>
        <translation>Error: register address out of range</translation>
    </message>
    <message>
        <source>轴位置写入成功 (地址:%1)</source>
        <translation>Axis position written (address: %1)</translation>
    </message>
    <message>
        <source>服务器设置</source>
        <translation>Server Settings</translation>
    </message>
    <message>
        <source>端口:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <source>通信协议:</source>
        <translation>Protocol:</translation>
    </message>
    <message>
        <source>命令终止符:</source>
        <translation>Command Terminator:</translation>
    </message>
    <message>
        <source>通信终止符:</source>
        <translation>Comm Terminator:</translation>
    </message>
    <message>
        <source>寄存器设置</source>
        <translation>Register Settings</translation>
    </message>
    <message>
        <source>显示地址:</source>
        <translation>Display Address:</translation>
    </message>
    <message>
        <source>数据类型:</source>
        <translation>Data Type:</translation>
    </message>
    <message>
        <source>数据显示格式:</source>
        <translation>Data Display Format:</translation>
    </message>
    <message>
        <source>十进制显示</source>
        <translation>Decimal Display</translation>
    </message>
    <message>
        <source>十六进制显示</source>
        <translation>Hex Display</translation>
    </message>
    <message>
        <source>日志显示格式:</source>
        <translation>Log Display Format:</translation>
    </message>
    <message>
        <source>ASCII码显示</source>
        <translation>ASCII Display</translation>
    </message>
    <message>
        <source>清空寄存器</source>
        <translation>Clear Registers</translation>
    </message>
    <message>
        <source>清空日志</source>
        <translation>Clear Log</translation>
    </message>
    <message>
        <source>小窗显示</source>
        <translation>Mini Window</translation>
    </message>
    <message>
        <source>通信日志</source>
        <translation>Communication Log</translation>
    </message>
    <message>
        <source>脚本</source>
        <translation>Script</translation>
    </message>
    <message>
        <source>循环执行</source>
        <translation>Loop</translation>
    </message>
    <message>
        <source>编辑</source>
        <translation>Edit</translation>
    </message>
    <message>
        <source>执行</source>
        <translation>Run</translation>
    </message>
    <message>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <source>脚本%1</source>
        <translation>Script %1</translation>
    </message>
</context>
<context>
    <name>PlatformBinding</name>
    <message>
        <source>平台绝对移动,双字</source>
        <translation>Absolute platform move (double word)</translation>
    </message>
    <message>
        <source>平台绝对移动,浮点数</source>
        <translation>Absolute platform move (float)</translation>
    </message>
    <message>
        <source>平台相对移动,双字</source>
        <translation>Relative platform move (double word)</translation>
    </message>
    <message>
        <source>平台相对移动,浮点数</source>
        <translation>Relative platform move (float)</translation>
    </message>
    <message>
        <source>写入当前位置,双字</source>
        <translation>Write current position (double word)</translation>
    </message>
    <message>
        <source>写入当前位置,浮点数</source>
        <translation>Write current position (float)</translation>
    </message>
</context>
<context>
    <name>PlatformControlPanel</name>
    <message>
        <source>显示</source>
        <translation>Show</translation>
    </message>
    <message>
        <source>跟随平台</source>
        <translation>Follow Platform</translation>
    </message>
    <message>
        <source>基准平台 (mm)</source>
        <translation>Base Platform (mm)</translation>
    </message>
    <message>
        <source>实时平台 (mm)</source>
        <translation>Live Platform (mm)</translation>
    </message>
    <message>
        <source>基准Mark (mm)</source>
        <translation>Base Mark (mm)</translation>
    </message>
    <message>
        <source>实时Mark (mm)</source>
        <translation>Live Mark (mm)</translation>
    </message>
    <message>
        <source>虚拟Mark (mm)</source>
        <translation>Virtual Mark (mm)</translation>
    </message>
    <message>
        <source>X偏移:</source>
        <translation>X Offset:</translation>
    </message>
    <message>
        <source>Y偏移:</source>
        <translation>Y Offset:</translation>
    </message>
</context>
<context>
    <name>PlatformParamsDialog</name>
    <message>
        <source>参数设置</source>
        <translation>Parameters</translation>
    </message>
    <message>
        <source>产品尺寸 (mm):</source>
        <translation>Product Size (mm):</translation>
    </message>
    <message>
        <source>缩放比 (px/mm):</source>
        <translation>Scale (px/mm):</translation>
    </message>
</context>
<context>
    <name>QuickPanel</name>
    <message>
        <source>退出小窗</source>
        <translation>Exit Mini Window</translation>
    </message>
</context>
<context>
    <name>RegisterBinding</name>
    <message>
        <source>设置寄存器值,单字</source>
        <translation>Set register value (word)</translation>
    </message>
    <message>
        <source>设置寄存器值,双字</source>
        <translation>Set register value (double word)</translation>
    </message>
    <message>
        <source>设置寄存器值,浮点数</source>
        <translation>Set register value (float)</translation>
    </message>
    <message>
        <source>设置寄存器值,双精度浮点数</source>
        <translation>Set register value (double)</translation>
    </message>
    <message>
        <source>设置寄存器字符串</source>
        <translation>Set register string</translation>
    </message>
    <message>
        <source>读取寄存器值,单字</source>
        <translation>Get register value (word)</translation>
    </message>
    <message>
        <source>读取寄存器值,双字</source>
        <translation>Get register value (double word)</translation>
    </message>
    <message>
        <source>读取寄存器值,浮点数</source>
        <translation>Get register value (float)</translation>
    </message>
    <message>
        <source>读取寄存器值,双精度浮点数</source>
        <translation>Get register value (double)</translation>
    </message>
    <message>
        <source>读取寄存器字符串</source>
        <translation>Get register string</translation>
    </message>
</context>
<context>
    <name>RegisterTableModel</name>
    <message>
        <source>输入非法</source>
        <translation>Invalid Input</translation>
    </message>
    <message>
        <source>输入为空,已保留原值!</source>
        <translation>Input is empty; original value kept.</translation>
    </message>
    <message>
        <source>输入截断</source>
        <translation>Input Truncated</translation>
    </message>
    <message>
        <source>输入值 %1 长度超过2,只保留前2位!</source>
        <translation>Input %1 exceeds 2 characters; only the first 2 are kept.</translation>
    </message>
    <message>
        <source>输入值 %1 非十六进制数</source>
        <translation>Input %1 is not a hexadecimal number</translation>
    </message>
    <message>
        <source>输入值 %1 超过范围,将截断输入数据!</source>
        <translation>Input %1 exceeds range; data will be truncated.</translation>
    </message>
    <message>
        <source>输入值 %1 非整型数</source>
        <translation>Input %1 is not an integer</translation>
    </message>
    <message>
        <source>输入超范围</source>
        <translation>Out of Range</translation>
    </message>
    <message>
        <source>输入值 %1 超过范围(%2 ~ %3)</source>
        <translation>Input %1 is out of range (%2 ~ %3)</translation>
    </message>
    <message>
        <source>输入值 %1 非浮点数</source>
        <translation>Input %1 is not a floating-point number</translation>
    </message>
    <message>
        <source>输入值 %1 超过浮点数范围</source>
        <translation>Input %1 exceeds floating-point range</translation>
    </message>
    <message>
        <source>值</source>
        <translation>Value</translation>
    </message>
    <message>
        <source>地址</source>
        <translation>Address</translation>
    </message>
</context>
<context>
    <name>ScriptEditor</name>
    <message>
        <source>Lua 脚本编辑器</source>
        <translation>Lua Script Editor</translation>
    </message>
    <message>
        <source>文件(&amp;F)</source>
        <translation>&amp;File</translation>
    </message>
    <message>
        <source>保存(&amp;S)</source>
        <translation>&amp;Save</translation>
    </message>
    <message>
        <source>另存为(&amp;A)...</source>
        <translation>Save &amp;As...</translation>
    </message>
    <message>
        <source>从文件加载(&amp;L)...</source>
        <translation>&amp;Load From...</translation>
    </message>
    <message>
        <source>退出(&amp;X)</source>
        <translation>E&amp;xit</translation>
    </message>
    <message>
        <source>编辑(&amp;E)</source>
        <translation>&amp;Edit</translation>
    </message>
    <message>
        <source>插入函数(&amp;I)</source>
        <translation>&amp;Insert Function</translation>
    </message>
    <message>
        <source>脚本(&amp;S)</source>
        <translation>&amp;Script</translation>
    </message>
    <message>
        <source>编译脚本(&amp;C)</source>
        <translation>&amp;Compile Script</translation>
    </message>
    <message>
        <source>执行脚本(&amp;E)</source>
        <translation>&amp;Execute Script</translation>
    </message>
    <message>
        <source>保存脚本</source>
        <translation>Save Script</translation>
    </message>
    <message>
        <source>Lua 脚本 (*.lua);;所有文件 (*)</source>
        <translation>Lua Scripts (*.lua);;All Files (*)</translation>
    </message>
    <message>
        <source>错误</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>编译</source>
        <translation>Compile</translation>
    </message>
    <message>
        <source>语法检查器不可用。</source>
        <translation>Syntax checker not available.</translation>
    </message>
    <message>
        <source>脚本编译成功。</source>
        <translation>Script compiled successfully.</translation>
    </message>
    <message>
        <source>编译错误</source>
        <translation>Compile Error</translation>
    </message>
    <message>
        <source>未配置脚本运行器。</source>
        <translation>Script runner not configured.</translation>
    </message>
    <message>
        <source>执行</source>
        <translation>Execute</translation>
    </message>
    <message>
        <source>脚本执行成功。</source>
        <translation>Script executed successfully.</translation>
    </message>
    <message>
        <source>执行错误</source>
        <translation>Execution Error</translation>
    </message>
    <message>
        <source>脚本运行中</source>
        <translation>Script Running</translation>
    </message>
    <message>
        <source>Lua 运行中，用时: 0 秒......</source>
        <translation>Lua is Running, Use-Time: 0 s......</translation>
    </message>
    <message>
        <source>Lua 运行中，用时: %1 秒......</source>
        <translation>Lua Running, UseTime: %1 s......</translation>
    </message>
    <message>
        <source>另存为脚本</source>
        <translation>Save Script As</translation>
    </message>
    <message>
        <source>Lua 脚本 (*.lua)</source>
        <translation>Lua Scripts (*.lua)</translation>
    </message>
    <message>
        <source>成功</source>
        <translation>Success</translation>
    </message>
    <message>
        <source>脚本保存成功。</source>
        <translation>Script saved successfully.</translation>
    </message>
    <message>
        <source>从文件加载脚本</source>
        <translation>Load Script From</translation>
    </message>
    <message>
        <source>文件未保存</source>
        <translation>Unsaved File</translation>
    </message>
    <message>
        <source>当前脚本已修改。是否保存到本地?</source>
        <translation>The current script has been modified. Save to disk?</translation>
    </message>
    <message>
        <source>保存</source>
        <translation>Save</translation>
    </message>
    <message>
        <source>不保存</source>
        <translation>Discard</translation>
    </message>
    <message>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <source>if 条件分支</source>
        <translation>if conditional branch</translation>
    </message>
    <message>
        <source>while 循环</source>
        <translation>while loop</translation>
    </message>
    <message>
        <source>for 循环</source>
        <translation>for loop</translation>
    </message>
    <message>
        <source>if-elseif-else 多分支</source>
        <translation>if-elseif-else multi-branch</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Ctrl+Alt+S</source>
        <translation>Ctrl+Alt+S</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <source>F7</source>
        <translation>F7</translation>
    </message>
    <message>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <source>无法写入文件 %1:
%2。</source>
        <translation>Cannot write file %1:
%2.</translation>
    </message>
    <message>
        <source>%1[*] - %2</source>
        <translation>%1[*] - %2</translation>
    </message>
    <message>
        <source>无法读取文件 %1:
%2。</source>
        <translation>Cannot read file %1:
%2.</translation>
    </message>
</context>
<context>
    <name>ScriptEngineHost</name>
    <message>
        <source>Lua执行失败:%1</source>
        <translation>Lua execution failed: %1</translation>
    </message>
</context>
<context>
    <name>ScriptManager</name>
    <message>
        <source>脚本不存在: %1</source>
        <translation>Script not found: %1</translation>
    </message>
    <message>
        <source>Lua执行错误</source>
        <translation>Lua Execution Error</translation>
    </message>
    <message>
        <source>切换脚本</source>
        <translation>Switch Script</translation>
    </message>
    <message>
        <source>当前脚本有未保存的更改,切换前是否保存?</source>
        <translation>The current script has unsaved changes. Save before switching?</translation>
    </message>
    <message>
        <source>保存</source>
        <translation>Save</translation>
    </message>
    <message>
        <source>不保存</source>
        <translation>Discard</translation>
    </message>
    <message>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
</context>
<context>
    <name>SimulationPlatform</name>
    <message>
        <source>模拟平台</source>
        <translation>Simulation Platform</translation>
    </message>
    <message>
        <source>收起 »</source>
        <translation>Collapse »</translation>
    </message>
    <message>
        <source>« 展开</source>
        <translation>« Expand</translation>
    </message>
    <message>
        <source>缩放: %1%</source>
        <translation>Zoom: %1%</translation>
    </message>
    <message>
        <source>未加载图片</source>
        <translation>No image loaded</translation>
    </message>
    <message>
        <source>视图</source>
        <translation>View</translation>
    </message>
    <message>
        <source>图片显示</source>
        <translation>Image Viewer</translation>
    </message>
    <message>
        <source>窗口位置</source>
        <translation>Window Position</translation>
    </message>
    <message>
        <source>左上</source>
        <translation>Top Left</translation>
    </message>
    <message>
        <source>右上</source>
        <translation>Top Right</translation>
    </message>
    <message>
        <source>左下</source>
        <translation>Bottom Left</translation>
    </message>
    <message>
        <source>右下</source>
        <translation>Bottom Right</translation>
    </message>
    <message>
        <source>居中</source>
        <translation>Center</translation>
    </message>
    <message>
        <source>参数设置…</source>
        <translation>Parameters…</translation>
    </message>
    <message>
        <source>基准平台</source>
        <translation>Base Platform</translation>
    </message>
    <message>
        <source>实时平台</source>
        <translation>Live Platform</translation>
    </message>
    <message>
        <source>基准Mark</source>
        <translation>Base Mark</translation>
    </message>
    <message>
        <source>实时Mark</source>
        <translation>Live Mark</translation>
    </message>
    <message>
        <source>虚拟Mark</source>
        <translation>Virtual Mark</translation>
    </message>
    <message>
        <source>图像</source>
        <translation>Image</translation>
    </message>
    <message>
        <source>加载图片…</source>
        <translation>Load Image…</translation>
    </message>
    <message>
        <source>产品尺寸: %1 mm    缩放比: %2 px/mm</source>
        <translation>Product size: %1 mm    Scale: %2 px/mm</translation>
    </message>
</context>
<context>
    <name>StatusBarController</name>
    <message>
        <source>服务器</source>
        <translation>Server</translation>
    </message>
    <message>
        <source>客户端</source>
        <translation>Client</translation>
    </message>
    <message>
        <source>未连接</source>
        <translation>Disconnected</translation>
    </message>
    <message>
        <source>客户端: &lt;span style=&apos;color:%1&apos;&gt;—&lt;/span&gt;</source>
        <translation>Clients: &lt;span style=&apos;color:%1&apos;&gt;—&lt;/span&gt;</translation>
    </message>
    <message>
        <source>客户端: %1</source>
        <translation>Clients: %1</translation>
    </message>
    <message>
        <source>收</source>
        <translation>Rx</translation>
    </message>
    <message>
        <source>发</source>
        <translation>Tx</translation>
    </message>
    <message>
        <source>脚本: &lt;span style=&apos;color:%1&apos;&gt;空闲&lt;/span&gt;</source>
        <translation>Scripts: &lt;span style=&apos;color:%1&apos;&gt;Idle&lt;/span&gt;</translation>
    </message>
    <message>
        <source>脚本: %1 运行中</source>
        <translation>Scripts: %1 running</translation>
    </message>
    <message>
        <source>平台 X:%1 Y:%2 θ:%3</source>
        <translation>Platform X:%1 Y:%2 θ:%3</translation>
    </message>
    <message>
        <source>服务器(监听中)</source>
        <translation>Server (listening)</translation>
    </message>
    <message>
        <source>协议</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <source>角色</source>
        <translation>Role</translation>
    </message>
    <message>
        <source>地址</source>
        <translation>Address</translation>
    </message>
    <message>
        <source>端口</source>
        <translation>Port</translation>
    </message>
    <message>
        <source>状态</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>未监听</source>
        <translation>Not listening</translation>
    </message>
    <message>
        <source>已连接客户端 (%1)</source>
        <translation>Connected Clients (%1)</translation>
    </message>
    <message>
        <source>收 / 发 帧</source>
        <translation>Rx / Tx Frames</translation>
    </message>
    <message>
        <source>收 / 发 字节</source>
        <translation>Rx / Tx Bytes</translation>
    </message>
    <message>
        <source>超时</source>
        <translation>Timeouts</translation>
    </message>
    <message>
        <source>最近</source>
        <translation>Last Activity</translation>
    </message>
    <message>
        <source>脚本 #%1</source>
        <translation>Script #%1</translation>
    </message>
    <message>
        <source>运行中脚本 (%1)</source>
        <translation>Running Scripts (%1)</translation>
    </message>
    <message>
        <source>Live 位姿</source>
        <translation>Live Pose</translation>
    </message>
    <message>
        <source>Base 位姿</source>
        <translation>Base Pose</translation>
    </message>
    <message>
        <source>单位幂 XY / D</source>
        <translation>Unit Power XY / D</translation>
    </message>
    <message>
        <source>对象轴写入</source>
        <translation>Object Axis Write</translation>
    </message>
    <message>
        <source>目标轴写入</source>
        <translation>Target Axis Write</translation>
    </message>
</context>
<context>
    <name>main</name>
    <message>
        <source>错误</source>
        <translation>Error</translation>
    </message>
    <message>
        <source>创建共享内存失败！</source>
        <translation>Failed to create shared memory!</translation>
    </message>
    <message>
        <source>提示</source>
        <translation>Notice</translation>
    </message>
    <message>
        <source>程序正在运行！请勿重复启动！</source>
        <translation>The application is already running. Do not start it again.</translation>
    </message>
</context>
</TS>
