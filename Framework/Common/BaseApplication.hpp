//
// Created by creator on 19-3-14.
//

#pragma once

#include "IApplication.hpp"
#include "GfxConfiguration.h"

namespace My
{
    class BaseApplication : implements IApplication
    {
    public:
        BaseApplication(GfxConfiguration &conf);

        //基类的纯虚函数都必须重载
        virtual int Initialize();

        virtual void Finalize();

        virtual void Tick();

        virtual bool IsQuit();

    protected:
        static bool m_bQuit;
        GfxConfiguration m_Config;
    private:
        //hide the default construct to enforce a configuration
        //创建对象时，必须要载入config
        BaseApplication()
        {}
    };
}
