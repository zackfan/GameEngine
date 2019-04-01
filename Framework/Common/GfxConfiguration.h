//
// Created by creator on 19-4-1.
//

#pragma once

#include <cstdint>
#include <iostream>

namespace My
{
    struct GfxConfiguration
    {
        GfxConfiguration(uint32_t r = 8, uint32_t g = 8, uint32_t b = 8, uint32_t a = 8,
                         uint32_t d = 24, uint32_t s = 0, uint32_t msaa = 0,
                         uint32_t width = 1920, uint32_t height = 1080) :
                redBits(r), greenBits(g), blueBits(b), alphaBits(a),
                depthBits(d), stencilBits(s), msaaBits(msaa),
                screenwidth(width), screenheight(height)
        {}

        uint32_t redBits;
        uint32_t greenBits;
        uint32_t blueBits;
        uint32_t alphaBits;
        uint32_t depthBits;
        uint32_t stencilBits;
        uint32_t msaaBits;
        uint32_t screenwidth;
        uint32_t screenheight;

        //友元输出重载符
        friend std::ostream &operator<<(std::ostream &out, const GfxConfiguration conf)
        {
            out<<"GfxConfiguration:"<<
                 "R:"<<conf.redBits<<
                 "G:"<<conf.greenBits<<
                 "B:"<<conf.blueBits<<
                 "A:"<<conf.alphaBits<<
                 "D:"<<conf.depthBits<<
                 "S:"<<conf.stencilBits<<
                 "M:"<<conf.msaaBits<<
                 "W:"<<conf.screenwidth<<
                 "H:"<<conf.screenheight<<std::endl;
            return out;
        }
    };
}
