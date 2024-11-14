# Linux常用命令

## man

在Linux中，man手册页按照特定的部件编号进行分类。man手册页通常被划分为多个部分，每个部分都包含了特定类型的信息。在Linux系统中，man手册页的部分编号通常包括以下几个。  

1：用户命令或可执行程序的手册页，包括大多数用户使用的命令，比如ls、cat、grep等。  
2：系统调用（内核接口）的手册页，包括了系统调用相关的信息。  
3：库函数（包括C标准库和其他库）的手册页，包括了常用的函数库的信息。  
4：特殊文件的手册页，包括了设备文件和驱动程序的信息。  
5：配置文件和文件格式的手册页，包括了配置文件的格式和含义。  
**通过man man命令可以查看man手册页的详细信息。**

**通过man命令查看哪些手册中有要查找的内容：**

```bash
    man -k xxx
```

**通过命令查看所有系统调用列表：**

```bash
 apropos -s 2 . 
```

**通过命令查看所有库函数列表：**

```bash
    apropos -s 3 . 
```

**通过man手册查到系统调用或函数，查询头文件中的内容：**

```bash
    less /usr/include/xxx.h
```

**通过命令安装posix规范的man手册页：**

```bash
    sudo apt-get install manpages-posix
```

**通过命令安装C标准库的man手册页：**

```bash
    sudo apt-get install glibc-doc
```

## whereis

whereis可以列出命令所在man手册页的位置。  

## which

## find

## locate

## grep

## whatis 

## info

> info命令在Linux中用于读取“info”文档。"info"文档通常包含了比"man"页面更详细的信息，它们以更友好的方式提供了关于Linux命令和程序的详细信息，包括它们的功能、选项和用法。
>
> info命令的基本语法是 info [选项] [主题或文件名]。如果没有指定主题或文件名，info命令将显示一个菜单，列出所有可用的info文档。
>