#! https://zhuanlan.zhihu.com/p/538979204
# 操作系统大赛：基于 eBPF 的容器监控工具 Eunomia 初赛报告

基于 eBPF 的轻量级 CloudNative Monitor 工具，用于容器安全性和可观察性

## 1. 目录
<!-- TOC -->

- [1. 目录](#1-目录)
- [2. 目标描述](#2-目标描述)
  - [2.1. 功能概述](#21-功能概述)
  - [2.2. Tutorial](#22-tutorial)
- [3. 比赛题目分析和相关资料调研](#3-比赛题目分析和相关资料调研)
  - [3.1. 题目描述](#31-题目描述)
  - [3.2. 赛题分析](#32-赛题分析)
  - [3.3. 相关资料调研](#33-相关资料调研)
    - [3.3.1. ebpf](#331-ebpf)
    - [3.3.2. ebpf 开发工具技术选型](#332-ebpf-开发工具技术选型)
    - [3.3.3. 容器可观测性](#333-容器可观测性)
    - [3.3.4. 信息可视化展示](#334-信息可视化展示)
    - [3.3.5. 容器运行时安全](#335-容器运行时安全)
- [4. 系统框架设计](#4-系统框架设计)
  - [4.1. 系统设计](#41-系统设计)
  - [4.2. 模块设计](#42-模块设计)
  - [4.3. ebpf 主要观测点](#43-ebpf-主要观测点)
  - [4.4. ebpf 探针设计](#44-ebpf-探针设计)
    - [4.4.1. ebpf 探针相关 C 代码设计，以 process 为例：](#441-ebpf-探针相关-c-代码设计以-process-为例)
    - [4.4.2. C++ 部分探针代码设计](#442-c-部分探针代码设计)
    - [4.4.3. handler 相关事件处理代码](#443-handler-相关事件处理代码)
  - [4.5. 容器追踪模块设计](#45-容器追踪模块设计)
    - [4.5.1. 容器信息数据结构](#451-容器信息数据结构)
    - [4.5.2. 容器追踪实现](#452-容器追踪实现)
  - [4.6. 安全规则设计](#46-安全规则设计)
  - [4.7. seccomp: syscall准入机制](#47-seccomp-syscall准入机制)
- [5. 开发计划](#5-开发计划)
  - [5.1. 日程表](#51-日程表)
  - [5.2. 未来的工作方向](#52-未来的工作方向)
- [6. 比赛过程中的重要进展](#6-比赛过程中的重要进展)
- [7. 系统测试情况](#7-系统测试情况)
  - [7.1. 快速上手](#71-快速上手)
  - [7.2. 命令行测试情况](#72-命令行测试情况)
    - [7.2.1. tracker系列命令](#721-tracker系列命令)
  - [7.3. 容器测试情况](#73-容器测试情况)
  - [7.4. 信息可视化测试情况： prometheus and grafana](#74-信息可视化测试情况-prometheus-and-grafana)
  - [7.5. CI/持续集成](#75-ci持续集成)
  - [7.6. benchmark](#76-benchmark)
- [8. 遇到的主要问题和解决方法](#8-遇到的主要问题和解决方法)
  - [8.1. 如何设计 ebpf 挂载点](#81-如何设计-ebpf-挂载点)
  - [8.2. 如何进行内核态数据过滤和数据综合](#82-如何进行内核态数据过滤和数据综合)
  - [8.3. 如何定位容器元信息](#83-如何定位容器元信息)
  - [8.4. 如何设计支持可扩展性的数据结构](#84-如何设计支持可扩展性的数据结构)
- [9. 分工和协作](#9-分工和协作)
- [10. 提交仓库目录和文件描述](#10-提交仓库目录和文件描述)
  - [10.1. 项目仓库目录结构](#101-项目仓库目录结构)
  - [10.2. 各目录及其文件描述](#102-各目录及其文件描述)
    - [10.2.1. bpftools目录](#1021-bpftools目录)
    - [10.2.2. cmake目录](#1022-cmake目录)
    - [10.2.3. doc目录](#1023-doc目录)
    - [10.2.4. include目录](#1024-include目录)
    - [10.2.5. libbpf目录](#1025-libbpf目录)
    - [10.2.6. src目录](#1026-src目录)
    - [10.2.7. test目录](#1027-test目录)
    - [10.2.8. third_party目录](#1028-third_party目录)
    - [10.2.9. tools目录](#1029-tools目录)
    - [10.2.10. vmlinux目录](#10210-vmlinux目录)
- [11. 比赛收获](#11-比赛收获)
  - [11.1. 郑昱笙同学](#111-郑昱笙同学)
  - [11.2. 张典典同学](#112-张典典同学)
  - [11.3. 濮雯旭同学](#113-濮雯旭同学)
- [12. 附录](#12-附录)
  - [12.1. Prometheus 观测指标](#121-prometheus-观测指标)
    - [12.1.1. Process Metrics](#1211-process-metrics)
    - [12.1.2. files Metrics](#1212-files-metrics)
    - [12.1.3. Tcp Connect Metrics](#1213-tcp-connect-metrics)
    - [12.1.4. Syscall Metrics](#1214-syscall-metrics)
    - [12.1.5. Security Event Metrics](#1215-security-event-metrics)
    - [12.1.6. Service Metrics](#1216-service-metrics)
    - [12.1.7. PromQL Example](#1217-promql-example)
  - [12.2. 命令行工具帮助信息](#122-命令行工具帮助信息)

<!-- /TOC -->

## 2. 目标描述

本项目由两部分组成：

* 一个零基础入门 `eBPF` 技术的教程实践和对应的命令行工具集，主要使用 `C/C++` 语言开发, 同时作为原型验证;
* 一个基于 `eBPF` 技术实现的用于监控容器的工具(**Eunomia**), 包含 `profile`、容器集群网络可视化分析、容器安全感知告警、一键部署、持久化存储监控等功能, 主要使用 C/C++ 语言开发, 力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案;

### 2.1. 功能概述

`Eunomia` 是一个使用 C/C++ 开发的基于eBPF的云原生监控工具，旨在帮助用户了解容器的各项行为、监控可疑的容器安全事件，力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。它使用 `Linux` `eBPF` 技术在运行时跟踪您的系统和应用程序，并分析收集的事件以检测可疑的行为模式。目前，它包含 `profile`、容器集群网络可视化分析*、容器安全感知告警、一键部署、持久化存储监控等功能。

* [X] 开箱即用：以单一二进制文件或 `docker` 镜像方式分发，一次编译，到处运行，一行代码即可启动，包含多种 ebpf 工具和多种监测点，支持多种输出格式（json, csv, etc) 并保存到文件；
* [X] 通过 `ebpf` 自动收集容器相关元信息，并和多种指标相结合；
* [X] 可集成 `prometheus` 和 `Grafana`，作为监控可视化和预警平台；
* [X] 可自定义运行时安全预警规则, 并通过 prometheus 等实现监控告警; 
* [X] 可以自动收集进程系统调用行为并通过 seccomp 进行限制；
* [ ] 可通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；可外接时序数据库，如 `InfluxDB` 等，作为可选的信息持久化存储和数据分析方案；

除了收集容器中的一般系统运行时内核指标，例如系统调用、网络连接、文件访问、进程执行等，我们在探索实现过程中还发现目前对于 `lua` 和 `nginx` 相关用户态 `profile` 工具和指标可观测性开源工具存在一定的空白，但又有相当大的潜在需求；因此我们还计划添加一系列基于 uprobe 的用户态 `nginx/lua` 追踪器，作为可选的扩展方案；（这部分需求来自中科院开源之夏， APISIX 社区的选题）

和过去常用的 `BCC` 不同，`Eunomia` 基于 `Libbpf` + BPF CO-RE（一次编译，到处运行）开发。Libbpf 作为 BPF 程序加载器，接管了重定向、加载、验证等功能，BPF 程序开发者只需要关注 BPF 程序的正确性和性能即可。这种方式将开销降到了最低，且去除了庞大的依赖关系，使得整体开发流程更加顺畅。目前，我们已经发布了 `pre-release` 的版本，其中部分功能已经可以试用，只需下载二进制文件即可运行，


### 2.2. Tutorial

`Eunomia` 的 `ebpf` 追踪器部分是从 `libbpf-tools` 中得到了部分灵感，但是目前关于 ebpf 的资料还相对零散且过时，这也导致了我们在前期的开发过程中走了不少的弯路。因此, 我们也提供了一系列教程，以及丰富的参考资料，旨在降低新手学习eBPF技术的门槛，试图通过大量的例程解释、丰富对 `eBPF、libbpf、bcc` 等内核技术和容器相关原理的认知，让后来者能更深入地参与到 ebpf 的技术开发中来。另外，`Eunomia` 也可以被单独编译为 C++ 二进制库进行分发，可以很方便地添加自定义 libbpf检查器，或者直接利用已有的功能来对 syscall 等指标进行监测，教程中也会提供一部分 `EUNOMIA` 扩展开发接口教程。

> 教程目前还在完善中。

1. [eBPF介绍](doc/tutorial/0_eBPF介绍.md)
2. [eBPF开发工具介绍: BCC/Libbpf，以及其他](doc/tutorial/1_eBPF开发工具介绍.md)
3. [基于libbpf的内核级别跟踪和监控: syscall, process, files 和其他](doc/tutorial/2_基于libbpf的内核级别跟踪和监控.md)
4. [基于uprobe的用户态nginx相关指标监控](doc/tutorial/3_基于uprobe的用户态nginx相关指标监控.md)
5. [seccomp权限控制](doc/tutorial/4_seccomp权限控制.md)
6. [上手Eunomia: 基于Eunomia捕捉内核事件](doc/tutorial/x_基于Eunomia捕捉内核事件.md)

## 3. 比赛题目分析和相关资料调研

### 3.1. 题目描述

容器是一种应用层抽象，用于将代码和依赖资源打包在一起。多个容器可以在同一台机器上运行，共享操作系统内核。这使得容器的隔离性相对较弱，带来安全上的风险，最严重时会导致容器逃逸，严重影响底层基础设施的保密性、完整性和可用性。

eBPF 是一个通用执行引擎，能够高效地安全地执行基于系统事件的特定代码，可基于此开发性能分析工具**、网络数据包过滤、系统调用过滤，**系统观测和分析等诸多场景。eBPF可以由hook机制在系统调用被使用时触发，也可以通过kprobe或uprobe将eBPF程序附在内核/用户程序的任何地方。

这些机制让eBPF的跟踪技术可以有效地感知容器的各项行为，包括但不限于：

- 容器对文件的访问
- 容器对系统的调用
- 容器之间的互访

请基于eBPF技术开发一个监控工具，该工具可以监控容器的行为，并生成报表（如json文件）将各个容器的行为分别记录下来以供分析。

- **第一题：行为感知**

  编写eBPF程序，感知容器的各项行为。
- **第二题：信息存储**

  在第一题的基础上，令工具可以将采集到的数据以特定的格式保存在本地。
- **第三题：权限推荐（可选）**

  Seccomp是Linux内核的特性，开发者可以通过seccomp限制容器的行为。capabilities则将进程作为root的权限分成了各项更小的权限，方便调控。这两个特性都有助于保障容器安全，但是因为业务执行的逻辑差异，准确配置权限最小集非常困难。请利用上面开发的监控工具，分析业务容器的行为记录报表，然后基于报表自动推荐精准的权限配置最小集。
### 3.2. 赛题分析
  本赛题分为三个部分，第一部分为编写ebpf程序，感知容器行为，这一部分的关键点在于找到合适的ebpf程序挂载点。挂载点确定后我们可以使用现有的各种ebpf开发框架编写开发代码，完成此部分任务。第二部分信息存储则需要我们将内核态捕捉到的信息传递到用户态，然后在用户态进行输出。这一部分具有较大的可操作空间。我们可以使用最简单的打印方式，将所有数据打印出来，也可以将所有数据存储到日志文件中，还可以通过可视化的手段，将数据进行可视化展示。第三部分则涉及到了一个新的模块Seccomp，seccomp是linux内核中的一个安全模块，可以限制某一进程可以调用的syscall的数量。该技术可以进一步增强本工具的监督保护能力，因此本次也应当将其融合进入本项目。

### 3.3. 相关资料调研

#### 3.3.1. ebpf

eBPF是一项革命性的技术，可以在Linux内核中运行沙盒程序，而无需更改内核源代码或加载内核模块。通过使Linux内核可编程，基础架构软件可以利用现有的层，从而使它们更加智能和功能丰富，而无需继续为系统增加额外的复杂性层。

* 优点：低开销

  eBPF 是一个非常轻量级的工具，用于监控使用 Linux 内核运行的任何东西。虽然 eBPF 程序位于内核中，但它不会更改任何源代码，这使其成为泄露监控数据和调试的绝佳伴侣。eBPF 擅长的是跨复杂系统实现无客户端监控。 
* 优点：安全

  解决内核观测行的一种方法是使用内核模块，它带来了大量的安全问题。而eBPF 程序不会改变内核，所以您可以保留代码级更改的访问管理规则。此外，eBPF 程序有一个验证阶段，该阶段通过大量程序约束防止资源被过度使用，保障了运行的ebpf程序不会在内核产生安全问题。
* 优点：精细监控、跟踪

  eBPF 程序能提供比其他方式更精准、更细粒度的细节和内核上下文的监控和跟踪标准。并且eBPF监控、跟踪到的数据可以很容易地导出到用户空间，并由可观测平台进行可视化。 
* 缺点：很新

  eBPF 仅在较新版本的 Linux 内核上可用，这对于在版本更新方面稍有滞后的组织来说可能是令人望而却步的。如果您没有使用较新版本的 Linux 内核，那么 eBPF 根本不适合您。

#### 3.3.2. ebpf 开发工具技术选型

原始的eBPF程序编写是非常繁琐和困难的。为了改变这一现状，llvm于2015年推出了可以将由高级语言编写的代码编译为eBPF字节码的功能，同时，其将 `bpf()` 
等原始的系统调用进行了初步地封装，给出了 `libbpf` 库。这些库会包含将字节码加载到内核中的函数以及一些其他的关键函数。在Linux的源码包的 `samples/bpf/` 目录下，有大量Linux提供的基于 `libbpf` 的eBPF样例代码。一个典型的基于 `libbpf` 的eBPF程序具有 `*_kern.c` 和 `*_user.c` 两个文件，
 `*_kern.c` 中书写在内核中的挂载点以及处理函数， `*_user.c` 中书写用户态代码，完成内核态代码注入以及与用户交互的各种任务。

更为详细的教程可以参考[该视频](https://www.bilibili.com/video/BV1f54y1h74r?spm_id_from=333.999.0.0)。

然而由于该方法仍然较难理解且入门存在一定的难度，因此现阶段的eBPF程序开发大多基于一些工具，比如：

- `BCC`
- `BPFtrace`
- `libbpf`
- `go-libbpf`
- etc

目前使用较多的是 `BCC` 工具，但本项目放弃了 `BCC` ，选择了 `libbpf` 作为我们的开发工具。  

`BCC` 全称为 `BPF Compiler Collection` ，是一个python库，包含了完整的编写、编译、和加载 `BPF` 程序的工具链，以及用于调试和诊断性能问题的工具。自2015年发布以来，`BCC` 经过上百位贡献者地不断完善后，目前已经包含了大量随时可用的跟踪工具。并且 [其官方项目库](https://github.com/iovisor/bcc/blob/master/docs/tutorial.md) 提供了一个方便上手的教程，用户可以快速地根据教程完成 `BCC` 入门工作。用户可以在 `BCC` 上使用Python、Lua等高级语言进行编程。

相较于使用C语言直接编程，这些高级语言具有极大的便捷性，用户只需要使用C来设计内核中的 `BPF` 程序，其余包括编译、解析、加载等工作在内，均可由 `BCC` 完成。

然而使用 `BCC` 存在一个缺点便是在于其兼容性并不好。基于 `BCC` 的 `eBPF` 程序每次执行时候都需要进行编译，编译则需要用户配置相关的头文件和对应实现。在实际应用中，相信大家也会有体会，编译依赖问题是一个很棘手的问题。也正是因此，在本项目的开发中我们放弃了BCC，选择了可以做到一次编译-多次运行的 `libbpf` 工具。

`libbpf-bootstrap` 是一个基于 `libbpf` 库的BPF开发脚手架，从其 [github](https://github.com/libbpf/libbpf-bootstrap) 上可以得到其源码。 `libbpf-bootstrap` 综合了BPF社区过去多年的实践，为开发者提了一个现代化的、便捷的工作流，实现了一次编译，重复使用的目的。

基于 `libbpf` 的BPF程序在编译时会先将 `*.bpf.c` 文件编译为对应的`.o`文件，然后根据此文件生成 `skeleton` 文件，即 `*.skel.h` ，这个文件会包含内核态中定义的一些数据结构，以及用于装载内核态代码的关键函数。在用户态代码 `include` 此文件之后调用对应的装载函数即可将字节码装载到内核中。

我们选择现代 C++ 语言（cpp20）开发 Eunomia 的时候也主要是看中和 libbpf 库以及 bpf 代码的良好兼容性，libbpf 库目前还在迅速更新迭代过程中，我可以直接基于 libbpf 库进行开发，不需要被其他语言（go/rust）的运行时 bpf 库所限制。现代 C++ 的开发速度和安全性应该并不会比其他语言差太多（要是编译提示能像 rust 那样好点就更好了，用了 concept 还是不够好）

#### 3.3.3. 容器可观测性

`Docker`类容器本身提供了较多命令用于观测容器，比如：
- `docker ps` 命令可以显示出目前正在运行的所有容器的ID，名称，运行时间等等数据，

- `docker top` 命令可以显示容器中所有正在运行的进程，并且显示其在宿主机上的进程号，通过这种方式我们可以在宿主机中找到和容器有关的进程号并进行重点追踪。

- `docker inspect` 命令则可以根据需要具体查看容器的各种信息。

通过这些命令，我们可以较为快速地得到容器内的一些情况。

容器中的进程会映射到宿主机中，他们和宿主机上的其他进程最直接的区别就在于namespace。为了隔离资源，容器中的进程和宿主机上的进程具有不同的namespace。因此，监测容器行为可以转变为监测特定进程，通过复用现有process模块的基础上添加container追踪模块。  

容器追踪模块的内核态ebpf代码和process模块一样，都是利用了 `sched_process_exec` 和 `sched_process_exit` 两个挂载点，区别在于用户态代码中对于内核态返回的数据的处理方式。在容器追踪模块中，每次有内核态数据写入时我们会调用 `judge_container()` 函数，该函数会检查此进程的namespace和其父进程是否相同。如果相同那么我们就会认为他和父进程是归为一类的，通过检查存有所有容器进程信息的哈希map即可确定此新进程的归属。如果不同，那么我们便会认为可能有新的容器产生。对于 `Docker` 类容器，我们会直接调用 `Docker` 给出的命令的进行观测。首先调用 `docker ps -q` 命令获得现有在运行的所有容器id，之后调用 `docker top id` 命令获取容器中的进程在宿主机上的进程信息，如果这些信息没有被记录到哈希map中，那么就将他们添加到其中并输出。在有进程退出时，我们只需要检查其是否存在于哈希map，如果存在删去即可。

#### 3.3.4. 信息可视化展示

`Prometheus` 是一套开源的监控、报警、时间序列数据库的组合，受启发于Google的Brogmon监控系统，2012年开始由前Google工程师在Soundcloud以开源软件的形式进行研发，并且于2015年早期对外发布早期版本。2016年，`Prometheus` 加入了云计算基金会，成为 `kubernetes` 之后的第二个托管项目。其架构如下所示:
<div  align="center">  
 <img src="./imgs//promethesu_arch.png" width = "600" height = "400" alt="prometheus_architecture" align=center />
 <p>Prometheus架构</p>
</div>

![](./imgs/promethesu_arch.png)   

`Prometheus`具有以下特点：
- 可以自定义多维数据模型并且使用metric和
- 存储高效，不依赖分布式存储，支持单节点工作
- 使用灵活且强大的查询语言 `PromQL`
- 通过基于http的pull方式采集时许数据
- 通过push gateway进行序列数据推送

`Grafana` 是一款用Go语言开发的开源数据可视化工具，具有数据监控、数据统计和告警功能，是目前较为流行的一种时序数据展示工具，并且支持目前绝大部分常用的时序数据库。

在本项目中，我们计划将程序捕获到的数据使用 `Prometheus` 进行存储，之后对于存储的数据我们使用 `Grafana` 进行可视化

#### 3.3.5. 容器运行时安全

确保容器运行时安全的关键点：

- 使用 `ebpf` 跟踪技术自动生成容器访问控制权限。包括：容器对文件的可疑访问，容器对系统的可疑调用，容器之间的可疑互访，检测容器的异常进程，对可疑行为进行取证。例如：
- 检测容器运行时是否创建其他进程。
- 检测容器运行时是否存在文件系统读取和写入的异常行为，例如在运行的容器中安装了新软件包或者更新配置。
- 检测容器运行时是否打开了新的监听端口或者建立意外连接的异常网络活动。
- 检测容器中用户操作及可疑的 shell 脚本的执行。

## 4. 系统框架设计

### 4.1. 系统设计

<div  align="center">  
 <img src="./imgs/architecture.jpg" width = "600" height = "400" alt="eunomia_architecture" align=center />
 <p>系统架构</p>
</div>

关于详细的系统架构设计和模块划分，请参考 [系统设计文档](doc/design_doc)

### 4.2. 模块设计


- tracker_manager

  负责启动和停止 ebpf 探针，并且和 ebpf 探针通信（每个 tracer 是一个线程）；

  - start tracker
  - stop tracker(remove tracker)

  我们主要有五个ebpf探针:

  - process
  - syscall
  - tcp
  - files
  - ipc

- container_manager

  负责观察 container 的启动和停止，保存每个 container 的相关信息：（cgroup，namespace），同时负责 container id, container name 等 container mata 信息到 pid 的转换（提供查询接口）

- seccomp_manager

  负责对 process 进行 seccomp 限制

- handler/data collector

  负责处理 ebpf 探针上报的事件

- security analyzer

  容器安全检测规则引擎和安全分析模块，通过ebpf采集到的底层相关数据，运用包括AI在内的多种方法进行安全性分析，可以帮助您检测事件流中的可疑行为模式。
  
- prometheus exporter

  将数据导出成Prometheus需要的格式，在Prometheus中保存时序数据，方便后续持久化和可视化功能。

- config loader

  解析 toml

- cmd

  命令行解析模块，将命令行字符串解析成对应的参数选项，对Eunomia进行配置。

- core

  负责装配所需要的 tracker，配置对应的功能用例，并且启动系统。

- server

  http 通信：通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析。这一个部分还没有完成。

### 4.3. ebpf 主要观测点

- process追踪模块

  进程的追踪模块本项目主要设置了两个 `tracepoint` 挂载点。
  第一个挂载点形式为

  ```c
          SEC("tp/sched/sched_process_exec")
          int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
          {
      
          }
  ```

  当进程被执行时，该函数会被调用，函数体中会从传入的上下文内容提取内容，我们需要的信息记录在Map中。
  第二个挂载点形式为

  ```c
          SEC("tp/sched/sched_process_exit")
          int handle_exit(struct trace_event_raw_sched_process_template *ctx)
          {
              
          }
  ```

  当有进程退出时，该函数会被调用，函数体同样会从传入的上下文内容提取内容，我们需要的信息记录在Map中。

- syscall追踪模块

  对于系统调用的追踪模块设置了一个 `tracepoint` 挂载点。挂载点形式为
  ```c
          SEC("tracepoint/raw_syscalls/sys_enter")
          int sys_enter(struct trace_event_raw_sys_enter *args)
          {
      
          }
  ```
  当有syscall发生时，其经过`sys_enter`执行点时我们的函数将会被调用，将相关信息存入map后供用户态读取。

- file追踪模块

  对于文件系统，我们设置了两个 `kprobe` 挂载点。第一个挂载点形式为
  ```c
          SEC("kprobe/vfs_read")
          int BPF_KPROBE(vfs_read_entry, struct file *file, char *buf, size_t count, loff_t *pos)
          {
      
          }
  ```
  第二个挂载点形式为
  ```c
          SEC("kprobe/vfs_write")
          int BPF_KPROBE(vfs_write_entry, struct file *file, const char *buf, size_t count, loff_t *pos)
          {
      
          }
  ```
  当系统中发生了文件读或写时，这两个执行点下的函数会被触发，记录相应信息。

-  tcp追踪模块   

    ```c
            SEC("kprobe/tcp_v6_connect")
            int BPF_KPROBE(tcp_v6_connect, struct sock *sk) {
              return enter_tcp_connect(ctx, sk);
            }

            SEC("kretprobe/tcp_v6_connect")
            int BPF_KRETPROBE(tcp_v6_connect_ret, int ret) {
              return exit_tcp_connect(ctx, ret, 6);
            }
    ```

### 4.4. ebpf 探针设计

采用 ebpf 探针的方式，可以获取到安全事件的相关信息，并且可以通过 prometheus 监控指标进行监控和分析。

我们的探针代码分为两个部分，其一是在 `bpftools` 中，是针对相关 ebpf 程序的 libbpf 具体探针接口实现，负责1ebpf 程序的加载、配置、以及相关用户态和内核态通信的代码；另外一部分是在 src 中，针对 ebpf 探针上报的信息进行具体处理的 C++ 类实现，负责根据配置决定ebpf上报的信息将会被如何处理。

#### 4.4.1. ebpf 探针相关 C 代码设计，以 process 为例：

process 部分的代码主要负责获取进程的执行和退出时和进程相关的以下的信息：

- pid
- cgroup
- namespace：user pid mount
- ppid
- command
- 可执行文件路径

其中容器相关信息会保存起来并被其他 tracker 用以查询。

ebpf 代码：在 bpftools\process\process.bpf.c 中，这里贴出来的代码经过了一定程度的化简。

```c
static __always_inline void fill_event_basic(pid_t pid, struct task_struct *task, struct process_event *e)
{
	e->common.pid = pid;
	e->common.ppid = BPF_CORE_READ(task, real_parent, tgid);
	e->common.cgroup_id = bpf_get_current_cgroup_id();
	e->common.user_namespace_id = get_current_user_ns_id();
	e->common.pid_namespace_id = get_current_pid_ns_id();
	e->common.mount_namespace_id = get_current_mnt_ns_id();
}


SEC("tp/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
{
	struct task_struct *task;
	unsigned fname_off;
	struct process_event *e;
	pid_t pid;
	u64 ts;

	/* remember time exec() was executed for this PID */
	pid = bpf_get_current_pid_tgid() >> 32;
	if (target_pid && pid != target_pid)
		return 0;
	ts = bpf_ktime_get_ns();
	bpf_map_update_elem(&exec_start, &pid, &ts, BPF_ANY);

	/* reserve sample from BPF ringbuf */
	e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
	if (!e)
		return 0;

	/* fill out the sample with data */
	task = (struct task_struct *)bpf_get_current_task();
	if (exclude_current_ppid) {
		if (exclude_current_ppid == BPF_CORE_READ(task, real_parent, tgid)) {
			return 0;
		}
	}
	fill_event_basic(pid, task, e);

	bpf_get_current_comm(&e->comm, sizeof(e->comm));
	e->exit_event = false;
	fname_off = ctx->__data_loc_filename & 0xFFFF;
	bpf_probe_read_str(e->filename, sizeof(e->filename), (void *)ctx + fname_off);

	/* successfully submit it to user-space for post-processing */
	bpf_ringbuf_submit(e, 0);
	return 0;
}

```

这部分是负责处理进程执行的代码，通过挂载点 `tp/sched/sched_process_exec` 来监测所有的进程执行和退出相关情况。其中包含了对进程的相关信息的获取，以及对进程的相关信息的填充。具体进程相关的信息会被放到这个结构体中，并传递给 C++ 编写的处理程序：

bpftools\process\process.h
```c

struct common_event {
	int pid;
	int ppid;
	uint64_t cgroup_id;
	uint32_t user_namespace_id;
	uint32_t pid_namespace_id;
	uint32_t mount_namespace_id;
};

struct process_event
{
	struct common_event common;

	unsigned exit_code;
	unsigned long long duration_ns;
	char comm[TASK_COMM_LEN];
	char filename[MAX_FILENAME_LEN];
	bool exit_code;
};
```

C++ 部分通过 `start_process_tracker` 函数来加载 process 相关 ebpf 探针，并且注册回调函数。以下是相关签名：


代码在 bpftools\process\process_tracker.h 中：
```c
static int start_process_tracker(
    ring_buffer_sample_fn handle_event,
    libbpf_print_fn_t libbpf_print_fn,
    struct process_env env,
    struct process_bpf *skel,
    void *ctx);
```

每个 ebpf 探针会被当做一个独立的线程运行，这个线程会被放到一个单独的线程池中，这样就可以保证每个 ebpf 探针都是独立的进程：

- 我们可以在同一个二进制程序或者进程中同时运行多个探针，例如可以同时运行 process 和 tcp，通过 process 获取的容器元信息，以 pid 作为主键来查询 tcp 每个连接相关的容器信息。
- 探针可以在 eunomia 运行的任意时刻被启动，也可以在任意时刻被关闭。
- 同一种类型的探针可以被运行多个实例，比如来监测不同的 cgroups 或者不同的进程。

这样设计的目的是，例如如果我们需要进行在线的监控数据获取和分析，可以通过远端的 http 请求，让 eunomia 往内核中注入一个 ebpf 探针，运行 30 秒后停止该探针，然后通过 graphql 请求，通过外界数据库或者内置的算子进行数据聚合，之后返回获取的数据指标。这样的好处是可以不用在任何时候都必须运行某些代价高昂的监控服务（例如 syscall 监控），极大地节省相关服务器资源，避免干扰正常的服务运行。

每个探针有两个重要的数据结构， event 和 env。event 上报给用户态的信息结构体， env是对应的 tracker 的配置：

```cpp
struct process_env
{
  bool verbose;
  pid_t target_pid;
  pid_t exclude_current_ppid;
  long min_duration_ms;
  volatile bool *exiting;
};

```

C++ 部分的代码会在调用 start_process_tracker 之前设置好对应的 env 信息，来控制 ebpf 代码的相关行为。

#### 4.4.2. C++ 部分探针代码设计

我们采用类似责任链的设计模式，通过一系列的回调函数和事件处理类来处理 ebpf 上报的内核事件：

- 每个 ebpf 探针都是一个单独的类
- 每个探针类都可以有数量不限的事件处理 handler 类（例如转换成 json 类型，上报给 prometheus，打印输出，保存文件，进行聚合等），它们通过类似链表的方式组织起来，并且可以在运行被动态组装；

以 process 为例，c++部分的探针代码如下：

see: include\eunomia\process.h
```cpp
// ebpf process tracker interface
// the true implementation is in process/process_tracker.h
//
// trace process start and exit
struct process_tracker : public tracker_with_config<process_env, process_event>
{
  using config_data = tracker_config<process_env, process_event>;
  using tracker_event_handler = std::shared_ptr<event_handler<process_event>>;

  process_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<process_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  process_tracker(process_env env);
  // start process tracker
  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<process_event>
  {
    prometheus::Family<prometheus::Counter> &eunomia_process_start_counter;
    prometheus::Family<prometheus::Counter> &eunomia_process_exit_counter;
    void report_prometheus_event(const struct process_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<process_event> &e);
  };

  // convert event to json
  struct json_event_handler_base : public event_handler<process_event>
  {
    std::string to_json(const struct process_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler_base
  {
    void handle(tracker_event<process_event> &e);
  };
  
  // used for print to console
  struct plain_text_event_printer : public event_handler<process_event>
  {
    void handle(tracker_event<process_event> &e);
  };
  
};
```

这部分代码继承自 tracker_base，每个 ebpf 探针的代码都会继承自 tracker_base 和 tracker_with_config:

include\eunomia\model\tracker.h
```cpp

// the base type of a tracker
// for tracker manager to manage
struct tracker_base
{
  // base thread
  std::thread thread;
  volatile bool exiting;
  // TODO: use the mutex
  std::mutex mutex;

 public:
  virtual ~tracker_base()
  {
    exiting = true;
    if (thread.joinable())
    {
      thread.join();
    }
  }
  virtual void start_tracker(void) = 0;
  void stop_tracker(void)
  {
    exiting = true;
  }
};

// all tracker should inherit from this class
template<typename ENV, typename EVENT>
struct tracker_with_config : public tracker_base
{
  tracker_config<ENV, EVENT> current_config;
  tracker_with_config(tracker_config<ENV, EVENT> config) : current_config(config)
  {
  }
};
```

分成两个类设计的目的是为了同时完成运行时多态编译期多态。其中 tracker_config 是对应的模板类，包含了探针的配置信息和处理事件的 handler，比如：

include\eunomia\model\tracker_config.h
```cpp 

// config data for tracker
// pass this to create a tracker
template <typename ENV, typename EVENT>
struct tracker_config
{   
    // tracker env in C code
    ENV env;
    std::string name;
    // event handler interface
    std::shared_ptr<event_handler<EVENT>> handler = nullptr;
};

```

每个 ebpf 探针类都要满足对应的 concept，比如：

include\eunomia\model\tracker.h
```cpp
// concept for tracker
// all tracker should have these types
template<typename TRACKER>
concept tracker_concept = requires
{
  typename TRACKER::config_data;
  typename TRACKER::tracker_event_handler;
  typename TRACKER::prometheus_event_handler;
  typename TRACKER::json_event_printer;
  typename TRACKER::plain_text_event_printer;
};

```

这个 concept 规定了 tracker 必须要实现的 handler ，以及需要有的子类型。

#### 4.4.3. handler 相关事件处理代码

每个探针类都可以有数量不限的事件处理 handler 类（例如转换成 json 类型，上报给 prometheus，打印输出，保存文件，进行聚合等），它们通过类似链表的方式组织起来，并且可以在运行被动态组装；

- ebpf 上报的 event 会按顺序被 handler 处理，如果 handler 返回 false，则 event 不会被后续的 handler 处理，否则会一直被处理到最后一个 handler（捕获机制）；
- 上报的 event 可以被转换成不同的类型，即可以做聚合操作，也可以从 event 结构体转换成 json 类型；
- 多个不同的 ebpf 探针可以把 event 发送给同一个 handler，例如将文件访问信息和 process 执行信息合并成一个 event，获取每个文件访问的进程的 docker id，docker name，然后发送给 prometheus；
- handler 同样可以用来匹配对应的安全规则，在出现可能的安全风险的时候执行告警操作；

例如，上面所描述的 process 类就有对应的 handler：

- prometheus_event_handler;
- json_event_printer;
- plain_text_event_printer;

我们的安全风险分析和安全告警也可以基于对应的handler 实现，例如：

include\eunomia\sec_analyzer.h
```cpp
// base class for securiy rules
template<typename EVNET>
struct rule_base : event_handler<EVNET>
{
  std::shared_ptr<sec_analyzer> analyzer;
  rule_base(std::shared_ptr<sec_analyzer> analyzer_ptr) : analyzer(analyzer_ptr) {}
  virtual ~rule_base() = default;

  // return rule id if matched
  // return -1 if not matched
  virtual int check_rule(const tracker_event<EVNET> &e, rule_message &msg) = 0;
  void handle(tracker_event<EVNET> &e);
};
```

handler 的具体实现在 include\eunomia\model\event_handler.h 中。

我们设计了有多种类型的 handler，并通过模板实现：

- 接受单一线程的事件，并且把同样的事件传递给下一个handler，只有一个 next handler；（事件传递）
- 接受单一线程的事件，并且把不同的事件传递给下一个handler，只有一个 next handler；（类型转换，如做聚合操作）
- 接受单一线程的事件，并且把不同的事件传递给下一个handler，可以有多个 next handler；（多线程传递）
- 接受多个线程传递的事件，并且把事件传递给下一个handler，只有一个 next handler；这部分需要有多线程同步，可以用无锁队列实现；

所有的 handler 都继承自 event_handler_base，它规定了 handler 接受的事件类型：

include\eunomia\model\event_handler.h
```cpp
template <typename T>
struct event_handler_base
{
public:
    virtual ~event_handler_base() = default;
    virtual void handle(tracker_event<T> &e) = 0;
    virtual void do_handle_event(tracker_event<T> &e) = 0;
};
```

对于第一类的 handler，也是我们目前最经常用到的事件处理程序，它的模板如下：

```cpp
template <typename T>
struct event_handler : event_handler_base<T>
{
// ptr for next handler
std::shared_ptr<event_handler_base<T>> next_handler = nullptr;
public:
    virtual ~event_handler() = default;

    // implement this function to handle the event
    virtual void handle(tracker_event<T> &e) = 0;

    // add a next handler after this handler
    std::shared_ptr<event_handler<T>> add_handler(std::shared_ptr<event_handler<T>> handler)
    {
        next_handler = handler;
        return handler;
    }
    // do the handle event
    // pass the event to next handler
    void do_handle_event(tracker_event<T> &e)
    {   
        bool is_catched = false;
        try {
           is_catched = handle(e);
        } catch (const std::exception& error) {
            std::cerr << "exception: " << error.what() << std::endl;
            is_catched = true;
        }
        if (!is_catched && next_handler)
            next_handler->do_handle_event(e);
        return;
    }
};
```

例如 prometheus_event_handler，它就继承自 event_handler 类。每个探针上报的 ebpf 事件都会被转换成 tracker_event 类型，然后传递给 event_handler event_handler 类的 handle 方法就是对事件进行处理，并且传递给下一个 handler：handler 被组织成为单链表的形式（也可以是树或者有向无环图的形式），这样就可以实现事件的传递。

其他类型的 handler 可以参考 include\eunomia\model\event_handler.h 文件。


### 4.5. 容器追踪模块设计

#### 4.5.1. 容器信息数据结构
目前我们的容器追踪模块是基于进程追踪模块实现的，其数据结构为：
```c
struct container_event {
	struct process_event process;
	unsigned long container_id;
	char container_name[50];
};
```
容器追踪模块由`container_tracker`实现
```cpp
struct container_tracker : public tracker_with_config<container_env, container_event>
{
  struct container_env current_env = { 0 };
  struct container_manager &this_manager;
  std::shared_ptr<spdlog::logger> container_logger;

  container_tracker(container_env env, container_manager &manager);
  void start_tracker();

  void fill_event(struct process_event &event);

  void init_container_table();

  void print_container(const struct container_event &e);

  void judge_container(const struct process_event &e);

  static int handle_event(void *ctx, void *data, size_t data_sz);
};
```
同时我们添加了一个manager类来控制tracker。
```cpp
struct container_manager
{
 private:
  struct tracker_manager tracker;
  std::mutex mp_lock;
  std::unordered_map<int, struct container_event> container_processes;
  friend struct container_tracker;

 public:
  void start_container_tracing(std::string log_path)
  { 
    tracker.start_tracker(std::make_unique<container_tracker>(container_env{
      .log_path = log_path,
      .print_result = true,
    }, *this));
  }
  unsigned long get_container_id_via_pid(pid_t pid);
};
```

#### 4.5.2. 容器追踪实现

容器追踪模块的ebpf代码服用了process追踪模块的ebpf代码，因此这里我们只介绍用户态下对数据处理的设计。   
当内核态捕捉到进程的数据返回到用户态时，我们调用`judge_container()`函数，判断该进程是否归属于一个container，其具体实现为：
```cpp
void container_tracker::judge_container(const struct process_event &e)
{
  if (e.exit_event)
  {
    this_manager.mp_lock.lock();
    auto event = this_manager.container_processes.find(e.common.pid);
    // remove from map
    if (event != this_manager.container_processes.end())
    {
      event->second.process.exit_event = true;
      print_container(event->second);
      this_manager.container_processes.erase(event);
    }
    this_manager.mp_lock.unlock();
  }
  else
  {
    /* parent process exists in map */
    this_manager.mp_lock.lock();
    auto event = this_manager.container_processes.find(e.common.ppid);
    this_manager.mp_lock.unlock();
    if (event != this_manager.container_processes.end())
    {
      struct container_event con = { .process = e, .container_id = (*event).second.container_id };
      strcpy(con.container_name, (*event).second.container_name);
      this_manager.mp_lock.lock();
      this_manager.container_processes[e.common.pid] = con;
      print_container(this_manager.container_processes[e.common.pid]);
      this_manager.mp_lock.unlock();
    }
    else
    {
      /* parent process doesn't exist in map */
      struct process_event p_event = { 0 };
      p_event.common.pid = e.common.ppid;
      fill_event(p_event);
      if ((p_event.common.user_namespace_id != e.common.user_namespace_id) ||
          (p_event.common.pid_namespace_id != e.common.pid_namespace_id) ||
          (p_event.common.mount_namespace_id != e.common.mount_namespace_id))
      {
        std::unique_ptr<FILE, int (*)(FILE *)> fp(popen("docker ps -q", "r"), pclose);
        unsigned long cid;
        /* show all alive container */
        pid_t pid, ppid;
        while (fscanf(fp.get(), "%lx\n", &cid) == 1)
        {
          std::string top_cmd = "docker top ", name_cmd = "docker inspect -f '{{.Name}}' ";
          char hex_cid[20], container_name[50];
          sprintf(hex_cid, "%lx", cid);
          top_cmd += hex_cid;
          name_cmd += hex_cid;
          std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose),
              name(popen(name_cmd.c_str(), "r"), pclose);
          fscanf(name.get(), "/%s", container_name);
          char useless[150];
          /* delet the first row */
          fgets(useless, 150, top.get());
          while (fscanf(top.get(), "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
          {
            this_manager.mp_lock.lock();
            /* this is the first show time for this process */
            if (this_manager.container_processes.find(pid) == this_manager.container_processes.end())
            {
              struct container_event con = {
                .process = e,
                .container_id = cid,
              };
              strcpy(con.container_name, container_name);
              this_manager.container_processes[pid] = con;
              print_container(this_manager.container_processes[pid]);
            }
            this_manager.mp_lock.unlock();
          }
        }
      }
    }
  }
}

```

首先，如果进程处于退出状态，那么该函数会直接判断其数据是否已经存在于`container_processes`这一哈希map中。该哈希map专门用于存储归属于容器的进程的信息。如果已经存在这直接输出并删除，否则跳过。如果进程处于执行状态，我们首先会检查该进程的父进程是否存在于`container_processes`中，如果存在则认为此进程也是容器中的进程，将此进程直接加入并输出即可。如果不存在则检查其namespace信息和其父进程是否一致，如果不一致我们会认为此时可能会有一个新的容器产生。对于`Docker`类容器，我们会直接调用`Docker`给出的命令的进行观测。首先调用`docker ps -q`命令获得现有在运行的所有容器id，之后调用`docker top id`命令获取容器中的进程在宿主机上的进程信息，如果这些信息没有被记录到哈希map中，那么就将他们添加到其中并输出。    
由于这一方式无法捕捉到在本追踪器启动前就已经在运行的容器进程，因此我们会在程序启动伊始，调用一次`init_container_table()`函数，其实现为：
```cpp
void container_tracker::init_container_table()
{
  unsigned long cid;
  pid_t pid, ppid;
  std::string ps_cmd("docker ps -q");
  std::unique_ptr<FILE, int (*)(FILE *)> ps(popen(ps_cmd.c_str(), "r"), pclose);
  while (fscanf(ps.get(), "%lx\n", &cid) == 1)
  {
    std::string top_cmd("docker top "), name_cmd("docker inspect -f '{{.Name}}' ");
    char hex_cid[20], container_name[50];
    sprintf(hex_cid, "%lx", cid);
    top_cmd += hex_cid;
    name_cmd += hex_cid;
    std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose),
        name(popen(name_cmd.c_str(), "r"), pclose);
    fscanf(name.get(), "/%s", container_name);
    /* delet the first row */
    char useless[150];
    fgets(useless, 150, top.get());
    while (fscanf(top.get(), "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
    {
      struct process_event event;
      event.common.pid = pid;
      event.common.ppid = ppid;
      fill_event(event);
      struct container_event con = {
        .process = event,
        .container_id = cid,
      };
      strcpy(con.container_name, container_name);
      print_container(con);
      this_manager.mp_lock.lock();
      this_manager.container_processes[pid] = con;
      this_manager.mp_lock.unlock();
    }
  }
}
```
该函数的实现逻辑与`judge_contaienr()`函数类似，但是它会将已经在运行的容器进程存入哈希map中，以方便后续追踪。

### 4.6. 安全规则设计

目前安全告警部分还未完善，只有一个框架和 demo，我们需要对更多的安全相关规则，以及常见的容器安全风险情境进行调研和完善，然后再添加更多的安全分析。


- 安全分析和告警

  目前我们的安全风险等级主要分为三类（未来可能变化，我觉得这个名字不一定很直观）：

  include\eunomia\sec_analyzer.h
  ```cpp
  enum class sec_rule_level
  {
    event,
    warnning,
    alert,
    // TODO: add more levels?
  };
  ```

  安全规则和上报主要由 sec_analyzer 模块负责：

  ```cpp

  struct sec_analyzer
  {
    // EVNETODO: use the mutex
    std::mutex mutex;
    const std::vector<sec_rule_describe> rules;

    sec_analyzer(const std::vector<sec_rule_describe> &in_rules) : rules(in_rules)
    {
    }
    virtual ~sec_analyzer() = default;
    virtual void report_event(const rule_message &msg);
    void print_event(const rule_message &msg);

    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(void);
    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules);
  };

  struct sec_analyzer_prometheus : sec_analyzer
  {
    prometheus::Family<prometheus::Counter> &eunomia_sec_warn_counter;
    prometheus::Family<prometheus::Counter> &eunomia_sec_event_counter;
    prometheus::Family<prometheus::Counter> &eunomia_sec_alert_counter;

    void report_prometheus_event(const struct rule_message &msg);
    void report_event(const rule_message &msg);
    sec_analyzer_prometheus(prometheus_server &server, const std::vector<sec_rule_describe> &rules);

    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(prometheus_server &server);
    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules, prometheus_server &server);
  };
  ```

  我们通过 sec_analyzer 类来保存所有安全规则以供查询，同时以它的子类 sec_analyzer_prometheus 完成安全事件的上报和告警。具体的告警信息发送，可以由 prometheus 的相关插件完成，我们只需要提供一个接口。由于 rules 是不可变的，因此它在多线程读条件下是线程安全的。

- 安全规则实现

  我们的安全风险分析和安全告警规则基于对应的handler 实现，例如：

  include\eunomia\sec_analyzer.h
  ```cpp

  // base class for securiy rules
  template<typename EVNET>
  struct rule_base : event_handler<EVNET>
  {
    std::shared_ptr<sec_analyzer> analyzer;
    rule_base(std::shared_ptr<sec_analyzer> analyzer_ptr) : analyzer(analyzer_ptr) {}
    virtual ~rule_base() = default;

    // return rule id if matched
    // return -1 if not matched
    virtual int check_rule(const tracker_event<EVNET> &e, rule_message &msg) = 0;
    void handle(tracker_event<EVNET> &e)
    {
      if (!analyzer)
      {
        std::cout << "analyzer is null" << std::endl;
      }
      struct rule_message msg;
      int res = check_rule(e, msg);
      if (res != -1)
      {
        analyzer->report_event(msg);
      }
    }
  };
  ```

  这个部分定义了一个简单的规则基类，它对应于某一个 ebpf 探针上报的事件进行过滤分析，以系统调用上报的事件为例：

  ```cpp
  // syscall rule:
  //
  // for example, a process is using a dangerous syscall
  struct syscall_rule_checker : rule_base<syscall_event>
  {
    syscall_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
    {}
    int check_rule(const tracker_event<syscall_event> &e, rule_message &msg);
  };
  ```

  其中的 check_rule 函数实现了对事件进行过滤分析，如果事件匹配了规则，则返回规则的 id，否则返回 -1：关于 check_rule 的具体实现，请参考：src\sec_analyzer.cpp

  除了通过单一的 ebpf 探针上报的事件进行分析之外，通过我们的 handler 机制，我们还可以综合多种探针的事件进行分析，或者通过时序数据库中的查询进行分析，来发现潜在的安全风险事件。

-  其他
  
  除了通过规则来实现安全风险感知，我们还打算通过机器学习等方式进行进一步的安全风险分析和发现。

### 4.7. seccomp: syscall准入机制

Seccomp(全称：secure computing mode)在2.6.12版本(2005年3月8日)中引入linux内核，将进程可用的系统调用限制为四种：read，write，_exit，sigreturn。最初的这种模式是白名单方式，在这种安全模式下，除了已打开的文件描述符和允许的四种系统调用，如果尝试其他系统调用，内核就会使用SIGKILL或SIGSYS终止该进程。Seccomp来源于Cpushare项目，Cpushare提出了一种出租空闲linux系统空闲CPU算力的想法，为了确保主机系统安全出租，引入seccomp补丁，但是由于限制太过于严格，当时被人们难以接受。

尽管seccomp保证了主机的安全，但由于限制太强实际作用并不大。在实际应用中需要更加精细的限制，为了解决此问题，引入了Seccomp – Berkley Packet Filter(Seccomp-BPF)。Seccomp-BPF是Seccomp和BPF规则的结合，它允许用户使用可配置的策略过滤系统调用，该策略使用Berkeley Packet Filter规则实现，它可以对任意系统调用及其参数（仅常数，无指针取消引用）进行过滤。Seccomp-BPF在3.5版（2012年7月21日）的Linux内核中（用于x86 / x86_64系统）和Linux内核3.10版（2013年6月30日）被引入Linux内核。

seccomp在过滤系统调用(调用号和参数)的时候，借助了BPF定义的过滤规则，以及处于内核的用BPF language写的mini-program。Seccomp-BPF在原来的基础上增加了过滤规则，大致流程如下：

<img src="./imgs/seccomp.png" weight=100% height=100%>

## 5. 开发计划

### 5.1. 日程表

阶段一：学习ebpf相关技术栈（3.10~4.2）

* [X] 入门ebpf技术栈
* [X] 调研、学习 `bcc`
* [X] 调研、学习 `libbpf` 、`libbpf-bootstrap`
* [X] 调研、学习 `seccomp`
* [X] 输出调研文档

阶段二：项目设计（4.3~4.10）

* [X] 与mentor讨论项目需求、并设计功能模块
* [X] 输出系统设计文档
* [X] 输出模块设计文档

阶段三：开发迭代（4.10~6.1）

* [X] 实现进程信息监控（pid、ppid等）
* [X] 实现系统调用信息监控
* [X] 实现进程间通信监控
* [X] 实现tcp（ipv4、ipv6）通信监控
* [X] 实现监控信息存储功能（csv或json格式）
* [X] 完成了系统的原型验证功能
* [X] 基于上述功能，实现命令行调用，完成版本v0.1
* [X] 输出开发v0.1日志文档
* [X] 实现进程id与容器id映射，进程信息过滤
* [X] 添加“seccomp”功能
* [x] 基于上述新增功能，迭代版本v0.2
* [X] 输出开发v0.2日志文档
* [x] 添加可视化模块: prometheus and grafana
* [X] add more tools from libbpf-tools
* [X] 基于上述新增功能，迭代版本v0.3
* [X] 输出开发v0.3日志文档
* [ ] 后续更新迭代

阶段四：开发测试（6.2~6.16）

* [ ] graphql for extentions
* [ ] lsm support
* [ ] add more rules
* [ ] 设计测试场景（分别针对基础功能、权限控制、安全逃逸场景）
* [X] 搭建测试环境
* [ ] 测试-开发
* [ ] 输出测试文档

阶段五：项目文档完善（6.17~7.1）

* [ ] 完善开发文档
* [ ] 完善教程文档
* [ ] 完善labs

### 5.2. 未来的工作方向

在未来我们计划继续按照上述日程表，完成我们未完成的工作，同时不断优化代码，使得Eunomia能成为一个具有较大使用价值的工具。主要有如下几个方向：

- 完善单元测试和测试场景，并且提供更完整的 benchmark；
- 完善教程文档；
- 完善安全规则和安全分析告警模块，这一部分目前只是个雏形；
- 添加更多的 ebpf 探针，并且支持自定义探针；
- 完善 http 在线分析模块；
- ......

## 6. 比赛过程中的重要进展

- 2022.5.1 首段代码push，实现了对系统调用的成功追踪
- 2022.5.15 完成了五大追踪模块的ebpf代码和简易用户态代码
- 2022.5.17 重构用户态代码，引入简易命令行控制
- 2020.5.20 正式定名Eunomia，该名字的原意是古希腊神话中的一位司管明智，法律与良好秩序女神。我们希望本工具也能在容器安全检测和可观测性中发挥到这样的作用。
- 2022.5.22 将CMake引入本工程，提高了项目编译的速度
- 2022.5.23 开始集成 Prometheus 模块进入工程
- 2022.5.24 重构用户态代码，基本确定命令行控制形式
- 2022.5.28 将日志记录工具spdlog引入本工程
- 2022.6.1 prometheus 和 Grafana 模块集成完成，设计相关 dashboard
- 2022.6.3 完成了 sec_analyzer 模块，对安全风险事件进行分析
- 2022.6.4 整理初赛文档；


## 7. 系统测试情况

### 7.1. 快速上手

从gitlab上clone本项目，注意，需要添加`--recursive`以clone子模块
```
git clone --recursive https://gitlab.eduxiji.net/zhangdiandian/project788067-89436.git
```
运行编译命令
```
sudo make install
```
编译完成后的所有可执行文件会在build目录下，eunomia可执行文件位于./build/bin/Debug/目录下，测试代码位于./build/test/目录下。使用
```
sudo ./eunomia run process
```
即可开启eunomia的process   
具体的命令行操作方法可以使用
```
sudo ./eunomia --help
```
进行查看

### 7.2. 命令行测试情况

各项命令测试结果如下：

#### 7.2.1. tracker系列命令

- process模块测试  
  - 追踪所有process
    ![所有追踪结果](./imgs/cmd_show/cmd_run_process_all.png)
  - 追踪所有process并设置输出格式为csv
    ![设置输出格式追踪结果](./imgs/cmd_show/cmd_run_process__fmt.png)
  - 追踪所有和id为7d4cc7108e89的容器有关的进程
    ![设置追踪容器的id](./imgs/cmd_show/cmd_run_process_container.png)
  - 追踪pid为322375的进程，并设置10s后自动退出
    ![设置追踪的进程和退出时间](./imgs/cmd_show/cmd_run_process_p_T.png)
  - 使用toml文件配置追踪参数(toml配置附在结果图后)
    ![使用toml配置参数](./imgs/cmd_show/cmd_run_process_config.png)
    ![toml格式](./imgs/cmd_show/toml.png)
  - 开启process追踪模块的同时开启单独的容器监视模块，并将记录写到指定文件夹
    ![开启contaienr_manager](./imgs/cmd_show/cmd_run_process_m.png)
    ![日志结果记录](./imgs/cmd_show/cmd_run_process_m2.png)
- tcp模块测试
  - 追踪所有tcp
    ![所有追踪结果](./imgs/cmd_show/cmd_run_tcp_all.png)
  - 追踪所有tcp并设置输出格式为csv
    ![设置输出格式追踪结果](./imgs/cmd_show/cmd_run_tcp_fmt.png)
  - 追踪所有和id为7d4cc7108e89的容器有关的进程
    ![设置追踪容器的id](./imgs/cmd_show/cmd_run_tcp_container.png)
  - 追踪pid为924913的进程，并设置15s后自动退出
    ![设置追踪的进程和退出时间](./imgs/cmd_show/cmd_run_tcp_p_T.png)
  - 使用toml文件配置追踪参数(toml配置与process相同)
    ![使用toml配置参数](./imgs/cmd_show/cmd_run_tcp_config.png)
  - 开启process追踪模块的同时开启单独的容器监视模块，并将记录写到指定文件夹
    ![开启contaienr_manager](./imgs/cmd_show/cmd_run_tcp_m.png)
    ![日志结果记录](./imgs/cmd_show/cmd_run_tcp_m2.png)
- syscall模块测试
  - 追踪所有syscall
    ![所有追踪结果](./imgs/cmd_show/cmd_run_syscall_all.png)  
  - 追踪所有syscall并设置输出格式为csv
    ![设置输出格式追踪结果](./imgs/cmd_show/cmd_run_syscall_fmt.png)
  - 使用toml文件配置追踪参数(toml配置与process相同)
    ![使用toml配置参数](./imgs/cmd_show/cmd_run_syscall_config.png)
  - 开启syscall追踪模块的同时开启单独的容器监视模块，并将记录写到指定文件夹
    ![开启contaienr_manager](./imgs/cmd_show/cmd_run_syscall_m.png)
    ![日志结果记录](./imgs/cmd_show/cmd_run_syscall_m2.png)
- files模块测试
  - 追踪所有文件读写
    ![所有追踪结果](./imgs/cmd_show/cmd_run_files_all.png)  
  - 追踪所有files读写并设置输出格式为json
    ![设置输出格式追踪结果](./imgs/cmd_show/cmd_run_files__fmt.png)
  - 使用toml文件配置追踪参数(toml配置与process相同)
    ![使用toml配置参数](./imgs/cmd_show/cmd_run_files_config.png)
  - 开启files追踪模块的同时开启单独的容器监视模块，并将记录写到指定文件夹
    ![开启contaienr_manager](./imgs/cmd_show/cmd_run_files_m.png)
    ![日志结果记录](./imgs/cmd_show/cmd_run_files_m2.png)


### 7.3. 容器测试情况

- 测试进程与容器id互相映射
  <img src="imgs/container_test_1.jpg" width=100% weigth=100%>

- 基于容器信息的可视化展示
  <img src="./imgs/counts-tcp.png" width=100% weigth=100%>

### 7.4. 信息可视化测试情况： prometheus and grafana
    
Grafana是一个开源的可视化和分析平台。允许查询、可视化、告警和监控的不同数据，无论数据存储在哪里。简单地说支持多种数据源，提供多种面板、插件来快速将复杂的数据转换为漂亮的图形和可视化的工具，另监控可自定义告警监控规则。Prometheus是高扩展性的监控和报警系统。它采用拉取策略获取指标数据，并规定了获取数据的API，用户可以通过exporter收集系统数据。

Eunomia能够将自定义的BPF跟踪数据导出到prometheus，它基于Prometheus-CPP这个SDK实现了prometheus获取数据的API，prometheus可以通过这些API主动拉取到自定义的BPF跟踪数据。具体来说，我们只需要在对应的tracker中嵌入BPF代码，运行Eunomia就可以实现导出BPF跟踪数据，而这些数据是可以被prometheus主动拉取到的，进而实现BPF跟踪数据的存储、处理和可视化展示。

Prometheus信息可视化测试：

  - 配置prometheus添加eunomia数据源
```
   job_name: "prometheus" 
     # metrics_path defaults to '/metrics'
     # scheme defaults to 'http'. 
     static_configs:
       - targets: ["localhost:9090"]
   job_name: "eunomia_node"
     static_configs:
       - targets: ["localhost:8528"] 
```
  - 从prometheus查看数据源的状态
    <img src="./imgs/prometheus4.png" width=100%>
  - 从promethesu查看eunomia暴露的指标列表
    <img src="./imgs/prometheus5.png" width=100%>
  - 从Prometheus查看部分指标的数值分布
    <img src="./imgs/prometheus1.png">
    <img src="./imgs/prometheus2.png">
    <img src="./imgs/prometheus3.png">
- grafana

  - grafana配置从peometheus拉取数据的端口
    <img src="./imgs/grafana1.png">
  - grafana部分指标展示效果如下图，左上为文件读操作Bytes监控;左下为为系统调用热力图，方便定位到热点调用路径;右上为文件读操作TOP10;右下为文件写操作TOP10。
    <img src="./imgs/grafana2.png">
    <img src="./imgs/grafana.png">

### 7.5. CI/持续集成

我们在 github 上面部署了 github actions，包含自动集成、自动构建、自动测试等功能：

![action](./imgs/ci.png)

### 7.6. benchmark

使用 top 查看 eunomia 的内存和cpu占用情况

![top](./imgs/top.png)

目前有一些简单的性能对比，使用 openresty 在本机上启动一个网络简单的服务，并且使用 wrk 进行压力测试。测试环境：

```
Linux ubuntu 5.13.0-44-generic #49~20.04.1-Ubuntu SMP x86_64 GNU/Linux
4 核，12 GB 内存：
```

这是未开启 eunomia server 的情况：

![no](imgs/openresty_no_eunomia.png)

这是启动 eunomia server 后的情况，使用默认配置并启用 process/container、tcp、files、ipc 等探针，在同样环境下进行测试：

![no](imgs/openresty_with_eunomia.png)

可以观测到仅有大约 2% 的性能损耗。

> OpenResty® 是一个基于 Nginx 与 Lua 的高性能 Web 平台，其内部集成了大量精良的 Lua 库、第三方模块以及大多数的依赖项。用于方便地搭建能够处理超高并发、扩展性极高的动态 Web 应用、Web 服务和动态网关。web开发人员可以使用lua编程语言，对核心以及各种c模块进行编程，可以利用openresty快速搭建超1万并发高性能web应用系统。这里的 benchmark 参考了：https://openresty.org/en/benchmark.html

目前还没有比较完善的 benchmark 测试和性能分析，这是我们接下来要完善的内容。

## 8. 遇到的主要问题和解决方法

### 8.1. 如何设计 ebpf 挂载点

如何设计挂载点是ebpf程序在书写时首先需要考虑的问题。ebpf程序是事件驱动的，即只有系统中发生了我们预先规定的事件，我们的程序才会被调用。因此，ebpf挂载点的选择直接关系到程序能否在我们需要的场合下被启动。

我们在选择挂载点时，首先需要明白的是我们需要在什么情况下触发处理函数，然后去寻找合适的挂载点。ebpf的挂载点有多种类型，较为常用的挂载点是 `tracepoint` ， `k/uprobe` ， `lsm` 等。

`tracepoint` 是一段静态的代码，以打桩的形式存在于程序源码中，并向外界提供钩子以挂载。一旦处理函数挂载到了钩子上，那么当钩子对应的事件发生时，处理函数就会被调用。由于 `tracepoint` 使用较为方便，且覆盖面广，ABI也较为稳定，他是我们设计挂载点的一个重要考虑对象。目前Linux已经有1000多个tracepoint可供选择，其支持的所有类型可以在 `/sys/kernel/debug/tracing/events/` 目录下看到，而至于涉及到的参数格式和返回形式，用户可以使用 `cat` 命令，查看对应 `tracepoint` 事件下的format文件得到。

如下便是`sched_process_exec`事件的输出格式。

![](./imgs/report/tracepoint_example1.png)

用户也可以直接访问 `tracepoint` 的源码获得更多信息。在Linux源码的 `./include/trace/events` 目录下，用户可以看到Linux中实现tracepoint的源码。  

`k/uprobe` 是Linux提供的，允许用户动态插桩的方式。由于 `tracepoint` 是静态的，如果用户临时需要对一些其不支持的函数进行追踪，就无法使用 `tracepoint` ，而 `k/uprobe` 允许用户事实对内核态/用户态中某条指令进行追踪。用户在指定了该指令的位置并启用 `k/uprobe` 后，当程序运行到该指令时，内核会自动跳转到我们处理代码，待处理完成后返回到原处。相较于 `tracepoint` ， `k/uprobe` 更为灵活，如果你需要追踪的指令不被 `tracepoint` 所支持，可以考虑使用 `k/uprobe`。

`lsm` 是Linux内核安全模块的一套框架，其本质也是插桩。相较于`tracepoint` ，`lsm` 主要在内核安全的相关路径中插入了hook点。因此如果你希望你的代码检测一些和安全相关的内容，可以考虑使用 `lsm` 。其所有钩子的定义在Linux源码的 `./include/linux/lsm_hook_defs.h` 中，你可以从中选择初你所需要的hook点。


### 8.2. 如何进行内核态数据过滤和数据综合

如果直接将所有捕获到的数据直接传递到用户态的话，将会带来很大的开销，ebpf 程序一个重要的特征就是能在内核态进行数据过滤和综合。我们设计了一系列数据综合和过滤模式，如：

- 根据次数进行统计，如 syscall 统计每个进程调用 syscall 的次数并存储在map中，而不是直接上报；
- 根据 pid、namespace、cgroup 进行过滤；
- process 短于一定时间间隔的不予统计；
- 根据一定时间进行统计采样，如 files

### 8.3. 如何定位容器元信息

在程序开始，我们调用 `docker ps -q` 命令获得当前所有正在运行的容器ID。

之后我们开始遍历这些ID，并对每一个ID调用 `docker top id` 命令,获得容器中的所有所有进程信息，并且将这些信息以键值对的形式存储到哈希map上。之后我们会在 `sched_process_exec` 和 `sched_process_exit` 的两个点挂载基于ebpf的处理函数，捕获进程信息。

如果捕获的进程与其父进程存在 namespace 变化的情况，那么我们就会重复一次开始的工作，判断是否有新的容器产生。

如果有，则将其添加到哈希map中。如果其父进程已经存在于哈希map中，那么我们就认为此进程也是一个容器相关进程，也将其存储到哈希map中。在进程退出时，我们则需要检查其是否存在于哈希map中，如果存在则需要将其删除。

处理函数的逻辑如下图所示：

![容器元信息处理处理](./imgs/container.jpg)


### 8.4. 如何设计支持可扩展性的数据结构

首先尽可能降低各个模块的耦合性，这样在修改时可以较为方便地完成改动。其次，在最初设计时为未来可能的扩展预留位置。这部分主要是 tracker 和 handler 的设计，具体可以参考我们的设计文档部分：[#44-ebpf-探针设计](#44-ebpf-探针设计)

## 9. 分工和协作

现在我们大部分的分工方式是在 gitlab 上面的 issue 中进行分配，并且且通过 issue 完成项目质量管理和追踪。参考：[issues](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/issues)

- 郑昱笙同学：负责项目架构设计和调研，负责了 process、files、syscall 相关探针设计；
- 张典典同学：主要负责了seccomp模块、可视化以及部分quickstart的工作
- 濮雯旭同学：主要负责了container和ipc追踪模块的撰写以及后期用户态代码中与命令行控制相关的重构工作


## 10. 提交仓库目录和文件描述


### 10.1. 项目仓库目录结构

本仓库的主要目录结构如下所示：   

  ```
  ├─bpftools       - ebpf内核态代码
  │  ├─container  
  │  ├─files  
  │  ├─ipc  
  │  ├─process  
  │  ├─seccomp  
  │  ├─syscall  
  │  └─tcp  
  ├─cmake  
  ├─doc           - 项目开发文档
  │  ├─develop_doc   
  │  ├─imgs  
  │  └─tutorial  
  ├─include       
  │   └─eunomia   - 项目主要头文件
  │       └─model  
  ├─libbpf  
  ├─src           - 项目主要代码
  ├─test          - 项目单元测试和集成测试
  │   └─src  
  ├─third_party  
  │       └─prometheus-cpp  
  ├─tools  
  └─vmlinux  
  ```
### 10.2. 各目录及其文件描述

#### 10.2.1. bpftools目录

本目录内的所有文件均为基于ebpf开发的内核态监视代码，
共有7个子目录，子目录名表示了子目录内文件所实现的模块。比如process子目录代表了其中的文件，主要实现了进程追踪方面的ebpf内核态代码，其他子目录同理。

#### 10.2.2. cmake目录

本项目使用cmake进行编译，本目录中的所有文件都是本项目cmake
的相关配置文件。

#### 10.2.3. doc目录

本目录内的所有文件为与本项目相关的文档，其中develop_doc目录为开发文档，其中记录了本项目开发的各种详细信息。tutorial目录为本项目为所有想进行ebpf开发的同学所设计的教学文档，其中会提供一些入门教程，方便用户快速上手。imgs目录为开发文档和教学文档中所需要的一些
图片。

#### 10.2.4. include目录

本项目中用户态代码的头文件均会存放在本目录下。eunomia子目录中存放的是各个模块和所需要的头文件，eunomia下的model子目录存放的是各个头文件中的一些必要结构体经过抽象后的声明。

#### 10.2.5. libbpf目录

该目录为libbpf-bootstrap框架中自带的libbpf头文件。

#### 10.2.6. src目录

该目录主要记录了各个模块的用户态代码cpp文件。

#### 10.2.7. test目录

本目录主要包括了对各个模块的测试代码。

#### 10.2.8. third_party目录

本模块为Prometheus库所需的依赖。

#### 10.2.9. tools目录

本模块主要包含了一些项目所需要的脚本。

#### 10.2.10. vmlinux目录

本目录主要是libbpf-bootstrap框架自带的vmlinux头文件。

## 11. 比赛收获

### 11.1. 郑昱笙同学

收获：

- 第一次大规模实践了现代 C++（cpp20） 的代码风格和开发模式，深入学习了相关语言知识和 cmake 编译工具链，以及代码持续集成、持续分析等知识；
- 对 ebpf 和操作系统相关概念有了深入的了解
- 对于云原生容器监控相关技术栈有一个深入的实践；

### 11.2. 张典典同学

此次比赛让我了解到了ebpf技术，借此机会对操作系统内核中进程相关的数据结构例如struct task、thread_info、regs等有了深入的了解，对于Linux的seccomp安全机制、cgroup机制有了充分的认识，亲手体验了使用内核监控的开发，通过应用Grafana等可视化组件，提升了我对时序数据的存储、操作、展示的理解。

### 11.3. 濮雯旭同学

此次比赛让我了解到了什么是ebpf技术，也亲手体验了一下如何使用ebpf技术开发内核监控程序，增强了我的工程能力。这对于立志从事操作系统相关工作的我来说具有重要的意义。与此同时，我还了解到了Prometheus，Graphna等众多新技术，也增长了我的视野。

## 12. 附录

### 12.1. Prometheus 观测指标

#### 12.1.1. Process Metrics

Process 探针相关的指标：

##### 12.1.1.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_process_start` | Counter | Number of observed process start |
| `eunomia_observed_process_end` | Counter | Number of observed process end |

##### 12.1.1.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `pod` | default | Name of the pod |
| `mount_namespace` | 46289463245 | Mount Namespace of the pod |
| `container_name` | Ubuntu | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `pid` | 12344 | The pid of the running process |
| `comm` | ps | The command of the running process |
| `filename` | /usr/bin/ps | The exec file name |
| `exit_code` | 0 | The exit code |
| `duration_ms` | 375 | The running time |


#### 12.1.2. files Metrics

文件读写探针相关的指标

##### 12.1.2.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_files_read_count` | Counter | Number of observed files read count |
| `eunomia_observed_files_write_count` | Counter | Number of observed files write count |
| `eunomia_observed_files_write_bytes` | Counter | Number of observed files read bytes |
| `eunomia_observed_files_read_bytes` | Counter | Number of observed files write bytes |

##### 12.1.2.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | eunomia | The command of the running process |
| `filename` | online | The exec file name |
| `pid` | 7686 | The pid of the running proces |
| `type` | 82 | Type of comm |

#### 12.1.3. Tcp Connect Metrics

tcp 连接探针相关的指标

##### 12.1.3.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_tcp_v4_count` | Counter | Number of observed tcp v4 connect count |
| `eunomia_observed_tcp_v6_count` | Counter | Number of observed tcp v6 connect count |

##### 12.1.3.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `dst` | 127.0.0.1 | Destination of TCP connection |
| `pid` | 4036 | The pid of the running proces |
| `port` | 20513 | TCP exposed ports |
| `src` | 127.0.0.1 | Resources of TCP connection |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `task` | Socket Thread | The task of the running process |
| `uid` | 1000 | The uid of the running proces |


#### 12.1.4. Syscall Metrics

系统调用探针相关的指标

##### 12.1.4.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_syscall_count` | Counter | Number of observed syscall count |

##### 12.1.4.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | firefox | The command of the running process |
| `pid` | 4036 | The pid of the running proces |
| `syscall` | writev | Name of the syscall called by running process |

#### 12.1.5. Security Event Metrics

安全风险相关的指标

##### 12.1.5.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_seccurity_warn_count` | Counter | Number of observed security warnings |
| `eunomia_seccurity_event_count` | Counter | Number of observed security event |
| `eunomia_seccurity_alert_count` | Counter | Number of observed security alert |

##### 12.1.5.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | firefox | The command of the running process |
| `pid` | 4036 | The pid of the running proces |
| `syscall` | writev | Name of the syscall called by running process |


#### 12.1.6. Service Metrics

Service metrics are generated from the eunomia server-side events, which are used to show the quality of eunomia own service.

##### 12.1.6.1. Metrics List

| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_run_tracker_total` | Counter | Total number of running trackers |

##### 12.1.6.2. Labels List

| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `namespace` | default | Namespace of the pod |
| `container` | api-container | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `ip` | 10.1.11.23 | The IP address of the entity |
| `port` | 80 | The listening port of the entity |

#### 12.1.7. PromQL Example

Here are some examples of how to use these metrics in Prometheus, which can help you understand them faster.

| **Describe** | **PromQL** |
| --- | --- |
| Request counts | `sum(increase(eunomia_observed_tcp_v4_count{}[1m])) by(task)` |
| read rate | `sum(rate(eunomia_observed_files_read_bytes{}[1m])) by(comm)` |
| write rate | `sum(rate(eunomia_observed_files_write_count{}[1m])) by(comm)` |


### 12.2. 命令行工具帮助信息

```sh
./eunomia 
SYNOPSIS
        bin/Debug/eunomia run [tcpconnect|syscall|ipc|process|files] [-c <container id>] [-p
                          <process id>] [-T <trace time in seconds>] [--config <config file>] [-m
                          [<path to store dir>]] [--fmt <output format of the program>]

        bin/Debug/eunomia safe [--config <config file>]
        bin/Debug/eunomia seccomp [-p <process id>] [-T <trace time in seconds>] [--config <config
                          file>] [-o [<output file name>]]

        bin/Debug/eunomia server [--config <config file>] [--no_safe] [--no_prometheus] [--listen
                          <listening address>]

        bin/Debug/eunomia help

OPTIONS
        -c, --container <container id>
                    The conatienr id of the contaienr the EUNOMIA will monitor

        -p, --process <process id>
                    The process id of the process the EUNOMIA will monitor

        -T <trace time in seconds>
                    The time the ENUNOMIA will monitor for

        --config <config file>
                    The toml file stores the config data

        -m <path to store dir>
                    Start container manager to trace contaienr.

        --fmt <output format of the program>
                    The output format of EUNOMIA, it could be "json", "csv", "plain_txt", and
                    "plain_txt" is the default choice.

        --config <config file>
                    The toml file stores the config data

        -p, --process <process id>
                    The process id of the process the EUNOMIA will monitor

        -T <trace time in seconds>
                    The time the ENUNOMIA will monitor for

        --config <config file>
                    The toml file stores the config data

        -o <output file name>
                    The output file name of seccomp

        --config <config file>
                    The toml file stores the config data

        --no_safe   Stop safe module
        --no_prometheus
                    Stop prometheus server

        --listen <listening address>
                    Listen http requests on this address, the format is like "127.0.0.1:8528"
```
