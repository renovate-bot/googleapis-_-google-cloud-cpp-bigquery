// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigquery_unified/client.h"
#include "google/cloud/bigquery_unified/job_options.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/log.h"
#include "google/cloud/project.h"
#include "absl/time/time.h"
#include <iostream>

namespace {

void GetJob(google::cloud::bigquery_unified::Client client,
            std::vector<std::string> const& argv) {
  //! [START bigquery_get_job] [bigquery-get-job]
  [](google::cloud::bigquery_unified::Client client, std::string project_id,
     std::string job_id) {
    google::cloud::bigquery::v2::GetJobRequest request;
    request.set_project_id(project_id);
    request.set_job_id(job_id);
    auto job = client.GetJob(request);
    if (!job) throw std::move(job).status();
    std::cout << "Job " << job_id << " exists and its metadata is:\n"
              << job->DebugString();
  }
  //! [END bigquery_get_job] [bigquery-get-job]
  (client, argv[0], argv[1]);
}

void InsertJob(google::cloud::bigquery_unified::Client client,
               std::vector<std::string> const& argv) {
  //! [START bigquery_create_job] [bigquery-create-job]
  [](google::cloud::bigquery_unified::Client client, std::string project_id,
     std::string query_text) {
    google::cloud::bigquery::v2::JobConfigurationQuery query;
    query.mutable_use_legacy_sql()->set_value(false);
    query.set_query(std::move(query_text));
    google::cloud::bigquery::v2::JobConfiguration config;
    *config.mutable_query() = query;
    config.mutable_labels()->insert({"test_suite", "job_samples"});
    config.mutable_labels()->insert({"test_case", "insert_job"});

    google::cloud::bigquery::v2::Job job;
    *job.mutable_configuration() = config;
    auto options =
        google::cloud::Options{}
            .set<google::cloud::bigquery_unified::BillingProjectOption>(
                project_id);

    auto insert_job = client.InsertJob(job, options).get();
    if (!insert_job) throw std::move(insert_job).status();
    std::cout << "Job " << insert_job->job_reference().job_id()
              << " is inserted and its metadata is:\n"
              << insert_job->DebugString();
  }
  //! [END bigquery_create_job] [bigquery-create-job]
  (client, argv[0], argv[1]);
}

void CancelJob(google::cloud::bigquery_unified::Client client,
               std::vector<std::string> const& argv) {
  //! [START bigquery_cancel_job] [bigquery-cancel-job]
  [](google::cloud::bigquery_unified::Client client, std::string project_id,
     std::string job_id, std::string job_location) {
    google::cloud::bigquery::v2::CancelJobRequest cancel_request;
    cancel_request.set_project_id(project_id);
    cancel_request.set_job_id(job_id);
    cancel_request.set_location(job_location);
    auto cancel_job = client.CancelJob(cancel_request).get();
    if (!cancel_job) throw std::move(cancel_job).status();
    std::cout << "Job " << job_id << " is cancelled.\n";
  }
  //! [END bigquery_cancel_job] [bigquery-cancel-job]
  (client, argv[0], argv[1], argv[2]);
}

void ListJobs(google::cloud::bigquery_unified::Client client,
              std::vector<std::string> const& argv) {
  //! [START bigquery_list_jobs] [bigquery-list-jobs]
  [](google::cloud::bigquery_unified::Client client, std::string project_id) {
    google::cloud::bigquery::v2::ListJobsRequest request;
    request.set_project_id(project_id);
    auto list_jobs = client.ListJobs(request);
    int count = 0;
    for (auto const& job : list_jobs) {
      if (!job) throw std::move(job).status();
      count += 1;
    }
    std::cout << count << " jobs are returned by ListJobs.\n";
  }
  //! [END bigquery_list_jobs] [bigquery-list-jobs]
  (client, argv[0]);
}

void DeleteJob(google::cloud::bigquery_unified::Client client,
               std::vector<std::string> const& argv) {
  //! [START bigquery_delete_job] [bigquery-delete-job]
  [](google::cloud::bigquery_unified::Client client, std::string project_id,
     std::string job_id, std::string job_location) {
    google::cloud::bigquery::v2::DeleteJobRequest delete_request;
    delete_request.set_project_id(project_id);
    delete_request.set_job_id(job_id);
    delete_request.set_location(job_location);
    auto delete_job = client.DeleteJob(delete_request);
    if (!delete_job.ok()) throw std::move(delete_job);
    std::cout << "Job " << job_id << " is deleted.\n";
  }
  //! [END bigquery_delete_job] [bigquery-delete-job]
  (client, argv[0], argv[1], argv[2]);
}

google::cloud::bigquery_unified::Client MakeSampleClient() {
  return google::cloud::bigquery_unified::Client(
      google::cloud::bigquery_unified::MakeConnection());
}

std::string Basename(absl::string_view name) {
  auto last_sep = name.find_last_of("/\\");
  if (last_sep != absl::string_view::npos) name.remove_prefix(last_sep + 1);
  return std::string(name);
}

int RunOneCommand(std::vector<std::string> argv) {
  using CommandType = std::function<void(std::vector<std::string> const&)>;
  using SampleFunction = void (*)(google::cloud::bigquery_unified::Client,
                                  std::vector<std::string> const&);
  using CommandMap = std::map<std::string, CommandType>;

  auto make_command_entry = [](std::string const& sample_name,
                               SampleFunction sample, std::size_t argc,
                               std::string const& usage) {
    auto make_command = [](std::string const& sample_name,
                           SampleFunction sample, std::size_t argc,
                           std::string const& usage) {
      return [sample_name, sample, argc,
              usage](std::vector<std::string> const& argv) {
        if (argv.size() != argc) {
          throw std::runtime_error(sample_name + usage);
        }
        sample(MakeSampleClient(), argv);
      };
    };
    return CommandMap::value_type(
        sample_name, make_command(sample_name, sample, argc, usage));
  };

  CommandMap commands = {
      make_command_entry("bigquery-get-job", GetJob, 2,
                         " <project_id> <job_id>"),
      make_command_entry("bigquery-cancel-job", CancelJob, 3,
                         " <project_id> <job_id> <job_location>"),
      make_command_entry("bigquery-insert-job", InsertJob, 2,
                         " <project_id> <query_text>"),
      make_command_entry("bigquery-list-jobs", ListJobs, 1, " <project_id>"),
      make_command_entry("bigquery-delete-job", DeleteJob, 3,
                         " <project_id> <job_id> <location>")};

  static std::string usage_msg = [&argv, &commands] {
    std::string usage;
    usage += "Usage: ";
    usage += Basename(argv[0]);
    usage += " <command> [arguments]\n\n";
    usage += "Commands:\n";
    for (auto&& kv : commands) {
      try {
        kv.second({});
      } catch (std::exception const& ex) {
        usage += "    ";
        usage += ex.what();
        usage += "\n";
      }
    }
    return usage;
  }();

  if (argv.size() < 2) {
    std::cerr << "Missing command argument\n" << usage_msg << "\n";
    return 1;
  }
  std::string command_name = argv[1];
  argv.erase(argv.begin());  // remove the program name from the list.
  argv.erase(argv.begin());  // remove the command name from the list.

  auto command = commands.find(command_name);
  if (commands.end() == command) {
    std::cerr << "Unknown command " << command_name << "\n"
              << usage_msg << "\n";
    return 1;
  }

  // Run the command.
  command->second(std::move(argv));
  return 0;
}  // NOLINT [clang-analyzer-cplusplus.NewDeleteLeaks]

bool AutoRun() {
  return google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES")
             .value_or("") == "yes";
}

void SampleBanner(std::string const& name) {
  std::cout << "\nRunning " << name << " sample at "
            << absl::FormatTime("%Y-%m-%dT%H:%M:%SZ", absl::Now(),
                                absl::UTCTimeZone())
            << std::endl;
  GCP_LOG(DEBUG) << "Running " << name << " sample";
}

void RunAll() {
  auto const project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  if (project_id.empty()) {
    throw std::runtime_error("GOOGLE_CLOUD_PROJECT is not set or is empty");
  }

  auto client = MakeSampleClient();

  SampleBanner("bigquery-insert-job");
  auto query_text =
      "SELECT name, state, year, sum(number) as total "
      "FROM `bigquery-public-data.usa_names.usa_1910_2013` "
      "WHERE year >= 1996 "
      "GROUP BY name, state, year ";
  InsertJob(client, {project_id, query_text});

  google::cloud::bigquery::v2::ListJobsRequest list_jobs_request;
  list_jobs_request.set_project_id(project_id);
  std::string job_id;
  std::string job_location;
  for (auto const& j : client.ListJobs(list_jobs_request)) {
    if (!j) throw std::move(j).status();
    job_id = j->job_reference().job_id();
    job_location = j->job_reference().location().value();
    break;
  }

  SampleBanner("bigquery-get-job");
  GetJob(client, {project_id, job_id});

  SampleBanner("bigquery-cancel-job");
  CancelJob(client, {project_id, job_id, job_location});

  SampleBanner("bigquery-list-jobs");
  ListJobs(client, {project_id});

  SampleBanner("bigquery-delete-job");
  DeleteJob(client, {project_id, job_id, job_location});

  // delete the jobs whose creation time > 7 days and labeled "job_samples"
  google::cloud::bigquery::v2::ListJobsRequest list_old_jobs_request;
  list_old_jobs_request.set_project_id(project_id);
  list_old_jobs_request.set_projection(
      google::cloud::bigquery::v2::ListJobsRequest::FULL);
  google::protobuf::UInt64Value t;
  t.set_value(absl::ToUnixMillis(absl::Now() - absl::Hours(24 * 7)));
  *list_old_jobs_request.mutable_max_creation_time() = t;

  for (auto const& j : client.ListJobs(list_old_jobs_request)) {
    if (!j) throw std::move(j).status();
    auto const& labels = j->configuration().labels();
    if (labels.find("test_suite") == labels.end()) continue;
    if (labels.at("test_suite") != "job_samples") continue;
    auto old_job_id = j->job_reference().job_id();

    google::cloud::bigquery::v2::DeleteJobRequest delete_request;
    delete_request.set_project_id(project_id);
    delete_request.set_job_id(old_job_id);
    auto delete_job = client.DeleteJob(delete_request);
    if (!delete_job.ok()) {
      std::cerr << "Failed to clean up Job " << old_job_id << ".\n";
    }
  }
}

}  // namespace

int main(int ac, char* av[]) try {
  if (AutoRun()) {
    RunAll();
    return 0;
  }
  return RunOneCommand({av, av + ac});
} catch (google::cloud::Status const& status) {
  std::cerr << status << "\n";
  google::cloud::LogSink::Instance().Flush();
  return 1;
} catch (std::exception const& ex) {
  std::cerr << ex.what() << "\n";
  return 1;
}
