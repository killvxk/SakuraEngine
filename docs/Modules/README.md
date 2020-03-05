<!--
 * @Author: your name
 * @Date: 2020-03-04 07:52:42
 * @LastEditTime: 2020-03-04 11:58:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \docs\Modules\README.md
 -->
## 模块
SakuraEngine使用完全的模块化开发。

### 什么是模块化开发
&emsp;&emsp;模块化是指解决一个复杂问题时自顶向下逐层把系统划分成若干模块的过程，有多种属性，分别反映其内部特性。例如我们将游戏引擎绘制的业务拆分为渲染设备模块, 渲染调度模块, 场景模块以及绘制提交模块。 

### 为什么采用模块化?
&emsp;&emsp;很多应用程序在长期的开发后陷入[焦油坑](https://zh.wikipedia.org/wiki/%E4%BA%BA%E6%9C%88%E7%A5%9E%E8%AF%9D#%E7%84%A6%E6%B2%B9%E5%9D%91)窘境, 难以进行进一步的扩展, 甚至无法维护。为了避免在开发后期遇到这样的问题, 需要提出一套解决方案。首先需要对应用程序开发做进一步的认知与理解。

实际上, 程序开发中的业务无外乎以下两项:
- 功能
- 状态(数据)

在实际项目中, 有如下的规律:

|问题\难度|开发|维护|
|:--:|:--:|:--:|
|功能|难|易|
|状态|nil|难|

&emsp;&emsp;对于难以开发的功能, 我们本着复用的态度。大部分时候代码级的复用已经足够, 但是由于数据(尤其是状态)的全局唯一性, 我们做不到这种级别的粗暴复用。

!>&emsp;&emsp;比如说, 多次链接复用带状态的lib会致使应用程序中有多份对象副本。而代码级别的重定义则会引发Compile Error。

&emsp;&emsp;功能代码的强耦合会降低可读性, 但在实际维护上不会造成过大的困扰, 因为代码可以随意的拷贝修改, 很难造成纠缠性的难解问题。实际上在工程中, 大部分时候我们在和各种交错凌乱的状态进行斗争:
- 由于状态本身和符号强挂钩, 这使得状态一旦写下就再也难以移动;
- 状态与业务强挂钩, 这也意味着无论如何设计, 业务代码中都难以避免各种状态的复杂引用;
- 需求的复杂变化会引入更多的状态以及功能。
  - 对于功能, 我们可以弃用旧函数/接口, 重起炉灶使用新的函数/接口而不引入过多的问题。
  - 然而对于一个运行着的系统而言, 无论如何都无法无视掉一个旧的状态。无视并停止维护一个状态, 可能会使系统的旧组件无法再运行下去;
  
!>&emsp;&emsp;一个形象的例子是面向对象以及面向接口(这里不讨论他们的包含关系)开发, OOP的一部分问题在于**耦合了状态和功能**, 这使得使用某个对象的功能必须考虑它特有的那些状态。面向接口开发使得功能和状态**可以**解耦, 引用一个接口实际上是引用了对象的功能组。 
</br>&emsp;&emsp;请注意上文中的**可以**, 因为对象内的接口实现同样有权修改对象的状态, 这使得不规范的接口化开发会退化至OOP。除非接口被声明为功能如此, 我们总应该在实现中引入更少的副作用。</br>&emsp;&emsp;或者仅使用**静态类+函数/模板**进行**纯功能**的实现, 而非陷入对象化的泥潭。

&emsp;&emsp;模块化的核心功能就是将业务进行**子服务化**, 每个服务(模块)专一地维护它自己的那些状态, 来避免状态地狱的产生。同时为了维持功能的可复用性, SakuraEngine推荐将模块的功能摘出实现, 导出为静态库以重用。将静态库链接至少量维护有状态的源并导出动态装载的模块, 如此可将维护状态的业务限制在极小的一个范围中。

### 如何在SakuraEngine中实现模块化开发
&emsp;&emsp;模块化开发的一大问题是维护模块信息与模块之间的关系。有以下几点:
- 链接
- 版本控制
- 依赖
- 装载与初始化
- 自动卸载与清理

&emsp;&emsp;由于C++缺失必须的语言级功能, 并且我们需要维护模块化开发的模块依赖, SakuraEngine的模块化开发基于自定义开发工具[SPA](/DevTools/SPA/SakuraPackageAdminister.md)。SPA需要用户在编辑时提供模块元数据(名称/版本/依赖等),工具自动在编译时将元数据注入, 并在运行时做依赖保序的加载以及初始化。

&emsp;&emsp;链接部分, SPA采取不干预的制度。这也是为了确保用户可以自由链接, 在大部分时候选择并链接到自己想要的静态库, 而不是递归地去链接动态库, 引入不必要的overhead和runtime eror。

&emsp;&emsp;版本控制的部分则在编译时进行保证。批处理文件会在编译时确认json内填写的依赖以及版本, 对于版本不符的依赖直接抛出error。SakuraEngine保证第一方插件的版本与引擎大版本一致, 这样可以简化引擎内的版本控制, 避免引擎插件间的版本冲突。

&emsp;&emsp;SPA同时支持**静态模块**和**动态模块**且都可以引入状态, 但在不能确保唯一链接的情况下, 请总是使用**动态模块**。

&emsp;&emsp;更为详细的内容, 请参阅[SPA](/DevTools/SPA/SakuraPackageAdminister.md)页。您可以在SPA的文档下学会如何创建用于SakruaEngine的模块, 并且了解到SPA批处理工具的一切细节。