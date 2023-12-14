# Parcook

Computation on Parquet data.

## Getting started

Parcook is built with CMake and depends on [Apache Arrow](https://arrow.apache.org). To install Arrow, we suggest [vcpkg](https://vcpkg.io/en/index.html). The recommended steps to build Parcook are as follows.

1. If you don't already have a vcpkg instance, clone and bootstrap vcpkg. Replace `$VCPKG_ROOT` with your desired vcpkg instance location.

    ```shell
    git clone https://github.com/Microsoft/vcpkg.git $VCPKG_ROOT
    $VCPKG_ROOT/bootstrap-vcpkg.sh
    ```

2. Create and navigate into a build directory.

    ```shell
    mkdir build
    cd build
    ```

3. Configure with CMake. Replace `$VCPKG_ROOT` with the path to your vcpkg instance. This step may take several minutes.

    ```shell
    cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    ```

4. Build with CMake.

    ```shell
    cmake --build .
    ```

5. Optionally, run the tests with CTest.

   ```shell
   ctest .
   ```

If you use an IDE, we suggest using the appropriate utilities to facilitate the above steps.

- In Visual Studio Code, add the following to your workspace `settings.json` file. Replace `$VCPKG_ROOT` with the path to your vcpkg instance.

   ```json
   {
     "cmake.configureSettings": {
       "CMAKE_TOOLCHAIN_FILE": "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
     }
   }
   ```

- In CLion, navigate to _Settings_ / _Build, Execution, and Deployment_ / _CMake_. To _CMake options_, add `-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`. Replace `$VCPKG_ROOT` with the path to your vcpkg instance.
