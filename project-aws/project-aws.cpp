// project-aws.cpp : Defines the entry point for the application.
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
 *  An AWS service class to encapsulate simple operations with S3 buckets and objects 
 */
class ServiceS3 {
    private:
        /**
        *  S3 Client instance 
        */
        Aws::S3::S3Client* s3_client;

        /**
        * Check if file exists
        */
        inline bool fileExists(const std::string& name)
        {
            struct stat buffer;
            return (stat(name.c_str(), &buffer) == 0);
        }

    public:

        /**
        * Constructor method
        */
        ServiceS3(const Aws::Client::ClientConfiguration& clientConfig)
        {
            s3_client = new Aws::S3::S3Client(clientConfig);
            
        }
         /**
         * Put an object into an Amazon S3 bucket
         */
        bool putObject(const Aws::String& bucketName,
            const Aws::String& objectName,
            const std::string& fileName)
        {
            // Verify fileName exists
            if (!fileExists(fileName)) {
                std::cout << "ERROR: NoSuchFile: The specified file does not exist"
                    << std::endl;
                //return false;
            }            

            Aws::S3::Model::PutObjectRequest objectRequest;

            objectRequest.SetBucket(bucketName);
            objectRequest.SetKey(objectName);
            const std::shared_ptr<Aws::IOStream> inputData =
                Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
                    fileName.c_str(),
                    std::ios_base::in | std::ios_base::binary);
            objectRequest.SetBody(inputData);

            // Put the object
            auto put_object_outcome = s3_client->PutObject(objectRequest);
            if (!put_object_outcome.IsSuccess()) {
                auto error = put_object_outcome.GetError();
                std::cout << "ERROR: " << error.GetExceptionName() << ": "
                    << error.GetMessage() << std::endl;
                return false;
            }
            return true;
            // snippet-end:[s3.cpp.put_object.code]
        }

        bool listObjects(const Aws::String& bucketName) 
        {
            Aws::S3::Model::ListObjectsRequest request;
            request.WithBucket(bucketName);

            auto outcome = s3_client->ListObjects(request);

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

        bool listBuckets()
        {
            auto outcome = s3_client->ListBuckets();

            if (!outcome.IsSuccess()) {
                std::cerr << "Failed with error: " << outcome.GetError() << std::endl;
            }
            else {
                std::cout << "Found " << outcome.GetResult().GetBuckets().size()
                    << " buckets\n";
                for (auto& bucket : outcome.GetResult().GetBuckets()) {
                    std::cout << bucket.GetName() << std::endl;
                }
            }
            return outcome.IsSuccess();
        }

        bool getObject(const Aws::String& objectKey,
            const Aws::String& fromBucket) 
        {
            Aws::S3::Model::GetObjectRequest request;
            request.SetBucket(fromBucket);
            request.SetKey(objectKey);

            Aws::S3::Model::GetObjectOutcome outcome =
                s3_client->GetObject(request);

            if (!outcome.IsSuccess()) {
                const Aws::S3::S3Error& err = outcome.GetError();
                std::cerr << "Error: GetObject: " <<
                    err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
            }
            else {
                std::cout << "Successfully retrieved '" << objectKey << "' from '"
                    << fromBucket << "'." << std::endl;

                std::string local_fileName = "/root/out/" + objectKey;
                std::ofstream local_file(local_fileName, std::ios::binary);
                auto& retrieved = outcome.GetResult().GetBody();
                local_file << retrieved.rdbuf();
                std::cout << "Done!";
            }

            return outcome.IsSuccess();
        }

        /**
        * Free up resources in class
        */
        void free()
        {
            delete s3_client;
        }
};
int main(int argc, char** argv) {
    Aws::SDKOptions options;
    // Optionally change the log level for debugging.
    // options.loggingOptions.logLevel = Utils::Logging::LogLevel::Debug;
    Aws::InitAPI(options); // Should only be called once.

    int result = 0;
    {
        Aws::Client::ClientConfiguration clientConfig;
        // Optional: Set to the AWS Region (overrides config file).
        clientConfig.region = Region::AP_SOUTHEAST_1;
        clientConfig.scheme = Http::Scheme::HTTPS;

        ServiceS3 serviceS3(clientConfig);

        serviceS3.listBuckets();

        // Assign these values before running the program
        const Aws::String bucketName = "myawsbucket-tl";
        const Aws::String objectName = "test01.txt";
        const std::string fileName = "/root/test01.txt";

        // Put the file into the S3 bucket
        if (serviceS3.putObject(bucketName, objectName, fileName)) {
            std::cout << "Put file " << fileName
                << " to S3 bucket " << bucketName
                << " as object " << objectName << std::endl;
        }

        serviceS3.listObjects(bucketName);

        serviceS3.getObject(objectName, bucketName);

        serviceS3.free();
        
    }

    Aws::ShutdownAPI(options); // Should only be called once.
    return result;
}