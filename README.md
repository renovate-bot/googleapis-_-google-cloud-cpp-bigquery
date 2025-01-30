# Google Cloud Platform C++ BigQuery Client Library

[![GCB CI status][gcb-clang-tidy-shield]][gcb-clang-tidy-link]
[![GCB CI status][gcb-asan-shield]][gcb-asan-link]
[![GHA][gha-shield]][gha-link]
[![Codecov Coverage status][codecov-shield]][codecov-link]

This repository contains a C++ library that integrates several of the
[Google Cloud Platform BigQuery](https://cloud.google.com/bigquery) services.

The goal of this library is to provide convenient ways to perform common
BigQuery tasks colocated in a single library. The available methods are intended
to encapsulate boilerplate code, provide reasonable defaults for operations, and
implement BigQuery service best practices.

This library does not attempt to replicate all the RPCs available in the [Cloud
C++ BigQuery libraries][google-cloud-cpp-bigquery-libraries].

## Dependencies

In addition to the [Cloud C++ BigQuery libraries][google-cloud-cpp-bigquery-libraries]
and its transitive dependencies, this library depends on [Apache Arrow][apache-arrow]
as the data format for reading BigQuery data. This introduces some additional
complexities when using bazel as Apache Arrow does not provide bazel support. It
is necessary to [install Apache Arrow on the target system and instruct bazel to
look for it there](/doc/bazel-apache-arrow.md).

> Please check the [CHANGELOG] for important announcements and upcoming changes.

## Quickstart

Located in the directory named `google/cloud/bigquery_unified/quickstart/` is a
trivial C++ program intended to help you get up and running in a matter of
minutes. This `quickstart/` directory contains a minimal "Hello World" program
demonstrating how to use the library, along with minimal build files for common
build systems, such as CMake and Bazel.

<!-- inject-quickstart-start -->

```cc
#include "google/cloud/bigquery_unified/client.h"
#include <iostream>

int main(int argc, char* argv[]) try {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " project-id\n";
    return 1;
  }

  auto const project_id = argv[1];

  namespace bigquery = ::google::cloud::bigquery_unified;
  namespace bigquery_v2_proto = ::google::cloud::bigquery::v2;
  auto client = bigquery::Client(bigquery::MakeConnection());

  bigquery_v2_proto::ListJobsRequest list_request;
  list_request.set_project_id(project_id);

  for (auto r : client.ListJobs(list_request)) {
    if (!r) throw std::move(r).status();
    std::cout << r->job_reference().project_id() << "/"
              << r->job_reference().job_id() << "\n";
  }

  return 0;
} catch (google::cloud::Status const& status) {
  std::cerr << "google::cloud::Status thrown: " << status << "\n";
  return 1;
}
```

<!-- inject-quickstart-end -->

## Building and Installing

This is a quickstart guide for developers wanting to compile the libraries and
run the examples included with the libraries.

- Packaging maintainers or developers who prefer to install the library in a
  fixed directory (such as `/usr/local` or `/opt`) should consult the
  [packaging guide](/doc/packaging.md).
- Developers who prefer using a package manager such as
  [vcpkg](https://vcpkg.io), or [Conda](https://conda.io), should follow the
  instructions for their package manager.
- Developers wanting to compile the library just to run some examples or tests
  should read the current document.
- Contributors and developers to `google-cloud-cpp-bigquery` should consult the guide to
  [set up a development workstation][howto-setup-dev-workstation].

### Building with Bazel

This library requires Bazel >= 7.0. [Apache Arrow][apache-arrow] must also
already be installed on the system.

From the top-level directory, run the usual commands.

```shell
bazel build //...
```

### Building with CMake

This library requires CMake >= 3.16. If you are planning to install the
libraries please consult the [packaging guide](/doc/packaging.md), these
instructions will **NOT** produce artifacts that you can put in `/usr/local`, or
share with your colleagues.

From the top-level directory of `google-cloud-cpp-bigquery` run these commands:

```shell
git -C $HOME clone https://github.com/microsoft/vcpkg.git
env VCPKG_ROOT=$HOME/vcpkg $HOME/vcpkg/bootstrap-vcpkg.sh
cmake -S . -B cmake-out/ -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build cmake-out -- -j $(nproc)
```

The binary artifacts, such as examples, will be placed in `cmake-out/`.

## Support

- This project follows Google's
  [Foundational C++ Support Policy][support-policy], which is summarized in the
  [Foundational C++ Support Matrix][support-matrix].
  - Language Version (>= C++17).
  - Operating Systems: Windows, macOS, and Linux.
  - Build Systems: Bazel (>= 7.0), CMake (>= 3.16).
  - Compilers: GCC (>= 7.5), Clang (>= 7.0), MSVC (>= 2022), Apple Clang (>=
    12).
- This project uses dependencies described in
  [doc/packaging.md](https://github.com/googleapis/google-cloud-cpp-bigquery/blob/main/doc/packaging.md).
- This project works with or without exceptions enabled.
- This project cuts
  [monthly releases](https://github.com/googleapis/google-cloud-cpp-bigquery/releases)
  with detailed release notes.

## Public API and API Breaking Changes

This project follows Google's [OSS Library Breaking Change Policy].

In general, we avoid making backwards incompatible changes to our
[public API](/doc/public-api.md). Sometimes such changes yield benefits to our
customers, in the form of better performance, easier-to-understand APIs, and/or
more consistent APIs across services. When these benefits warrant it, we will:

- Announce these changes prominently in our `CHANGELOG.md` file and in the
  affected release's notes.
- Increase the major version for `google-cloud-cpp-bigquery`.

Nevertheless, though we take commercially reasonable efforts to prevent this, it
is possible that backwards incompatible changes go undetected and, therefore,
undocumented. We apologize if this is the case and welcome feedback or bug
reports to rectify the problem.

## Contact us

- [GitHub Discussions] -- For questions and general comments
- [GitHub Issues] -- For reporting bugs and feature requests

## Contributing changes

See [`CONTRIBUTING.md`](/CONTRIBUTING.md) for details on how to contribute to
this project, including how to build and test your changes as well as how to
properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](/LICENSE) for details.

[apache-arrow]: https://github.com/apache/arrow
[changelog]: /CHANGELOG.md
[codecov-link]: https://codecov.io/gh/googleapis/google-cloud-cpp-bigquery
[codecov-shield]: https://codecov.io/gh/googleapis/google-cloud-cpp-bigquery/branch/main/graph/badge.svg
[gcb-asan-link]: https://storage.googleapis.com/cloud-cpp-community-publiclogs/badges/google-cloud-cpp-bigquery/main/asan.html
[gcb-asan-shield]: https://storage.googleapis.com/cloud-cpp-community-publiclogs/badges/google-cloud-cpp-bigquery/main/asan.svg
[gcb-clang-tidy-link]: https://storage.googleapis.com/cloud-cpp-community-publiclogs/badges/google-cloud-cpp-bigquery/main/clang-tidy.html
[gcb-clang-tidy-shield]: https://storage.googleapis.com/cloud-cpp-community-publiclogs/badges/google-cloud-cpp-bigquery/main/clang-tidy.svg
[gha-link]: https://github.com/googleapis/google-cloud-cpp-bigquery/actions/workflows/test-runner.yml
[gha-shield]: https://github.com/googleapis/google-cloud-cpp-bigquery/actions/workflows/test-runner.yml/badge.svg
[github discussions]: https://github.com/googleapis/google-cloud-cpp-bigquery/discussions
[github issues]: https://github.com/googleapis/google-cloud-cpp-bigquery/issues
[google-cloud-cpp-bigquery-libraries]: https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/bigquery
[howto-setup-dev-workstation]: /doc/contributor/howto-guide-setup-development-workstation.md
[oss library breaking change policy]: https://opensource.google/documentation/policies/library-breaking-change
[support-matrix]: https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md
[support-policy]: https://opensource.google/documentation/policies/cplusplus-support
