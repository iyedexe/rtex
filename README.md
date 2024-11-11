# RTEX
A low latency crypto trading system.   
using c++ and websockets, connections are implemented through message queues and asio selects for the fastest and most reliable data transfer.  
The aim is a framework that allows recording market data, backtesting trading strategies, live running and monitoring.

# What's inside?
market data recorder, supports binance exchange for now.  
live trader that loads a strategy a d runs it as per market events.  
a backtester that determines several metrics about a strategy in a specified time frame.   

# Progress
## Recorder: 
So far the recorder is ready, subscribing to market data streams and recording them into flat csv files for future usage.    
It also exposes the recording metrics on a prometheus client page to be scrapped and analyzed.   
## LiveTrader:
The circular arbitrage strategy has been implemented, not yet tested nor backtested.

# Next steps.
Generic Istrategy onmarket data handler.   
Generic Recorder for MDframes.   
Config file base recorder monitor port and configuration.   
Implement ED25519 and RSA auth on message sign for BNB Broker.   
Add into recorder configuration, recording of related symbols to certain coins.     
Find a method to bypass binance streams limitations.     
Archive recoding files bz2 and send to a datalake to be queried from a webinterface.   
Add unit tests.  

# Done.
Refactor scheduler for less functions.   
Refactor BNB Broker to pass methods for calls to API.    
Investigate crashes at recorder (feeder) after a few hours run and add retry protocol.  
Impelment a recovery mechanism in case of crash for feeder.   
Replace logging library with Quill for perf : https://github.com/odygrd/quill?tab=readme-ov-file#-table-of-contents.   

# How to use ?

## Run the recorder
```
/recorder --symbol all --date 2024-09-21 --configfile ../config/test_config.ini 
```
# But before setup the project !
## Install dependecies (Fedora instructions) :
Installation on debian varies so be careful on package names and install commands.   

```
sudo dnf update -y
sudo dnf install -y git cmake python gcc java-devel pip
sudo pip install numpy
```
### Install bazel
```
sudo yum install -y java-1.8.0-openjdk-devel wget
wget https://github.com/bazelbuild/bazel/releases/download/5.4.0/bazel-5.4.0-installer-linux-x86_64.sh
chmod +x bazel-5.4.0-installer-linux-x86_64.sh
sudo ./bazel-5.4.0-installer-linux-x86_64.sh
```

### Download tensor_flow_cc
```
git clone https://github.com/FloopCZ/tensorflow_cc.git
cd tensorflow_cc/tensorflow_cc
```

### Install tensor_flow_cc
```
cd tensorflow_cc
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig
```

### Install boost 
```
get bz2 from https://github.com/boostorg/boost/releases/tag/boost-1.86.0/boost-1.86.0-b2-nodocs.tar.gz 
tar -xzvf boost-1.86.0-b2-nodocs.tar.gz 
cd boost-1.86.0-b2-nodocs
sudo ./bootstrap
sudo ./b2 install
```

### Install openssl

```
sudo dnf install perl
wget https://www.openssl.org/source/openssl-3.3.1g.tar.gz
tar -xzvf openssl-1.1.1g.tar.gz
cd openssl-1.1.1g
./config
make
make test
sudo make install
```

### Install libsodium
```
wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.20-stable.tar.gz 
./configure
make && make check
sudo make install
```

### Install ZLIB and CURL
```
Fedora :
sudo dnf install zlib-devel zlib-static curl libcurl-devel
Debian: 
sudo apt-get install zlib1g-dev curl libcurl4-openssl-dev

```

### Update submodules
```
git submodule update --init --recursive
```

### Build project
```
mkdir build && cd build
cmake ..
make
```

# References :
https://wiki.hanzheteng.com/development/cmake/cmake-find_package.  
https://medium.com/@TomPJacobs/c-tensorflow-a-journey-bdecbbdd0f65.  
https://medium.com/@yurh/installing-prometheus-grafana-as-services-with-simple-bash-script-7a7488fc8afe.   
https://dev.to/tythos/cmake-and-git-submodules-more-advanced-cases-2ka.  
https://dane-bulat.medium.com/.   vim-setting-up-a-build-system-and-code-completion-for-c-and-c-eb263c0a19a1.     
* Binance spot WS streams api :  
https://developers.binance.com/docs/binance-spot-api-docs/web-socket-streams.   
* Binance spot WS api :  
https://developers.binance.com/docs/binance-spot-api-docs/web-socket-api.  

