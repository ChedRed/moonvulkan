/* The MIT License (MIT)
 *
 * Copyright (c) 2017 Stefano Trettel
 *
 * Software repository: MoonVulkan, https://github.com/stetre/moonvulkan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

static int freecommand_pool(lua_State *L, ud_t *ud)
    {
    VkCommandPool command_pool = (VkCommandPool)ud->handle;
    VkDevice device = ud->device;
    const VkAllocationCallbacks *allocator = ud->allocator;
    freechildren(L, COMMAND_BUFFER_MT, ud);
    if(!freeuserdata(L, ud))
        return 0; /* double call */
    TRACE_DELETE(command_pool, "command_pool");
    UD(device)->ddt->DestroyCommandPool(device, command_pool, allocator);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud, *device_ud;
    VkResult ec;
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo info;
    const VkAllocationCallbacks *allocator;

    VkDevice device = checkdevice(L, 1, &device_ud);

    if(lua_istable(L, 2))
        {
        allocator = optallocator(L, 3);
        if(echeckcommandpoolcreateinfo(L, 2, &info)) return argerror(L, 2);
        }
    else
        {
        allocator = NULL;
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = NULL;
        info.flags = checkflags(L, 2);
        info.queueFamilyIndex = luaL_checkinteger(L, 3);
        }

    ec = device_ud->ddt->CreateCommandPool(device, &info, allocator, &command_pool);

    CheckError(L, ec);
    TRACE_CREATE(command_pool, "command_pool");
    ud = newuserdata_nondispatchable(L, command_pool, COMMAND_POOL_MT);
    ud->parent_ud = device_ud;
    ud->device = device;
    ud->instance = device_ud->instance;
    ud->allocator = allocator;
    ud->destructor = freecommand_pool;
    ud->ddt = device_ud->ddt;
    return 1;
    }

static int ResetCommandPool(lua_State *L)
    {
    VkResult ec;
    ud_t *ud;
    VkCommandPool command_pool = checkcommand_pool(L, 1, &ud);
    VkDevice device = ud->device;
    VkCommandPoolResetFlags flags = optflags(L, 2, 0);
    ec = ud->ddt->ResetCommandPool(device, command_pool, flags);
    CheckError(L, ec);
    return 0;
    }

// void vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlagsKHR flags);

static int TrimCommandPool(lua_State *L)
    {
    ud_t *ud;
    VkCommandPool command_pool = checkcommand_pool(L, 1, &ud);
    VkDevice device = ud->device;
    VkCommandPoolTrimFlagsKHR flags = optflags(L, 2, 0);
    CheckDevicePfn(L, ud, TrimCommandPoolKHR);
    ud->ddt->TrimCommandPoolKHR(device, command_pool, flags);
    return 0;
    }


RAW_FUNC(command_pool)
TYPE_FUNC(command_pool)
INSTANCE_FUNC(command_pool)
DEVICE_FUNC(command_pool)
PARENT_FUNC(command_pool)
DELETE_FUNC(command_pool)
DESTROY_FUNC(command_pool)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "instance", Instance },
        { "device", Device },
        { "parent", Parent },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "create_command_pool",  Create },
        { "destroy_command_pool",  Destroy },
        { "reset_command_pool", ResetCommandPool },
        { "trim_command_pool", TrimCommandPool },
        { NULL, NULL } /* sentinel */
    };


void moonvulkan_open_command_pool(lua_State *L)
    {
    udata_define(L, COMMAND_POOL_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

