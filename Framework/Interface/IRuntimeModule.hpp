//
// Created by creator on 19-3-14.
//

#pragma once

#include "Interface.hpp"

namespace My {
    Interface IRuntimeModule    //抽象基类
    {
    public:
        virtual ~IRuntimeModule(){}    //虚析构,分号可不要

        virtual int Initialize()=0;     //纯虚函数，为了让派生类强制实现，以防遗漏
        virtual void Finalize()=0;

        virtual void Tick()=0;
    };
}
