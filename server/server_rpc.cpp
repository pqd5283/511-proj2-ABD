#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "../proto/abdalgorithm.grpc.pb.h"
#include "server_rpc.h"
#include "server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using abd::ClientRead;
using abd::ClientReadWriteback;
using abd::ClientWrite;
using abd::ClientWriteback;
using abd::read_request;
using abd::read_reply;
using abd::read_writeback;
using abd::read_writeback_reply;
using abd::write_request;
using abd::write_reply;
using abd::writeback_request;
using abd::writeback_reply;
using abd::GetLock;
using abd::lock_request;
using abd::lock_reply;

// a lot of this code is yet again taken from and modified from the last project 
//server read 
class ClientReadService final : public ClientRead::Service {
public:
    Status SendRead(ServerContext* context,
                    const read_request* request,
                    read_reply* reply) override {

        int key = 0;
        char value_buf[1024]; // 1024 max for now 

        if (server_receive_read(&key, value_buf, sizeof(value_buf)) != 0) {
            return Status(grpc::StatusCode::INTERNAL, "abd_handle_read failed");
        }
        printf("successful read with key %d and value %s\n", key, value_buf);
        reply->set_key(key);
        reply->set_value(value_buf);
        return Status::OK;
    }
};



//server read writeback
class ClientReadWritebackService final : public ClientReadWriteback::Service {
public:
    Status SendReadWriteback(ServerContext* context,
                             const read_writeback* request,
                             read_writeback_reply* reply) override {

        int key = request->key();
        const std::string& value = request->value();

        printf("server received read writeback for key %d with value %s\n", key, value.c_str());
        int check = server_read_writeback(key, value.c_str());
        reply->set_success(check == 0);

        return Status::OK;
    }
};


//server write
class ClientWriteService final : public ClientWrite::Service {
public:
    Status SendWrite(ServerContext* context,
                     const write_request* request,
                     write_reply* reply) override {

        int key = 0;
        char value_buf[1024];

        if (server_receive_write(&key, value_buf, sizeof(value_buf)) != 0) {
            return Status(grpc::StatusCode::INTERNAL, "abd_handle_write_query failed");
        }
        printf("successful write with key %d and value %s\n", key, value_buf);
        reply->set_key(key);
        reply->set_value(value_buf);
        return Status::OK;
    }
};


//server write writeback
class ClientWritebackService final : public ClientWriteback::Service {
public:
    Status SendWriteback(ServerContext* context,
                         const writeback_request* request,
                         writeback_reply* reply) override {

        int key = request->key();
        const std::string& value = request->value();
        const std::string& client_id = request->client_id();

        int check = server_write_writeback(key, value.c_str(), client_id.c_str());
        reply->set_success(check == 0);
        printf("server received write writeback for key %d with value %s from client %s\n", key, value.c_str(), client_id.c_str());
        return Status::OK;
    }
};

// server lock service
class ClientLockService final : public GetLock::Service {
public:
    Status AcquireLock(ServerContext* context,
                       const lock_request* request,
                       lock_reply* reply) override {

        // the key is just gonna be set to a dummy value it doesnt really mean anything i dont think
        // just being sent to identify that there was a lock request
        std::string key = request->key();

        int check = server_acquire_lock(key.c_str());
        reply->set_granted(granted);
        printf("server received lock request with key %s\n", key.c_str());
        return Status::OK;
    }
};


// setup and run the server
void RunABDServer(const std::string& address) {
    ClientReadService read_service;
    ClientReadWritebackService readwb_service;
    ClientWriteService write_service;
    ClientWritebackService writewb_service;
    ClientLockService lock_service;

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&read_service);
    builder.RegisterService(&readwb_service);
    builder.RegisterService(&write_service);
    builder.RegisterService(&writewb_service);
    build.RegisterService(&lock_service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}

// main function to start the server
int main(int argc, char** argv) {
    if(server_init() != 0) {
        std::cerr << "Server initialization failed." << std::endl;
        return 1;
    }
    std::string port = "50051";
    if (argc > 1) {
        port = argv[1];
    }

    std::string server_address = "0.0.0.0:" + port;
    RunABDServer(server_address);
    return 0;
}
