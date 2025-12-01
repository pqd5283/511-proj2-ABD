#include <iostream>
#include <memory>
#include <string>
#include <cstring>

#include <grpcpp/grpcpp.h>

// need to make this still but should be what it looks like 
#include "../proto/abdalgorithm.grpc.pb.h"
#include "client_rpc.h"

// gRPC stuff
using grpc::Channel;
using grpc::ClientContext;
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

// almost all of this code is adapted from gRPC examples AND the code from the last project
// biggest difference is we need to save the returned values from the server calls (since they return key/value pairs)
// just need to make classes for all 4 client things, reading, writeback reading, writing, writeback writing
class ClientReadClient {
public:
    explicit ClientReadClient(std::shared_ptr<Channel> channel)
        : stub_(ClientRead::NewStub(channel)) {}

    bool SendRead(int &out_key, std::string &out_value) {
        read_request req;

        read_reply reply;
        ClientContext context;

        Status status = stub_->SendRead(&context, req, &reply);
        if (!status.ok()) {
            return false;
        }

        out_key = static_cast<int>(reply.key());
        out_value = reply.value();
        return true;
    }

private:
    std::unique_ptr<ClientRead::Stub> stub_;
};

class ClientReadWritebackClient {
public:
    explicit ClientReadWritebackClient(std::shared_ptr<Channel> channel)
        : stub_(ClientReadWriteback::NewStub(channel)) {}

    bool SendReadWriteback(int key, const std::string &value) {
        read_writeback req;
        req.set_key(key);
        req.set_value(value);

        read_writeback_reply reply;
        ClientContext context;

        Status status = stub_->SendReadWriteback(&context, req, &reply);
        if (!status.ok()) {
            return false;
        }
        return reply.success();
    }

private:
    std::unique_ptr<ClientReadWriteback::Stub> stub_;
};

class ClientWriteClient {
public:
    explicit ClientWriteClient(std::shared_ptr<Channel> channel)
        : stub_(ClientWrite::NewStub(channel)) {}

    bool SendWrite(int &out_key, std::string &out_value) {
        write_request req;

        write_reply reply;
        ClientContext context;

        Status status = stub_->SendWrite(&context, req, &reply);
        if (!status.ok()) {
            return false;
        }
        out_key = static_cast<int>(reply.key());
        out_value = reply.value();
        return true;
    }

private:
    std::unique_ptr<ClientWrite::Stub> stub_;
};

class ClientWritebackClient {
public:
    explicit ClientWritebackClient(std::shared_ptr<Channel> channel)
        : stub_(ClientWriteback::NewStub(channel)) {}

    bool SendWriteback(int key, const std::string &value, const std::string &client_id) {
        writeback_request req;
        req.set_key(key);
        req.set_value(value);
        req.set_client_id(client_id);

        writeback_reply reply;
        ClientContext context;

        Status status = stub_->SendWriteback(&context, req, &reply);
        if (!status.ok()) {
            return false;
        }
        return reply.success();
    }

private:
    std::unique_ptr<ClientWriteback::Stub> stub_;
};

// will be able to call these in the c code to interact with the grpc stuff, need to be able to read and write 
extern "C" {

int rpc_send_read(const char *ip, int *out_key, char *out_value, size_t out_value_size)
{
    if (!ip) {
        return -1;
    }
    std::string server_address = ip;
    ClientReadClient client(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    // set dummy key and value to be filled by the client call 
    int key = 0;
    std::string value;
    // call the client to send the read request to the server
    if (!client.SendRead(key, value)) {
        return -1;
    }
    if (out_key) {
        *out_key = key;
    }
    // copy the returned value into the output buffer
    if (out_value && out_value_size > 0) {
        std::strncpy(out_value, value.c_str(), out_value_size - 1);
        out_value[out_value_size - 1] = '\0'; 
    }
    return 0; 
}


int rpc_send_read_writeback(const char *ip, int key, const char *value)
{
    if (!ip || !value) {
        return -1;
    }

    std::string server_address = ip;
    ClientReadWritebackClient client(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    if (!client.SendReadWriteback(key, std::string(value))) {
        return -1;
    }
    return 0;

}

int rpc_send_write(const char *ip, int *out_key, char *out_value, size_t out_value_size)
{
    if (!ip) {
        return -1;
    }

    std::string server_address = ip;
    ClientWriteClient client(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    // exact same setup as the read function, just calling the write client instead (need to do this to get the last highest timestamp, we dont really need the value but grabbing it just incase)
    int key = 0;
    std::string value;
    if (!client.SendWrite(key, value)) {
        return -1;
    }
    if (out_key) {
        *out_key = key;
    }
    if (out_value && out_value_size > 0) {
        std::strncpy(out_value, value.c_str(), out_value_size - 1);
        out_value[out_value_size - 1] = '\0'; 
    }
    return 0; 
}

int rpc_send_writeback(const char *ip, int key, const char *value, const char *client_id)
{
    if (!ip || !value || !client_id) {
        return -1;
    }

    std::string server_address = ip;
    ClientWritebackClient client(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    if (!client.SendWriteback(key, std::string(value), std::string(client_id))) {
        return -1;
    }

    return 0;
}

} 
