﻿// project-aws.cpp : Defines the entry point for the application.
//

//#include "project-aws.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <iostream>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <fstream>
#include <sys/stat.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/Object.h>

using namespace Aws;
using namespace Aws::Auth;

/*
 *  A "Hello S3" starter application which initializes an Amazon Simple Storage Service (Amazon S3) client
 *  and lists the Amazon S3 buckets in the selected region.
 */

 /**
 * Check if file exists
 *
 * Note: If using C++17, can use std::filesystem::exists()
 */
inline bool file_exists(const std::string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

 /**
 * Put an object into an Amazon S3 bucket
 */
bool put_s3_object(const Aws::String& s3_bucket_name,
    const Aws::String& s3_object_name,
    const std::string& file_name,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Verify file_name exists
    if (!file_exists(file_name)) {
        std::cout << "ERROR: NoSuchFile: The specified file does not exist"
            << std::endl;
        //return false;
    }

    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    const std::shared_ptr<Aws::IOStream> input_data =
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
            file_name.c_str(),
            std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Put the object
    auto put_object_outcome = s3_client.PutObject(object_request);
    if (!put_object_outcome.IsSuccess()) {
        auto error = put_object_outcome.GetError();
        std::cout << "ERROR: " << error.GetExceptionName() << ": "
            << error.GetMessage() << std::endl;
        return false;
    }
    return true;
    // snippet-end:[s3.cpp.put_object.code]
}

bool ListObjects(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig) 
{
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::ListObjectsRequest request;
    request.WithBucket(bucketName);

    auto outcome = s3_client.ListObjects(request);

    if (!outcome.IsSuccess()) {
        std::cerr << "Error: ListObjects: " <<
            outcome.GetError().GetMessage() << std::endl;
    }
    else {
        Aws::Vector<Aws::S3::Model::Object> objects =
            outcome.GetResult().GetContents();

        for (Aws::S3::Model::Object& object : objects) {
            std::cout << object.GetKey() << std::endl;
        }
    }

    return outcome.IsSuccess();
}

bool GetObject(const Aws::String& objectKey,
    const Aws::String& fromBucket,
    const Aws::Client::ClientConfiguration& clientConfig) {
    Aws::S3::S3Client client(clientConfig);

    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(fromBucket);
    request.SetKey(objectKey);

    Aws::S3::Model::GetObjectOutcome outcome =
        client.GetObject(request);

    if (!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    else {
        std::cout << "Successfully retrieved '" << objectKey << "' from '"
            << fromBucket << "'." << std::endl;

        std::string local_file_name = "/root/out/" + objectKey;
        std::ofstream local_file(local_file_name, std::ios::binary);
        auto& retrieved = outcome.GetResult().GetBody();
        local_file << retrieved.rdbuf();
        std::cout << "Done!";
    }

    return outcome.IsSuccess();
}

int main(int argc, char** argv) {
    Aws::SDKOptions options;
    // Optionally change the log level for debugging.
//   options.loggingOptions.logLevel = Utils::Logging::LogLevel::Debug;
    Aws::InitAPI(options); // Should only be called once.


    int result = 0;
    {
        Aws::Client::ClientConfiguration clientConfig;
        // Optional: Set to the AWS Region (overrides config file).
        clientConfig.region = Region::AP_SOUTHEAST_1;
        clientConfig.scheme = Http::Scheme::HTTPS;

        clientConfig.proxyUserName = Aws::String("AKIAQVRKOSAV3GMXAQ32");
        clientConfig.proxyPassword = Aws::String("LvU7SjJUWXjYp2+d27oWy6cfn8ouY71012KCyIan");

        Aws::S3::S3Client s3_client(clientConfig);

        // Assign these values before running the program
        const Aws::String bucket_name = "myawsbucket-tl";
        const Aws::String object_name = "test01.txt";
        const std::string file_name = "/root/test01.txt";

        // Put the file into the S3 bucket
        if (put_s3_object(bucket_name, object_name, file_name, clientConfig)) {
            std::cout << "Put file " << file_name
                << " to S3 bucket " << bucket_name
                << " as object " << object_name << std::endl;
        }

        auto provider = Aws::MakeShared<DefaultAWSCredentialsProviderChain>("alloc-tag");
        auto creds = provider->GetAWSCredentials();
        if (creds.IsEmpty()) {
            std::cerr << "Failed authentication" << std::endl;
        }

        Aws::S3::S3Client s3Client(clientConfig);
        auto outcome = s3Client.ListBuckets();

        if (!outcome.IsSuccess()) {
            std::cerr << "Failed with error: " << outcome.GetError() << std::endl;
            result = 1;
        }
        else {
            std::cout << "Found " << outcome.GetResult().GetBuckets().size()
                << " buckets\n";
            for (auto& bucket : outcome.GetResult().GetBuckets()) {
                std::cout << bucket.GetName() << std::endl;
            }
        }

        ListObjects(bucket_name, clientConfig);

        GetObject(object_name, bucket_name, clientConfig);
        
    }

    Aws::ShutdownAPI(options); // Should only be called once.
    return result;
}