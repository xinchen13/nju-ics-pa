# PA0
### 安装Linux与相关工具
### 获取框架代码: [NJU ICS PA](https://github.com/NJU-ProjectN/ics-pa.git): `2022`作为默认分支
### 配置git, 并修改`/Makefile`中的ID与name, 以及开发追踪`git commit`的author 
### 尝试配置、编译并运行nemu

# PA1
### 初始化 `am-kernels`
### 在配置文件中选择ISA为riscv32
### 修复`welcome()`函数导致的错误
根据log输出注释`welcome()`函数中的`assert(0)`语句

### 修复运行NEMU之后直接键入`q`退出产生的报错
根据gdb单步执行结果, 发现`engine_start()`函数结束后返回`is_exit_status_bad()`，此函数定义在`nemu/src/utils/state.c`根据其要求，发现返回值为`-1`. 查看代码，他是利用`nemu_state`来判定返回值. 其中判断语句为:

```c
int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) || (nemu_state.state == NEMU_QUIT);
```

因此，只需要将按下`q`时的`nemu_state.state`设置为`NEMU_QUIT`即可，这样就可正常退出程序. 由于NEMU中`c`和`q`分别对应`nemu/src/monitor/sdb/sdb.c`中的`cmd_c()`与`cmd_q()`函数，发现`cmd_c()`函数调用了`cpu_exec()`, 此函数中对`nemu_state`作出了修改. 同理，我们可以直接在`cmd_q()`中添加代码改变`nemu_state`的值, 之后重新编译运行，测试无误

### 单步执行
框架代码中已经给出了模拟CPU执行方式的函数, 即在`nemu/src/cpu/cpu-exec.c`中的`cpu_exec(uint64_t n)`, 基于此，单步执行要求执行N条指令后暂停执行，换句话说就是从命令行读取到N ，就将`cpu_exec()`函数执行N次

首先在`cmd_table`中添加`si`命令，然后编写`cmd_si(char *args)`函数完成功能. 从命令行读取命令并获得参数，`cmd_help()`函数中给出了例子，直接按其的框架修改. 利用`strtok()`函数获取参数，缺省时默认为1; 给`cpu_exec()`传入参数n(即执行的指令条数即可)

`sscanf`通常被用来解析并转换字符串，其格式定义灵活多变，可以实现很强大的字符串解析功能: 在这里从字符串读入格式化内容，并解析为`int`类型数字, `strtok()`可以每次提取一个词条(提取参数)

### 打印寄存器
要求执行`info r`之后, 就调用`nemu/src/isa/$ISA/reg.c`目录下的`isa_reg_display()`, 在里面直接通过`printf()`输出所有寄存器的值即可. RTFSC可知，寄存器结构体`CPU_state`的定义放在`nemu/src/isa/$ISA/include/isa-def.h`中, 并在`nemu/src/cpu/cpu-exec.c`中定义一个全局变量`cpu`

首先在`cmd_table`中添加`info`命令, 并设置参数`r`. 之后就在`cmd_info(char *args)`函数中利用`strcmp()`进行参数判断与调用`isa_reg_display()`

在`isa_reg_display()`中通过`printf()`格式控制输出`pc`和32个通用寄存器的值，由于ISA为`riscv32`, 因此通用寄存器可以32位表示, 采用api提供的`FMT_WORD`来输出

# PA2

# PA3

# PA4