# Submodules:
```
* https://github.com/zaphoyd/websocketpp  
* https://github.com/nlohmann/json  
* https://github.com/gabime/spdlog
```

# Issues 
GCC=14
BAZEL=5.4

Error1.:  
```
ERROR: /home/iyedexe/.cache/bazel/_bazel_iyedexe/c86f93f3eb14ecfc0e9baa2126cdf78f/external/com_google_absl/absl/strings/BUILD.bazel:1078:11: Compiling absl/strings/internal/str_format/extension.cc failed: (Exit 1): gcc failed: error executing command /usr/bin/gcc -U_FORTIFY_SOURCE -fstack-protector -Wall -Wunused-but-set-parameter -Wno-free-nonheap-object -fno-omit-frame-pointer -g0 -O2 '-D_FORTIFY_SOURCE=1' -DNDEBUG -ffunction-sections ... (remaining 38 arguments skipped)
In file included from external/com_google_absl/absl/strings/internal/str_format/extension.cc:16:
external/com_google_absl/absl/strings/internal/str_format/extension.h:34:33: error: found ':' in nested-name-specifier, expected '::'
   34 | enum class FormatConversionChar : uint8_t;
```

Error1.1:   
```
ERROR: /home/iyedexe/.cache/bazel/_bazel_iyedexe/c86f93f3eb14ecfc0e9baa2126cdf78f/external/llvm-project/llvm/BUILD.bazel:178:11: Compiling llvm/lib/Support/Signals.cpp failed: (Exit 1): gcc failed: error executing command /usr/bin/gcc -U_FORTIFY_SOURCE -fstack-protector -Wall -Wunused-but-set-parameter -Wno-free-nonheap-object -fno-omit-frame-pointer -g0 -O2 '-D_FORTIFY_SOURCE=1' -DNDEBUG -ffunction-sections ... (remaining 77 arguments skipped)
In file included from external/llvm-project/llvm/lib/Support/Signals.cpp:14:
external/llvm-project/llvm/include/llvm/Support/Signals.h:119:8: error: variable or field 'CleanupOnSignal' declared void
  119 |   void CleanupOnSignal(uintptr_t Context);
      |        ^~~~~~~~~~~~~~~
```

Error1.2:   
```
ERROR: /home/iyedexe/workbench/tensorflow_cc/tensorflow_cc/build/tensorflow/tensorflow/core/lib/io/BUILD:207:11: Compiling tensorflow/core/lib/io/cache.cc failed: (Exit 1): gcc failed: error executing command /usr/bin/gcc -U_FORTIFY_SOURCE -fstack-protector -Wall -Wunused-but-set-parameter -Wno-free-nonheap-object -fno-omit-frame-pointer -g0 -O2 '-D_FORTIFY_SOURCE=1' -DNDEBUG -ffunction-sections ... (remaining 49 arguments skipped)
In file included from tensorflow/core/lib/io/cache.cc:16:
./tensorflow/core/lib/io/cache.h:99:11: error: 'uint64_t' does not name a type
   99 |   virtual uint64_t NewId() = 0;
      |           ^~~~~~~~
```

Solution1:

I think these are the relevant issues that are cause by gcc 13 https://gcc.gnu.org/gcc-13/porting_to.html#header-dep-changes
```
#include  <cstdint> (for std::int8_t, std::int32_t etc.)  in extension.h / Signals.h / cache.h 
```

# Notes :
Session Authentication

Note: Only Ed25519 keys are supported for this feature.

If you do not want to specify apiKey and signature in each individual request, you can authenticate your API key for the active WebSocket session.

Once authenticated, you no longer have to specify apiKey and signature for those requests that need them. Requests will be performed on behalf of the account owning the authenticated API key.

Note: You still have to specify the timestamp parameter for SIGNED requests.



Timing security

    SIGNED requests also require a timestamp parameter which should be the current millisecond timestamp.

    An additional optional parameter, recvWindow, specifies for how long the request stays valid.
        If recvWindow is not sent, it defaults to 5000 milliseconds.
        Maximum recvWindow is 60000 milliseconds.

    Request processing logic is as follows:

    if (timestamp < (serverTime + 1000) && (serverTime - timestamp) <= recvWindow) {
      // process request
    } else {
      // reject request
    }

Serious trading is about timing. Networks can be unstable and unreliable, which can lead to requests taking varying amounts of time to reach the servers. With recvWindow, you can specify that the request must be processed within a certain number of milliseconds or be rejected by the server.
