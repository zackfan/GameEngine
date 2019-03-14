//
// Created by creator on 19-3-14.
//

#include<iostream>
#include "IApplication.h"

using namespace std;
using namespace My;

namespace My {
    //extern声明此变量在别处定义，要在此处引用
    extern IApplication* g_pApp;
}

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

