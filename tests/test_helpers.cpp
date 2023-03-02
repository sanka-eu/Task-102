#include "utils/proto/message.pb.h"
#include "src/helpers.hpp"

#include <gtest/gtest.h>

using TestTask::Messages::WrapperMessage;
using std::vector;
using std::string;

#if GOOGLE_PROTOBUF_VERSION >= 3012004
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSizeLong())
#else
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSize())
#endif

typedef vector<char> Data;
typedef shared_ptr<const Data> PointerToConstData;

//функция для возвращения сериализованного сообщения
template <typename Message>
PointerToConstData serializeDelimited(const Message& msg) {
    const size_t messageSize = PROTOBUF_MESSAGE_BYTE_SIZE(msg);
    const size_t headerSize = google::protobuf::io::CodedOutputStream::VarintSize32(messageSize);

    const PointerToConstData & result = std::make_shared<Data>(headerSize + messageSize);
    google::protobuf::uint8 * buffer = reinterpret_cast<google::protobuf::uint8*>(const_cast<char *>(&*result->begin()));
    
    google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(messageSize, buffer);
    msg.SerializeWithCachedSizesToArray(buffer + headerSize);

    return result;
}

//проверка пустого буфера
TEST(ParseDelimitedTest, NullData) {
    shared_ptr<WrapperMessage> message;
    
    message = parseDelimited<WrapperMessage>(nullptr, 0);
    EXPECT_EQ(nullptr, message);

    message = parseDelimited<WrapperMessage>("", 0);
    EXPECT_EQ(nullptr, message);

    string buffer = "";
    message = parseDelimited<WrapperMessage>(buffer.data(), buffer.size());
    EXPECT_EQ(nullptr, message);
}

//проверка данных неправильного формата
TEST(ParseDelimitedTest, WrongData) {
    string buffer;
    shared_ptr<WrapperMessage> message;

    buffer = "Hello World";
    message = parseDelimited<WrapperMessage>(buffer.data(), buffer.size());
    EXPECT_EQ(nullptr, message);

    buffer = "\x02\x04\x1A";
    message = parseDelimited<WrapperMessage>(buffer.data(), buffer.size());
    EXPECT_EQ(nullptr, message);
}

//проверка данных с неправильным размером
TEST(ParseDelimitedTest, WrongSize) {
    WrapperMessage message;
    shared_ptr<WrapperMessage> parsedMessage;

    message.mutable_fast_response()->set_current_date_time("");
    auto data = serializeDelimited(message);

    parsedMessage = parseDelimited<WrapperMessage>(data->data(), 0);
    EXPECT_EQ(nullptr, parsedMessage);

    parsedMessage = parseDelimited<WrapperMessage>(data->data(), data->size() / 2);
    EXPECT_EQ(nullptr, parsedMessage);

    parsedMessage = parseDelimited<WrapperMessage>(data->data(), data->size() * 2);
    EXPECT_EQ(message.has_fast_response(), parsedMessage->has_fast_response());
    EXPECT_FALSE(parsedMessage->has_slow_response());
    EXPECT_EQ(message.fast_response().current_date_time(), parsedMessage->fast_response().current_date_time());
}

//проверка количества байтов потребляемых функцией
TEST(ParseDelimitedTest, CheckBytesConsumedPtr) {
    WrapperMessage message;
    shared_ptr<WrapperMessage> parsedMessage;
    size_t bytesConsumed = 0;

    message.mutable_fast_response()->set_current_date_time("");
    auto data = serializeDelimited(message);

    parsedMessage = parseDelimited<WrapperMessage>(data->data(), data->size(), &bytesConsumed);
    EXPECT_EQ(data->size(), bytesConsumed);
    bytesConsumed = 0;

    parsedMessage = parseDelimited<WrapperMessage>(nullptr, data->size(), &bytesConsumed);
    EXPECT_EQ(0, bytesConsumed);
}
