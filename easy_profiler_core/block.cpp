/************************************************************************
* file name         : block.cpp
* ----------------- :
* creation time     : 2016/02/16
* authors           : Sergey Yagovtsev, Victor Zarubkin
* emails            : yse.sey@gmail.com, v.s.zarubkin@gmail.com
* ----------------- :
* description       : The file contains implementation of profiling blocks
*                   :
* license           : Lightweight profiler library for c++
*                   : Copyright(C) 2016-2017  Sergey Yagovtsev, Victor Zarubkin
*                   :
*                   : Licensed under either of
*                   :     * MIT license (LICENSE.MIT or http://opensource.org/licenses/MIT)
*                   :     * Apache License, Version 2.0, (LICENSE.APACHE or http://www.apache.org/licenses/LICENSE-2.0)
*                   : at your option.
*                   :
*                   : The MIT License
*                   :
*                   : Permission is hereby granted, free of charge, to any person obtaining a copy
*                   : of this software and associated documentation files (the "Software"), to deal
*                   : in the Software without restriction, including without limitation the rights
*                   : to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
*                   : of the Software, and to permit persons to whom the Software is furnished
*                   : to do so, subject to the following conditions:
*                   :
*                   : The above copyright notice and this permission notice shall be included in all
*                   : copies or substantial portions of the Software.
*                   :
*                   : THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
*                   : INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
*                   : PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
*                   : LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*                   : TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
*                   : USE OR OTHER DEALINGS IN THE SOFTWARE.
*                   :
*                   : The Apache License, Version 2.0 (the "License")
*                   :
*                   : You may not use this file except in compliance with the License.
*                   : You may obtain a copy of the License at
*                   :
*                   : http://www.apache.org/licenses/LICENSE-2.0
*                   :
*                   : Unless required by applicable law or agreed to in writing, software
*                   : distributed under the License is distributed on an "AS IS" BASIS,
*                   : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*                   : See the License for the specific language governing permissions and
*                   : limitations under the License.
************************************************************************/

#include "profile_manager.h"
#include "current_time.h"

using namespace profiler;

#ifndef EASY_PROFILER_API_DISABLED
BaseBlockData::BaseBlockData(timestamp_t _begin_time, block_id_t _descriptor_id)
    : m_begin(_begin_time)
    , m_end(0)
    , m_id(_descriptor_id)
{

}

BaseBlockData::BaseBlockData(timestamp_t _begin_time, timestamp_t _end_time, block_id_t _descriptor_id)
    : m_begin(_begin_time)
    , m_end(_end_time)
    , m_id(_descriptor_id)
{

}

Block::Block(Block&& that)
    : BaseBlockData(that.m_begin, that.m_id)
    , m_name(that.m_name)
    , m_status(that.m_status)
    , m_isScoped(that.m_isScoped)
{
    m_end = that.m_end;
    that.m_end = that.m_begin;
}

Block::Block(timestamp_t _begin_time, block_id_t _descriptor_id, const char* _runtimeName)
    : BaseBlockData(_begin_time, _descriptor_id)
    , m_name(_runtimeName)
    , m_status(::profiler::ON)
    , m_isScoped(true)
{

}

Block::Block(timestamp_t _begin_time, timestamp_t _end_time, block_id_t _descriptor_id, const char* _runtimeName)
    : BaseBlockData(_begin_time, _end_time, _descriptor_id)
    , m_name(_runtimeName)
    , m_status(::profiler::ON)
    , m_isScoped(true)
{

}

Block::Block(const BaseBlockDescriptor* _descriptor, const char* _runtimeName, bool _scoped)
    : BaseBlockData(1ULL, _descriptor->id())
    , m_name(_runtimeName)
    , m_status(_descriptor->status())
    , m_isScoped(_scoped)
{

}

void Block::start()
{
    m_begin = getCurrentTime();
}

void Block::start(timestamp_t _time)
{
    m_begin = _time;
}

void Block::finish()
{
    m_end = getCurrentTime();
}

void Block::finish(timestamp_t _time)
{
    m_end = _time;
}

Block::~Block()
{
    if (!finished())
        ::profiler::endBlock();
}
#else
BaseBlockData::BaseBlockData(timestamp_t, block_id_t)
    : m_begin(0)
    , m_end(0)
    , m_id(~0U)
{

}

Block::Block(Block&& that)
    : BaseBlockData(0, ~0U)
    , m_name("")
    , m_status(::profiler::OFF)
    , m_isScoped(that.m_isScoped)
{
}

Block::Block(timestamp_t, block_id_t, const char*)
    : BaseBlockData(0, ~0U)
    , m_name("")
    , m_status(::profiler::OFF)
    , m_isScoped(true)
{

}

Block::Block(timestamp_t, timestamp_t, block_id_t, const char*)
    : BaseBlockData(0, ~0U)
    , m_name("")
    , m_status(::profiler::OFF)
    , m_isScoped(true)
{

}

Block::Block(const BaseBlockDescriptor*, const char*, bool _scoped)
    : BaseBlockData(0, ~0U)
    , m_name("")
    , m_status(::profiler::OFF)
    , m_isScoped(_scoped)
{

}

void Block::start()
{
}

void Block::start(timestamp_t)
{
}

void Block::finish()
{
}

void Block::finish(timestamp_t)
{
}

Block::~Block()
{
}
#endif
