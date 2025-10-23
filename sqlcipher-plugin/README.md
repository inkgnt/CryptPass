# qt-sqlcipher-plugin

**SQLCipher/SQLite Plugin for Qt 6 Projects**

This project provides the necessary files and build instructions to compile a custom SQLCipher plugin for use with Qt 6 applications, enabling encrypted database support.

It is based on original `CMakeLists.txt` files from the Qt source code (`sqldrivers` and `sqlite3` folders) with minimal modifications.

---

## Prerequisites

All necessary source amalgamations and pre-compiled libraries are included in this repository for convenience. If you require different versions or architectures, you will need to replace the contents of the respective folders and potentially modify the `CMakeLists.txt` files.

1.  **SQLCipher/SQLite Amalgamation:**
    *   Source amalgamations obtained from SQLCipher v4.11.0 are already provided in the [sqlcipher_am](sqlcipher_am) folder.

2.  **OpenSSL Static Libraries:**
    *   Pre-compiled static OpenSSL v3.6.0 libraries are included in the [openssl](openssl) folder.

---

## Building the Plugin

These instructions use the Ninja generator. You should install this tool before proceeding, as it is required to build the project.

### Step 1: Prepare the Environment

1.  Open the **x64 Native Tools Command Prompt for VS 2022**. This ensures your environment variables are correctly set for a 64-bit build.
2.  Navigate to your desired project location and clone the repository:

    ```bash
    git clone https://github.com/inkgnt/qsqlcipher-plugin.git
    ```

3.  Navigate into the project folder and create a dedicated build directory:

    ```bash
    cd qsqlcipher-plugin
    mkdir .build
    cd .build
    ```

### Step 2: Configure the Build

Run CMake to configure the project. Be sure to **replace the paths** in the command below to match your specific Qt installation location. Use forward slashes (`/`) for paths.

The flag `-DCMAKE_BUILD_TYPE=Release` is used to force a Release build type. If you prefer a different configuration, you can use any other valid build type, such as Debug, RelWithDebInfo, or MinSizeRel, in place of Release.

```bash
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../.install -DQt6_DIR=c:/Qt/6.7.3/msvc2022_64/lib/cmake/Qt6
cmake --build .
cmake --install .
```
In the .install folder, you will find your version of the plugin. Simply copy these files to the sqldrivers folder in your Qt6 installation and/or your project. Once done, your plugin will be ready to use.

---

## Licensing

This project is distributed under the **[MIT License](LICENSE)**.

It incorporates the following third-party libraries, which are distributed under their own licenses:

*   **Qt 6:** LGPL v3.0
*   **OpenSSL:** Apache License 2.0
*   **SQLCipher Community Edition:** BSD License

Full copyright and license notices for these dependencies can be found in the **[`NOTICE`](NOTICE)** file.
