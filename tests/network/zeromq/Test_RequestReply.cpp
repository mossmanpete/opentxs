// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace zmq = opentxs::network::zeromq;

namespace
{

class Test_RequestReply : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/request_reply_test"};
    const std::string endpoint2_{"inproc://opentxs/test/request_reply_test2"};

    void requestSocketThread(const std::string& msg);
    void replySocketThread(const std::string& endpoint);
};

OTZMQContext Test_RequestReply::context_{zmq::Context::Factory()};

void Test_RequestReply::requestSocketThread(const std::string& msg)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    auto requestSocket =
        zmq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->SendRequest(msg);

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const std::string& messageString = *message->Body().begin();
    ASSERT_EQ(msg, messageString);
}

void Test_RequestReply::replySocketThread(const std::string& endpoint)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this, &replyReturned](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = zmq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RequestReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyReturned && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);
}

}  // namespace

TEST_F(Test_RequestReply, Request_Reply)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            EXPECT_EQ(testMessage_, inputString);

            auto reply = zmq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RequestReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    auto requestSocket =
        zmq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->SendRequest(testMessage_);

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const std::string& messageString = *message->Body().begin();
    ASSERT_EQ(testMessage_, messageString);
}

TEST_F(Test_RequestReply, Request_2_Reply_1)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = zmq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RequestReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    std::thread requestSocketThread1(
        &Test_RequestReply::requestSocketThread, this, testMessage2_);
    std::thread requestSocketThread2(
        &Test_RequestReply::requestSocketThread, this, testMessage3_);

    requestSocketThread1.join();
    requestSocketThread2.join();
}

TEST_F(Test_RequestReply, Request_1_Reply_2)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    std::thread replySocketThread1(
        &Test_RequestReply::replySocketThread, this, endpoint_);
    std::thread replySocketThread2(
        &Test_RequestReply::replySocketThread, this, endpoint2_);

    auto requestSocket =
        zmq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);
    requestSocket->Start(endpoint2_);

    auto [result, message] = requestSocket->SendRequest(testMessage2_);

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    std::string messageString = *message->Body().begin();
    ASSERT_EQ(testMessage2_, messageString);

    auto [result2, message2] = requestSocket->SendRequest(testMessage3_);

    ASSERT_EQ(result2, SendResult::VALID_REPLY);

    messageString = *message2->Body().begin();
    ASSERT_EQ(testMessage3_, messageString);

    replySocketThread1.join();
    replySocketThread2.join();
}

TEST_F(Test_RequestReply, Request_Reply_Multipart)
{
    ASSERT_NE(nullptr, &Test_RequestReply::context_.get());

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            for (const std::string& frame : input.Header()) {
                EXPECT_EQ(testMessage_, frame);
            }

            for (const std::string& frame : input.Body()) {
                bool match = frame == testMessage2_ || frame == testMessage3_;

                EXPECT_TRUE(match);
            }

            auto reply = zmq::Message::ReplyFactory(input);
            for (const auto& frame : input.Body()) { reply->AddFrame(frame); }
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_RequestReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    auto requestSocket =
        zmq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto multipartMessage = zmq::Message::Factory(testMessage_);
    multipartMessage->AddFrame();
    multipartMessage->AddFrame(testMessage2_);
    multipartMessage->AddFrame(testMessage3_);

    auto [result, message] = requestSocket->SendRequest(multipartMessage);

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const std::string& messageHeader = *message->Header().begin();

    ASSERT_EQ(testMessage_, messageHeader);

    for (const std::string& frame : message->Body()) {
        bool match = frame == testMessage2_ || frame == testMessage3_;
        ASSERT_TRUE(match);
    }
}
