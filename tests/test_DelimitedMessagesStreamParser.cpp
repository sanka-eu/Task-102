#include "utils/proto/message.pb.h"
#include "src/DelimitedMessagesStreamParser.hpp"

#include <gtest/gtest.h>

using TestTask::Messages::WrapperMessage;
typedef DelimitedMessagesStreamParser<WrapperMessage> Parser;

#if GOOGLE_PROTOBUF_VERSION >= 3012004
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSizeLong())
#else
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSize())
#endif

typedef vector<char> Data;
typedef shared_ptr<const Data> PointerToConstData;

//функци€ дл€ возвращени€ сериализованного сообщени€
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
TEST(DelimitedMessagesStreamParserTest, NullData) {
    string buffer;
    Parser parser;
    list<typename DelimitedMessagesStreamParser<WrapperMessage>::PointerToConstValue> messages;

    messages = parser.parse("");
    EXPECT_EQ(0, messages.size());

    buffer = "";
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());
}

//проверка данных не в том формате, в котором ожидаетс€
TEST(DelimitedMessagesStreamParserTest, WrongData) {
    string buffer;
    Parser parser;
    list<typename DelimitedMessagesStreamParser<WrapperMessage>::PointerToConstValue> messages;

    buffer = "Hello World";
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());

    buffer = "\x02\x04\x1A";
    parser = Parser();
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());

    buffer = "\n\n\n\n\n\t\t\t";
    parser = Parser();
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());

    buffer = "\x05""TEST";
    parser = Parser();
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());

    buffer = "\r\n";
    parser = Parser();
    messages = parser.parse(buffer);
    EXPECT_EQ(0, messages.size());
}

//проверка данных с неправильным префиксом длинны
TEST(DelimitedMessagesStreamParserTest, CorruptedData) {
    std::vector<char> stream;
    std::vector<char> copy;

    Parser parser;
    list<typename DelimitedMessagesStreamParser<WrapperMessage>::PointerToConstValue> messages;

    WrapperMessage fastResponseMessage;
    fastResponseMessage.mutable_fast_response()->set_current_date_time("1");
    // "\x05\n\x03\n\x01\x01"
    auto data = serializeDelimited(fastResponseMessage);

    stream.insert(stream.end(), data->begin(), data->end());
    stream.insert(stream.end(), data->begin(), data->end());
    stream.insert(stream.end(), data->begin(), data->end());
    // -------------------- -------------------- --------------------
    // \x05\n\x03\n\x01\x01 \x05\n\x03\n\x01\x01 \x05\n\x03\n\x01\x01

    messages = parser.parse(string(stream.begin(), stream.end()));
    EXPECT_EQ(3, messages.size());

    copy = std::vector<char>(stream);
    copy[data->size()] = '\x03';
    // corrupt size of second message from 5 to 3
    // -------------------- -------------------- --------------------
    // \x05\n\x03\n\x01\x01 \x05\n\x03\n\x01\x01 \x05\n\x03\n\x01\x01
    //                      ^
    //                      \x03
    // -------------------- ------------ -------- --------------------
    // \x05\n\x03\n\x01\x01 \x03\n\x03\n \x01\x01 \x05\n\x03\n\x01\x01
    messages = parser.parse(string(copy.begin(), copy.end()));
    EXPECT_EQ(2, messages.size());

    shared_ptr<const WrapperMessage> message;
    auto iter = messages.begin();
    
    message = *iter++;
    EXPECT_FALSE(message->has_request_for_fast_response());
    EXPECT_FALSE(message->has_request_for_slow_response());
    EXPECT_TRUE(message->has_fast_response());
    EXPECT_EQ(fastResponseMessage.fast_response().current_date_time(), message->fast_response().current_date_time());
    EXPECT_FALSE(message->has_slow_response());

    message = *iter++;
    EXPECT_FALSE(message->has_request_for_fast_response());
    EXPECT_FALSE(message->has_request_for_slow_response());
    EXPECT_TRUE(message->has_fast_response());
    EXPECT_EQ(fastResponseMessage.fast_response().current_date_time(), message->fast_response().current_date_time());
    EXPECT_FALSE(message->has_slow_response());

    copy = std::vector<char>(stream);
    string something = "\x02\x01\x05\x02\x01\x01\x01";
    copy.insert(copy.begin(), something.begin(), something.end());
    // ------------ ------------ -------- ------------------------------------ --------------------
    // \x02\x01\x05 \x02\x01\x01 \x01\x05 \n\x03\n\x01\x01\x05\n\x03\n\x01\x01 \x05\n\x03\n\x01\x01

    messages = parser.parse(string(copy.begin(), copy.end()));
    EXPECT_EQ(1, messages.size());
    
    iter = messages.begin();

    message = *iter++;
    EXPECT_FALSE(message->has_request_for_fast_response());
    EXPECT_FALSE(message->has_request_for_slow_response());
    EXPECT_TRUE(message->has_fast_response());
    EXPECT_EQ(fastResponseMessage.fast_response().current_date_time(), message->fast_response().current_date_time());
    EXPECT_FALSE(message->has_slow_response());
}
