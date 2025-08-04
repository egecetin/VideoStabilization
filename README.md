<div align="center">

# Repo-Init

<img src="doc/logo.png" alt="Repo-Init Logo" width="200">

### ⚡ **Modern C++ Application Template with Enterprise-Grade Features** ⚡

<p align="center">
  <img src="https://img.shields.io/github/languages/top/egecetin/Repo-Init?style=for-the-badge&logo=cplusplus&logoColor=white" alt="Top Language">
  <img src="https://img.shields.io/github/license/egecetin/Repo-Init?style=for-the-badge&color=brightgreen" alt="License">
</p>

<p align="center">
  <a href="https://github.com/egecetin/Repo-Init/actions/workflows/build_and_test.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/egecetin/Repo-Init/build_and_test.yml?label=Build%20%26%20Test&branch=master&logo=github&style=for-the-badge" alt="Build Status">
  </a>
  <a href="https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init">
    <img src="https://img.shields.io/sonar/coverage/egecetin_Repo-Init?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarqubecloud" alt="Coverage">
  </a>
  <a href="https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init">
    <img src="https://img.shields.io/sonar/quality_gate/egecetin_Repo-Init?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarqubecloud" alt="Quality Gate">
  </a>
  <a href="https://scorecard.dev/viewer/?uri=github.com/egecetin/Repo-Init">
    <img src="https://img.shields.io/ossf-scorecard/github.com/egecetin/Repo-Init?label=OpenSSF%20Score&style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCI+PHJlY3QgeD0iMyIgeT0iMTEiIHdpZHRoPSIxOCIgaGVpZ2h0PSIxMSIgcng9IjIiIHJ5PSIyIj48L3JlY3Q+PHBhdGggZD0ibTcgMTFWN0E1IDUgMCAwIDEgMTcgN3Y0Ij48L3BhdGg+PC9zdmc+" alt="OpenSSF Score">
  </a>
</p>

---

</div>

## What is Repo-Init?

**Repo-Init** is a powerful, production-ready CMake template designed to kickstart your C++ applications with enterprise-grade features built-in. Say goodbye to boilerplate code and hello to modern, robust software development!

> 🔧 **Requires**: C++17 supported compiler

> 💡 **Perfect for**: Microservices, CLI tools, system applications, and performance-critical software

<div align="center">

### **Key Features at a Glance**

</div>

<table align="center">
    <tr>
        <td align="center" width="33%">
            <h3>🛡️ <strong>Crash Protection</strong></h3>
            <p>Automatic minidump generation with <a
                    href="https://chromium.googlesource.com/crashpad/crashpad/"><strong>Crashpad</strong></a></p>
            <p><em>Never lose critical debugging information</em></p>
        </td>
        <td align="center" width="33%">
            <h3>📊 <strong>Advanced Logging</strong></h3>
            <p>Multi-output logging with <strong>Spdlog</strong></p>
            <p><em>File rotation • Syslog • Colorized console</em></p>
        </td>
        <td align="center" width="33%">
            <h3>🔗 <strong>Service Integration</strong></h3>
            <p><a href="https://sentry.io/"><strong>Sentry</strong></a> • <a
                    href="https://grafana.com/oss/loki/"><strong>Grafana Loki</strong></a> • <a
                    href="https://prometheus.io/"><strong>Prometheus</strong></a></p>
            <p><em>Real-time monitoring & alerting</em></p>
        </td>
    </tr>
    <tr>
        <td align="center">
            <h3>⚡ <strong>Performance Metrics</strong></h3>
            <ul>
                <li>Success/failure counters</li>
                <li>Min/max timing analysis</li>
                <li>Quantile-based bottleneck detection</li>
            </ul>
        </td>
        <td align="center">
            <h3>🌐 <strong>Network Ready</strong></h3>
            <ul>
                <li>Built-in Telnet server</li>
                <li><a href="https://zeromq.org/"><strong>ZeroMQ</strong></a> messaging</li>
                <li>HTTP connectivity</li>
            </ul>
        </td>
        <td align="center">
            <h3>🚀 <strong>Developer Experience</strong></h3>
            <ul>
                <li>One-command setup</li>
                <li>Comprehensive testing</li>
                <li>Auto-generated docs</li>
            </ul>
        </td>
    </tr>
</table>

---

