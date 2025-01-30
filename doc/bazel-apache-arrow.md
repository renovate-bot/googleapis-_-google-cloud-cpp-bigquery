# Building `google-cloud-cpp-bigquery` with Bazel

As [Apache Arrow][apache-arrow] does not provide native bazel support, nor does
the [Bazel Central Registry currently contain a module](https://github.com/bazelbuild/bazel-central-registry/issues/3344)
for it, building this library with bazel requires some extra steps.

If Apache Arrow is not already installed, you can likely obtain it from your
preferred package manager, or it can be built from source and installed via:

```
curl -fsSL https://github.com/apache/arrow/archive/apache-arrow-18.1.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -GNinja -S cpp -B cmake-out \
      --preset ninja-release-minimal \
      -DARROW_JEMALLOC=OFF \
      -DARROW_BUILD_STATIC=ON  && \
    cmake --build cmake-out --target install
```

First, we have to rely on `WORKSPACE.bazel` files and cannot take advantage of
bazel modules.

In your `WORKSPACE.bazel` file, you will need to add:

```
native.new_local_repository(
    name = "libarrow",
    path = "/usr/local/",
    build_file = "//bazel:arrow.BUILD",
)
```

And provide a minimal `arrow.BUILD` file:

```
cc_import(
    name = "libarrow",
    hdrs = glob(["include/arrow/*.h"]),
    interface_library = "lib64/libarrow.so",
    static_library = "lib64/libarrow.a",
    system_provided = True,
    visibility = ["//visibility:public"],
)
```

Take note of the `include` and `lib` paths the `WORKSPACE.bazel` and
`arrow.BUILD` files specify as these are the locations bazel expects to find the
installed artifacts. If your system does not install the headers and libraries
to these locations, you may need to create symbolic links:

```
mkdir -p /usr/local/lib64
ln -s $(find /usr -name libarrow.a) /usr/local/lib64/libarrow.a
ln -s $(find /usr -name libarrow.so) /usr/local/lib64/libarrow.so
ln -s $(find /usr/include -name arrow -type d) /usr/local/include/arrow

export LD_LIBRARY_PATH=/usr/local/lib64:${LD_LIBRARY_PATH}
```

[apache-arrow]: https://github.com/apache/arrow
