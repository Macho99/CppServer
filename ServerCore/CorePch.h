#pragma once

#include "Types.h"
#include "CoreMacro.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "Container.h"

#include <Windows.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

using std::array;
using std::atomic;
using std::basic_ifstream;
using std::cerr;
using std::cout;
using std::deque;
using std::enable_shared_from_this;
using std::endl;
using std::forward;
using std::function;
using std::list;
using std::locale;
using std::make_shared;
using std::make_unique;
using std::map;
using std::move;
using std::pair;
using std::queue;
using std::remove_pointer_t;
using std::set;
using std::shared_ptr;
using std::stack;
using std::static_pointer_cast;
using std::string;
using std::thread;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::wcout;
using std::weak_ptr;
using std::wstring;

namespace this_thread = std::this_thread;
using namespace std::chrono_literals;

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"

#include "ObjectPool.h"
#include "TypeCast.h"
#include "Memory.h"
#include "SendBuffer.h"
#include "Session.h"
#include "LockQueue.h"
#include "ConsoleLog.h"
