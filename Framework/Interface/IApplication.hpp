//
// Created by creator on 19-3-14.
//

//在很多平台上用户关闭应用程序都是通过系统通知过来的。我们的程序自身并不会直接进行这方面的判断。
//所以当我们收到这样的关闭通知的时候，我们就通过这个接口告诉主循环，我们该结束了.
//接下来我们可以直接从这个类派生出各个平台的Application类。
//但是实际上，各个平台的Application虽然有很多不同，共通点也是很多的。
//提高代码可维护性的一个重要做法，就是要避免同样的代码分散在不同的文件当中。

#pragma once

#include "Interface.hpp"
#include "IRuntimeModule.hpp"

namespace My{
    Interface IApplication : implements IRuntimeModule
    {
    public:
        virtual int Initialize()=0;
        virtual void Finalize()=0;

        //one cycle of main loop
        virtual void Tick()=0;
        virtual bool IsQuit()=0;
    };
}

