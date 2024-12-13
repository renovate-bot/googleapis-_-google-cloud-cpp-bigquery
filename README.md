Under Construction

<!-- inject-quickstart-start -->

```cc
#include "google/cloud/bigquery_unified/client.h"
#include <iostream>

int main(int argc, char* argv[]) {
  auto connection = google::cloud::bigquery_unified::MakeConnection();
  std::cout << "side effect to try and fool the optimizer: " << connection.get()
            << "\n";
  std::cout << "Hello, world!\n";
}
```

<!-- inject-quickstart-end -->
