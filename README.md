Under Construction

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
