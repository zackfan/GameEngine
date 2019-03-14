//
// Created by creator on 19-3-14.
//

#pragma once

#include "IApplication.h"

namespace My{
    class BaseApplication : implements IApplication
    {
    public:
        //基类的纯虚函数都必须重载
        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick();
        virtual bool IsQuit();

    protected:
        bool m_bQuit;
    };
}
