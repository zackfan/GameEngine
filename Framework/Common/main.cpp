//
// Created by creator on 19-3-14.
//

#include<iostream>
#include "IApplication.hpp"

using namespace std;
using namespace My;

namespace My {
    //extern声明此变量在别处定义，要在此处引用
    extern IApplication* g_pApp;
}

//因为我们将不同平台的应用程序进行了抽象，所以我们的main函数不需要关心我们目前到底是工作在哪个平台。
//我们只需要通过IApplication接口提供的方法进行调用就可以了
int main(int argc, char** argv)
{
    int ret;

    if((ret=g_pApp->Initialize())!=0)
    {
        cout<<"App Initialize failed, will exit now!"<<endl;
        return ret;
    }

    while(!g_pApp->IsQuit())
    {
        g_pApp->Tick();
    }

    g_pApp->Finalize();

    return 0;
}