> 💬 **Questions? Issues? Contributions?**
> Feel free to ask, use, and report any bugs you encounter! We ❤️ community feedback.

<div align="center">

[![SonarCloud](https://sonarcloud.io/images/project_badges/sonarcloud-orange.svg)](https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init)

---

</div>

## Table of Contents

- [What is Repo-Init?](#-what-is-repo-init)
- [Table of Contents](#-table-of-contents)
- [CMake Modules](#-cmake-modules)
- [Utility Scripts](#-utility-scripts)
- [Dependencies](#-dependencies)
- [Testing Setup](#-testing-setup)
- [Build Targets](#-build-targets)
- [Grafana Integration](#-grafana-integration)

## CMake Modules

Our carefully crafted CMake modules provide powerful build automation:

| Module | 🎯 Purpose | ✨ Benefits |
|--------|------------|-------------|
| **CodeCoverage** | Detects and enables `gcovr` | 📊 Automatic test coverage reports |
| **CompilerSecurityOptions** | Enables/Disables secure compiler flags | 🛡️ Hardened binary security |
| **Doxy** | Finds Doxygen package and prepares docs | 📖 Auto-generated documentation |
| **GenerateSymbols** | Adds target for symbol file generation | 🔍 Enhanced debugging with minidumps |
| **GitVersion** | Gets SHA1 hash of current commit | 🏷️ Version tracking and build reproducibility |
| **GraphViz** | Finds GraphViz and dot executable | 🎨 Visual dependency graphs |

---

## Utility Scripts

> 💡 **Tip**: All scripts should be executed from the top-level directory

| Script | 🚀 Function | 📝 Description |
|--------|-------------|----------------|
| `firstName.sh` | **Name Changer** | Replaces placeholder names throughout the project |
| `dump_syms.py` | **Symbol Dumper** | Generates symbol files for crash analysis |

---

## Dependencies

<div align="center">

### **Core Runtime Libraries**

</div>

<table>
    <tr>
        <td width="50%">
            <h4>🔥 <strong>Integrated Dependencies</strong></h4>
            <p><em>Built and bundled automatically</em></p>
            <ul>
                <li>🔌 <a href="https://github.com/zeromq/cppzmq.git"><strong>CppZMQ</strong></a> - Modern C++ ZeroMQ
                    bindings</li>
                <li>💥 <a href="https://chromium.googlesource.com/crashpad/crashpad/"><strong>Crashpad</strong></a> -
                    Crash reporting system</li>
                <li>📅 <a href="https://github.com/HowardHinnant/date.git"><strong>Date</strong></a> - C++ date/time
                    library</li>
                <li>🌐 <a
                        href="https://github.com/j-ulrich/http-status-codes-cpp.git"><strong>Http-status-codes</strong></a>
                    - HTTP utilities</li>
                <li>📊 <a href="https://github.com/jupp0r/prometheus-cpp.git"><strong>Prometheus-cpp</strong></a> -
                    Metrics collection</li>
                <li>⚡ <a href="https://github.com/Tencent/rapidjson.git"><strong>RapidJSON</strong></a> - Ultra-fast
                    JSON parser</li>
                <li>🔐 <a href="https://github.com/getsentry/sentry-native.git"><strong>Sentry</strong></a> - Error
                    monitoring</li>
                <li>🖥️ <a href="https://github.com/lukemalcolm/TelnetServLib.git"><strong>TelnetServLib</strong></a> -
                    <em>Modified & embedded</em></li>
            </ul>
        </td>
        <td width="50%">
            <h4>🛠️ <strong>Development Dependencies</strong></h4>
            <p><em>For building and testing only</em></p>
            <ul>
                <li>🔧 <a href="https://chromium.googlesource.com/breakpad/breakpad/"><strong>Breakpad</strong></a> -
                    Symbol dumping</li>
                <li>✅ <a href="https://github.com/google/googletest.git"><strong>GoogleTest</strong></a> - Unit testing
                    framework</li>
                <li>🔍 <a href="https://github.com/seladb/MemPlumber.git"><strong>MemPlumber</strong></a> - Memory leak
                    detection</li>
                <li>📦 <strong>ZLIB</strong> - Compression (required by Breakpad)</li>
            </ul>
            <h4>🌐 <strong>System Dependencies</strong></h4>
            <p><em>Install via package manager (apt/dnf/brew)</em></p>
            <ul>
                <li>🌍 <a href="https://github.com/curl/curl"><strong>cURL</strong></a> - HTTP client library</li>
                <li>📝 <a href="https://github.com/gabime/spdlog.git"><strong>Spdlog</strong></a> - Fast logging library
                </li>
                <li>⚡ <a href="https://github.com/zeromq/libzmq.git"><strong>ZeroMQ</strong></a> - High-performance
                    messaging</li>
            </ul>
        </td>
    </tr>
</table>

<div align="center">

> 📊 **Want to see the full picture?** Check out our complete [**dependency graph**](doc/dependency-tree.svg)!

---

</div>

## Testing Setup

<div align="center">

### **Python Test Dependencies**

</div>

Our test suite requires some Python dependencies for comprehensive testing. Here's how to set them up:

<table>
    <tr>
        <td width="50%">
            <h4>🚀 <strong>Quick Setup</strong></h4>
            <pre><code class="bash">
    # Create virtual environment
    python3 -m venv .venv
    <br>
    # Activate virtual environment
    source .venv/bin/activate  # Linux/macOS
    # OR
    .venv\Scripts\activate     # Windows
    <br>
    # Install test dependencies
    pip install -r tests/data/requirements.txt</code></pre>
        </td>
        <td width="50%">
            <h4>📋 <strong>Required Dependencies</strong></h4>
            <ul>
                <li><strong>pyzmq</strong> - Python ZeroMQ bindings for testing messaging functionality</li>
            </ul>
            <h4>⚠️ <strong>Important Notes</strong></h4>
            <ul>
                <li>Virtual environment <strong>must be activated</strong> before running tests</li>
                <li>Dependencies are automatically detected by the test suite</li>
                <li>Deactivate with <code>deactivate</code> when done</li>
            </ul>
        </td>
    </tr>
</table>

---

## Build Targets

<div align="center">

### **One Command, Multiple Possibilities**

</div>

| Target | 🚀 Command | 📋 Description |
|--------|------------|----------------|
| **all** | `cmake --build .` | Builds the complete project with all components |
| **coverage** | `cmake --build . --target coverage` | Generates comprehensive test coverage reports |
| **docs** | `cmake --build . --target docs` | Creates beautiful documentation with Doxygen |
| **dependency-graph** | `cmake --build . --target dependency-graph` | Visualizes project dependencies with GraphViz |
| **package** | `cmake --build . --target package` | Creates distribution packages (DEB/RPM + systemd service) |
| **test** | `ctest . --parallel` | Runs the complete test suite with GoogleTest |

<div align="center">

> 💡 **Pro Tips**:
> - For packages, specify your preferred format with `-DCPACK_GENERATOR="DEB"` or `"RPM"`
> - Ensure Python virtual environment is activated before running tests!

---

</div>

### **Build Options**

<div align="center">

**Customize your build with powerful CMake configuration options**

</div>

> 💡 **Important**: Re-run CMake configuration (`cmake -B build`) after changing any options

<table>
    <tr>
        <td width="60%">
            <h4>🧪 <strong>Testing & Quality Assurance</strong></h4>
            <table>
                <tr>
                    <th>Option</th>
                    <th>Description</th>
                    <th>Default</th>
                    <th>🎯 Use Case</th>
                </tr>
                <tr>
                    <td><code>XXX_BUILD_TESTS</code></td>
                    <td>🔧 Build all test suites</td>
                    <td><code>ON</code></td>
                    <td>Complete testing pipeline</td>
                </tr>
                <tr>
                    <td><code>XXX_BUILD_UNITTESTS</code></td>
                    <td>✅ Build unit tests only</td>
                    <td><code>ON</code></td>
                    <td>Fast development feedback</td>
                </tr>
                <tr>
                    <td><code>XXX_BUILD_FUZZTESTS</code></td>
                    <td>🎲 Build fuzz testing suite</td>
                    <td><code>OFF</code></td>
                    <td>Security & robustness testing</td>
                </tr>
                <tr>
                    <td><code>XXX_ENABLE_COVERAGE</code></td>
                    <td>📊 Generate test coverage reports</td>
                    <td><code>OFF</code></td>
                    <td>Code quality metrics</td>
                </tr>
                <tr>
                    <td><code>XXX_ENABLE_MEMLEAK_CHECK</code></td>
                    <td>🔍 Memory leak detection with MemPlumber</td>
                    <td><code>OFF</code></td>
                    <td>Debug memory issues</td>
                </tr>
            </table>
            <h4>🚀 <strong>Release & Distribution</strong></h4>
            <table>
                <tr>
                    <th>Option</th>
                    <th>Description</th>
                    <th>Default</th>
                    <th>🎯 Use Case</th>
                </tr>
                <tr>
                    <td><code>XXX_ENABLE_SYMBOL_GENERATION</code></td>
                    <td>🔧 Generate debug symbols for crash dumps</td>
                    <td><code>OFF</code></td>
                    <td>Production debugging</td>
                </tr>
                <tr>
                    <td><code>XXX_ENABLE_PACKAGING</code></td>
                    <td>📦 Enable DEB/RPM packaging with systemd</td>
                    <td><code>OFF</code></td>
                    <td>Distribution & deployment</td>
                </tr>
            </table>
        </td>
        <td width="40%">
            <h4>💡 <strong>Quick Examples</strong></h4>
            <pre><code class="bash"># Development build with all tests
cmake -B build -DXXX_BUILD_TESTS=ON \
                -DXXX_ENABLE_COVERAGE=ON
<br>
# Production build with packaging
cmake -B build -DXXX_BUILD_TESTS=OFF \
                -DXXX_ENABLE_PACKAGING=ON \
                -DXXX_ENABLE_SYMBOL_GENERATION=ON
<br>
# Security testing build
cmake -B build -DXXX_BUILD_FUZZTESTS=ON \
                -DXXX_ENABLE_MEMLEAK_CHECK=ON
<br>
# Minimal build (fastest)
cmake -B build -DXXX_BUILD_TESTS=OFF</code></pre>
            <h4>⚡ <strong>Pro Tips</strong></h4>
            <ul>
                <li>🏃 <strong>Fast Iteration</strong>: Disable tests for quick builds during development</li>
                <li>🔒 <strong>Security Focus</strong>: Enable fuzz tests and memory checks for critical code</li>
                <li>📈 <strong>CI/CD</strong>: Use coverage reports in your automated pipelines</li>
                <li>🚀 <strong>Production</strong>: Always enable symbol generation for crash analysis</li>
            </ul>
        </td>
    </tr>
</table>


## Grafana Integration

<div align="center">

### **Real-Time Monitoring Made Beautiful**

Thanks to our integrated **Prometheus server**, monitoring your application has never been easier! Get instant insights into your application's performance, health, and behavior.

</div>

<table>
    <tr>
        <td width="50%">
            <h4>🚀 <strong>Quick Setup</strong></h4>
            <ol>
                <li><strong>Import Dashboard</strong>: Use our pre-built <a
                        href="scripts/GrafanaDashboard-1730032129887.json"><strong>Grafana template</strong></a></li>
                <li><strong>Connect Prometheus</strong>: Point to your app's metrics endpoint</li>
                <li><strong>Monitor</strong>: Watch real-time metrics flow in!</li>
            </ol>
            <h4>✨ <strong>What You Get</strong></h4>
            <ul>
                <li><strong>Performance Metrics</strong>: Response times, throughput, resource usage</li>
                <li><strong>Error Tracking</strong>: Real-time error rates and alerting</li>
                <li><strong>Resource Monitoring</strong>: CPU, memory, and system metrics</li>
                <li><strong>Custom Metrics</strong>: Track your application-specific KPIs</li>
            </ul>
        </td>
        <td width="50%">
            <div align="center">
                <h4>📸 <strong>Live Dashboard Preview</strong></h4>
                <img src="doc/GrafanaDashboard.png" alt="Grafana Dashboard" width="100%">
                <p><em>Beautiful, responsive, and information-rich monitoring</em></p>
            </div>
        </td>
    </tr>
</table>

<div align="center">

---

## **Contributing & Support**

<p>
  <a href="https://github.com/egecetin/Repo-Init/issues">🐛 Report Issues</a> •
  <a href="https://github.com/egecetin/Repo-Init/discussions">💬 Discussions</a> •
  <a href="https://github.com/egecetin/Repo-Init/pulls">🔧 Pull Requests</a>
</p>

**Made with ❤️ for the C++ community**

*Star ⭐ this repo if you find it useful!*

---

</div>
