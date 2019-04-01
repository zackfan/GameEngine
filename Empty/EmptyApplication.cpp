//
// Created by creator on 19-3-14.
//

#include "BaseApplication.hpp"

namespace My{
    GfxConfiguration cfg=GfxConfiguration();
    BaseApplication g_App(cfg);
    //临时对象不能赋给引用
    IApplication* g_pApp = &g_App;
}
