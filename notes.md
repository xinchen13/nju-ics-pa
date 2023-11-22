# PA0
### 安装Linux与相关工具
### 获取框架代码: [NJU ICS PA](https://github.com/NJU-ProjectN/ics-pa.git): `2022`作为默认分支
### 配置git, 并修改`/Makefile`中的ID与name, 以及开发追踪`git commit`的author 
### 尝试配置、编译并运行nemu

# PA1
### 初始化 `am-kernels`
### 在配置文件中选择ISA为riscv32
### 修复
### 修复运行NEMU之后直接键入`q`退出产生的报错
根据gdb单步执行结果, 发现`engine_start()`函数结束后返回`is_exit_status_bad()`，此函数定义在`nemu/src/utils/state.c`根据其要求，发现返回值为`-1`. 查看代码，他是利用`nemu_state`来判定返回值. 其中判断语句为:

# PA2

# PA3

# PA4